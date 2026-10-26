#!/usr/bin/env python3
"""Let Claude write the scripts of the tests. The scripts are replayed by run.py without AI.

  gen.py APP FILE.feature [--edition opensource|premium] [--test SUBSTRING] [--force] [--model M]

For every test without a script (or every test with --force) Claude gets the scenario, drives the
GUI once with the guitest.py commands and answers with a script, see testfile.SCRIPT_DOC.
The script is run again from scratch and is saved in FILE.scripts.json only if that replay passes.
A Scenario Outline is explored with the first row of its Examples, the script uses {column} where
the values of the row belong. With --edition APP is that edition, tests that are not for it are
left out; the script is used for all the editions of the test.
"""

import argparse
import json
import os
import re
import signal
import subprocess
import sys
import tempfile

import runner
import testfile

HERE = os.path.dirname(os.path.abspath(__file__))
GUITEST = os.path.join(HERE, 'guitest.py')

PROMPT = """You are writing an automated GUI test script for a Qt application that has been started for you.
First do the test by hand, driving the application only with this command line tool (each call is separate,
the session is already running):

  python3 {guitest} <command>

{help}

Tips:
- Start with `tree --showing --depth 3`, `windows` and `texts` to see what is on screen.
- Use `info --name X` to see the roles and states of an element, e.g. the [checked] state.
- Menu items: click the menu (e.g. --name File --role "menu item"), then click the entry.
- Keyboard input goes to the focused element: `focus` first, then `type` / `key`.
- The files of the test are in the test projects, they are already in place, use them as they are:{projects}
  Do not make copies of them. In the script write {{project:NAME}} instead of the directory, it is another
  directory every run. Anything else that you need can be created in {work} (use Write), do not touch other places.
- A menu that is on screen but not in `tree`/`texts` (e.g. a context menu) can be read with `ocr`; check it in the
  script with {{"expect": "ocr", "contains": "..."}} and use the keyboard (`key Down`, `key Return`) to choose in it.
- Never read or modify the application source code.

Test name: {name}

The test is written in Gherkin: the Background and the Given steps are the starting point, the When steps are
what you do and the Then steps are the EXPECTED result.

{body}

When you have done the test and checked the EXPECTED result (the Then steps), answer with a script that does the
same without you. The script is replayed by a program, so:
- use the exact roles and names that you saw. Give a name and a role. Use "path" only when nothing else tells
  the element apart, paths change when the dialogs change (a file dialog's name field is found by its label)
- a name matches any element that contains it, so always give the role too: a toolbar button
  "Preferences" and the dialog "Preferences" have the same name, the dialog is role "dialog"
- give every step a "label" that says what it is for, in the words of the test
- end with "expect" steps that check the EXPECTED result, at least one for each Then step
- the files of the test are the test projects, refer to them with {{project:NAME}} paths; files that
  you made yourself are created by "write" steps (in {{work}}), not by you
- the application is started fresh before the script, with an empty HOME
- {matrixnote}
- {editionnote}

{scriptdoc}

{previous}Check the EXPECTED result honestly. If the application does not do what EXPECTED says that is a finding, not
a mistake in the script: still write the script that checks EXPECTED (it will fail, that is the point) and put
the line `OBSERVED: FAIL - <what happens instead>` before the JSON block and add "observed_fail": true to
the one check that fails. Otherwise put `OBSERVED: PASS`.

Finish with the script as one JSON list in a ```json code block and nothing after it.
If the test cannot be done through the GUI answer with the single line CANNOT: <reason>.
"""


PREVIOUS = """An earlier attempt gave this script:
{script}

When it was replayed from scratch by the program it failed:
{error}

Find out what is wrong (look at the GUI again if needed) and answer with a corrected script.

"""


def start_session(app, root, projects):
    work = tempfile.mkdtemp(prefix='guitest-work-')
    state = os.path.join(work, 'state.json')
    env = dict(os.environ, GUITEST_STATE=state)
    sub = lambda *a, **kw: subprocess.run([sys.executable, GUITEST, *a], env=env, text=True, **kw)
    variables = testfile.materialize(root, projects, work)
    r = sub('start', app, capture_output=True)
    if r.returncode:
        sys.exit('error: could not start: ' + (r.stdout + r.stderr).strip())
    with open(state) as f:
        home = json.load(f)['env']['HOME']
    return work, home, env, sub, variables


