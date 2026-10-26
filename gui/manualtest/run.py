#!/usr/bin/env python3
"""Run the manual tests in a test file by letting Claude drive the GUI.

  run.py APP FILE.feature... [--edition opensource|premium] [--test SUBSTRING] [--id ID] [--src DIR] [--out DIR]

FILE.feature is a Gherkin test file, edited with web.py or by hand, see testfile.py. The scripts
are in FILE.scripts.json. For each scenario (and each row of its Examples) the application is started
fresh (empty HOME) and the script of the test is run with guitest.py commands, see runner.py.
No AI is involved. With --edition APP is that edition and only the tests for it are run.
Tests without a script are SKIP. A step that fails gives PASS -> FAIL with the
step, the reason and the elements that are on the screen.
"""

import argparse
import os
import re
import datetime
import json
import platform
import signal
import subprocess
import sys
import tempfile
import time

import report
import runner
import testfile

HERE = os.path.dirname(os.path.abspath(__file__))
GUITEST = os.path.join(HERE, 'guitest.py')

def parse(path, edition=None):
    return [dict(t, file=os.path.basename(path)) for t in testfile.expand(path, edition)]


CRASH_RE = re.compile(r'exited with status (?!0\b|143\b)\d+|Segmentation fault|SIGSEGV|SIGABRT|Aborted|core dumped|terminate called|'
                      r'QFATAL|Fatal error|AddressSanitizer|runtime error:|Assertion .* failed', re.I)


def screenshot(sub, path):
    """Save a screenshot without the empty (black) part of the screen right of and below the windows.
    The top left corner stays, so positions in the image are screen positions."""
    if sub('screenshot', path, capture_output=True).returncode:
        return False
    try:
        from PIL import Image
        img = Image.open(path)
        box = img.convert('RGB').getbbox()
        if box and (box[2] < img.width or box[3] < img.height):
            img.crop((0, 0, box[2], box[3])).save(path)
    except (ImportError, OSError):
        pass
    return True


def detect_crash(sub, outdir, slug):
    """Return a description when the application died or logged a fatal error, else ''."""
    log = sub('log', capture_output=True).stdout
    with open(os.path.join(outdir, slug + '.app.log'), 'w') as f:
        f.write(log)
    hits = [l.strip() for l in log.splitlines() if CRASH_RE.search(l)]
    dead = sub('alive', capture_output=True).returncode != 0
    if dead and re.search(r'application exited with status 0\b', log):
        dead = False        # the test closed the application (Quit)
    if not dead and not hits:
        return ''
    why = 'application is not running at the end of the test' if dead else 'fatal message in application log'
    return why + (': ' + hits[0] if hits else '')


def emit(kind, **data):
    """One line of progress for the web interface, with --events."""
    if EVENTS:
        print('@' + json.dumps(dict(data, kind=kind)), flush=True)


EVENTS = False


def run_test(app, test, outdir, on_step=None):
    """Returns {status, reason, crash, steps, failed_step, shots}. failed_step is 0 when no step failed,
    shots are the screenshots after the steps: [{step, label, file}]."""
    slug = re.sub(r'\W+', '_', test['name'])
    shots, reached = [], [0]
    result = lambda status, reason, crash='', failed=0: {'status': status, 'reason': reason, 'crash': crash,
                                                         'steps': len(test['script']), 'failed_step': failed, 'shots': shots}
    if test.get('stale'):
        return result('SKIP', 'OUTDATED SCRIPT: the test was changed after the script was made. '
                              'Generate the script again (gen.py or the web interface) or approve it there.')
    if not test['script']:
        return result('SKIP', 'no script, make one with gen.py or in the web interface')
    work = tempfile.mkdtemp(prefix='guitest-work-')
    state = os.path.join(work, 'state.json')
    env = dict(os.environ, GUITEST_STATE=state)
    sub = lambda *a, **kw: subprocess.run([sys.executable, GUITEST, *a], env=env, text=True, **kw)
    try:
        variables = testfile.materialize(test['root'], test['projects'], work)   # same files every run
    except FileNotFoundError as e:
        return result('FAIL', str(e))
    r = sub('start', app, capture_output=True)
    if r.returncode:
        return result('FAIL', 'could not start: ' + (r.stdout + r.stderr).strip())
    try:
        with open(state) as f:
            home = json.load(f)['env']['HOME']
        def shot(i, label):
            name = '%s.step%02d.png' % (slug, i)
            if not screenshot(sub, os.path.join(outdir, name)):
                return None
            shots.append({'step': i, 'label': label, 'file': name})
            return name
        def step(i, n, label):
            reached[0] = i
            if on_step:
                on_step(i, n, label)
        ok, reason, log = runner.run_script(sub, test['script'], work, home, variables, step, shot)
        with open(os.path.join(outdir, slug + '.txt'), 'w') as f:
            f.write(log)
        screenshot(sub, os.path.join(outdir, slug + '.png'))
        crash = detect_crash(sub, outdir, slug)
        status = 'PASS' if ok else 'FAIL'
        if crash:
            # a crash is a failure even if all the steps passed
            status, reason = 'FAIL', 'APPLICATION CRASHED: %s\n%s' % (crash, reason)
        return result(status, reason, crash, 0 if ok else reached[0])
    finally:
        sub('stop', capture_output=True)


def git(directory, *a):
    r = subprocess.run(['git', '-C', directory, *a], text=True, capture_output=True)
    return r.stdout.strip() if r.returncode == 0 else ''


def app_info(app, src):
    """git info for the tested application, taken from SRC (default: the directory of the binary)."""
    d = os.path.abspath(src) if src else os.path.dirname(app)
    sha = git(d, 'rev-parse', 'HEAD')
    return {
        'path': app,
        'sha': sha or 'unknown',
        'branch': git(d, 'rev-parse', '--abbrev-ref', 'HEAD') or 'unknown',
        'subject': git(d, 'log', '-1', '--format=%s') if sha else '',
        'dirty': bool(git(d, 'status', '--porcelain', '--untracked-files=no')) if sha else False,
    }


def load_history(out, edition, current):
    """results.json of the earlier runs of EDITION in OUT, oldest first."""
    runs = []
    for d in sorted(os.listdir(out)):
        path = os.path.join(out, d, 'results.json')
        if d == current or not os.path.isfile(path):
            continue
        try:
            with open(path, encoding='utf-8') as f:
                run = json.load(f)
        except (OSError, ValueError):
            continue
        if run.get('edition') == edition:
            runs.append(run)
    return runs[-(report.HISTORY_RUNS - 1):]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('app')
    ap.add_argument('file', nargs='+', help='one or more .feature test files')
    ap.add_argument('--edition', choices=testfile.EDITIONS, help='the edition that APP is, only the tests for it')
    ap.add_argument('--test', help='only tests whose name contains this')
    ap.add_argument('--id', help='only the test with this id (all the rows of its Examples)')
    ap.add_argument('--src', help='git checkout of the application (default: the directory of APP)')
    ap.add_argument('--out', default=os.path.join(HERE, 'results'))
    ap.add_argument('--events', action='store_true', help='progress lines for the web interface')
    args = ap.parse_args()
    global EVENTS
    EVENTS = args.events
    signal.signal(signal.SIGINT, signal.default_int_handler)   # also when started in the background

    started = datetime.datetime.now()
    info = app_info(os.path.abspath(args.app), args.src)
    # each run gets its own directory: results/<date>[-<edition>]-<sha>/
    parent = args.out
    args.out = os.path.join(args.out, '%s%s-%s' % (started.strftime('%Y%m%d-%H%M%S'),
                                                   '-' + args.edition if args.edition else '', info['sha'][:8]))
    os.makedirs(args.out, exist_ok=True)
    failed = 0
    results = []
    aborted = False
    try:
        selected = [t for f in args.file for t in parse(f, args.edition)
                    if not (args.test and args.test.lower() not in t['name'].lower() or args.id and t['id'] != args.id)]
        emit('start', total=len(selected), out=args.out, edition=args.edition)
        for index, test in enumerate(selected, 1):
            name = test['name']
            emit('test', index=index, name=name, file=test['file'], steps=len(test['script']))
            t0 = time.time()
            try:
                res = run_test(os.path.abspath(args.app), test, args.out,
                               lambda k, n, label: emit('step', step=k, steps=n, label=label))
            except KeyboardInterrupt:
                results.append({'name': name, 'file': test['file'], 'status': 'SKIP', 'reason': 'aborted by user', 'crash': '',
                                'seconds': time.time() - t0, 'slug': re.sub(r'\W+', '_', name), 'steps': 0, 'failed_step': 0, 'shots': []})
                emit('result', **results[-1])
                raise
            results.append(dict(res, name=name, file=test['file'], seconds=time.time() - t0, slug=re.sub(r'\W+', '_', name)))
            emit('result', **results[-1])
            print('%-4s %s: %s' % (res['status'], name, res['reason']), flush=True)
            failed += res['status'] == 'FAIL'
    except KeyboardInterrupt:
        # the report is written with what is done, tests not started are left out
        aborted = True
        print('aborted')
    run = {'started': started.isoformat(timespec='seconds'), 'edition': args.edition, 'app': info['path'],
           'sha': info['sha'], 'dirty': info['dirty'], 'branch': info['branch'], 'subject': info['subject'],
           'host': '%s, %s' % (platform.node(), platform.platform()), 'aborted': aborted,
           'files': [{'file': os.path.basename(f), 'title': testfile.load(f).get('title', '')} for f in args.file],
           'results': results}
    with open(os.path.join(args.out, 'results.json'), 'w', encoding='utf-8') as f:
        json.dump(run, f, indent=1)
    path = os.path.join(args.out, 'report.html')
    report.write(path, run, load_history(parent, args.edition, os.path.basename(args.out)))
    print('report: ' + path)
    emit('done', report=path)
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