def explore(app, root, test, combo, edition, model, background, previous=''):
    """Claude does the test by hand and returns (script, error); with a script the error is (OBSERVED, reason)."""
    work, home, env, sub, variables = start_session(app, root, test.get('projects', []))
    try:
        note = ('this is a Scenario Outline, you explore it with the Examples row %s. Write {%s} in the script wherever '
                'that value is used, the script is run once for every row of the Examples' % (
                    ', '.join('%s=%s' % kv for kv in combo.items()), '}, {'.join(combo))
                ) if combo else 'there are no Examples, write the values in the script as they are'
        others = [e for e in testfile.editions(test) if e != edition]
        if not edition:
            editionnote = 'there is one edition of the application'
        elif others:
            editionnote = ('the application is the %s edition. The same script is also run with the %s edition, '
                           'write {edition} where the name of the edition is shown' % (edition, ' and '.join(others)))
        else:
            editionnote = 'the script is only run with the %s edition' % edition
        projects = ''.join('\n    {project:%s} = %s, files: %s' % (n, variables['project:' + n],
                                                                  ', '.join(sorted(testfile.project_files(root, n))))
                           for n in test.get('projects', [])) or ' none'
        prompt = PROMPT.format(guitest=GUITEST, help=sub('--help', capture_output=True).stdout, work=work,
                               name=test['name'], body=testfile.body(test, combo, edition, background), matrixnote=note,
                               editionnote=editionnote,
                               scriptdoc=testfile.SCRIPT_DOC, previous=previous, projects=projects)
        cmd = ['claude', '-p', '--allowedTools', 'Bash(python3 %s:*)' % GUITEST, 'Read', 'Write', '--add-dir', work]
        if model:
            cmd += ['--model', model]
        p = subprocess.run(cmd, input=prompt, env=env, text=True, capture_output=True, cwd=work)
    finally:
        sub('stop', capture_output=True)
    m = re.findall(r'^CANNOT:\s*(.*)$', p.stdout, re.M)
    if m:
        return None, 'Claude: cannot be tested through the GUI: ' + m[-1]
    m = re.findall(r'```json\s*(.*?)```', p.stdout, re.S)
    if not m:
        return None, 'no script in the answer of Claude: ' + (p.stdout.strip() or p.stderr.strip())[:300]
    try:
        script = json.loads(m[-1])
    except ValueError as e:
        return None, 'the script is not valid JSON: %s' % e
    errors = testfile.validate_script(script)
    observed = re.findall(r'^OBSERVED:\s*(PASS|FAIL)\s*-?\s*(.*)$', p.stdout, re.M)
    return (None, '; '.join(errors)) if errors else (script, observed[-1] if observed else ('PASS', ''))


def replay(app, root, test, script):
    work, home, env, sub, variables = start_session(app, root, test.get('projects', []))
    try:
        ok, msg, log = runner.run_script(sub, script, work, home, variables)
    finally:
        sub('stop', capture_output=True)
    return ok, msg


def store(path, test_id, script, made_for):
    """Put the script in the file as it is now, the file may have been saved by the web interface meanwhile."""
    doc = testfile.ensure_ids(testfile.load(path))
    for t in doc['tests']:
        if t['id'] == test_id:
            t['script'], t['script_for'] = script, made_for
    testfile.save(path, doc)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('app')
    ap.add_argument('file')
    ap.add_argument('--edition', choices=testfile.EDITIONS, help='the edition that APP is, only the tests for it')
    ap.add_argument('--test', help='only tests whose name contains this')
    ap.add_argument('--id', help='only the test with this id')
    ap.add_argument('--force', action='store_true', help='also tests that have a script')
    ap.add_argument('--model')
    ap.add_argument('--attempts', type=int, default=3, help='tries per test, a failed replay is told to Claude')
    args = ap.parse_args()
    signal.signal(signal.SIGINT, signal.default_int_handler)   # also when started in the background
    if not args.file.endswith(testfile.EXT):
        sys.exit('error: FILE must be a %s test file, convert markdown or JSON with testfile.py convert' % testfile.EXT)
    app = os.path.abspath(args.app)
    doc = testfile.ensure_ids(testfile.load(args.file))
    root = os.path.dirname(os.path.abspath(args.file))
    errors = testfile.validate(doc, root)
    if errors:
        sys.exit('error: ' + '; '.join(errors))

    failed = 0
    for t in doc['tests']:
        if args.test and args.test.lower() not in t['name'].lower() or args.id and t['id'] != args.id:
            continue
        if (t.get('script') and not args.force) or not t.get('enabled', True):
            continue
        if args.edition and args.edition not in testfile.editions(t):
            print('%s: not a test for the %s edition' % (t['name'], args.edition), flush=True)
            continue
        combo = testfile.combinations(t)[0]
        values = dict(combo, edition=args.edition) if args.edition else combo
        made_for = testfile.definition_hash(t, root, doc.get('background'))   # the test may be edited while Claude works
        print('%s: exploring...' % t['name'], flush=True)
        previous, saved = '', False
        for attempt in range(1, args.attempts + 1):
            script, err = explore(app, root, t, combo, args.edition, args.model, doc.get('background'), previous)
            if script is not None:
                # the replay uses the first combination, as the exploration did
                observed, why = err
                ok, err = replay(app, root, t, testfile.subst_deep(script, values))
                # a test that fails on the check that Claude saw fail by hand is a finding: the script is right
                m = re.match(r'step (\d+)/', err)
                finding = (not ok and observed == 'FAIL' and m and script[int(m.group(1)) - 1].get('observed_fail'))
                if ok or finding:
                    store(args.file, t['id'], script, made_for)
                    print('%s: saved a script with %d steps (attempt %d)%s' % (
                        t['name'], len(script), attempt,
                        '. It FAILS on a check, as Claude saw: ' + why if finding else ''), flush=True)
                    saved = True
                    break
                err = 'the replay of the script failed:\n' + err
                previous = PREVIOUS.format(script=json.dumps(script, indent=1), error=err)
            print('%s: attempt %d failed. %s' % (t['name'], attempt, err), flush=True)
            if re.search(r'usage limit|rate limit|limit reached|out of credits', str(err), re.I):
                sys.exit('error: Claude refuses to answer (limit), stopping. Run gen.py again later.')
            if err.startswith('Claude: cannot'):
                break
        if not saved:
            failed += 1
            print('%s: no script saved' % t['name'], flush=True)
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
