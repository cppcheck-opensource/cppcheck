#!/usr/bin/env python3
"""Test files for run.py, written in Gherkin (FILE.feature), with the scripts in FILE.scripts.json.

  Feature: Project file dialog

    Background:
      Given the application is started with no project loaded

    @id:t1a2b3c4d @ticket:14983 @project:relative-paths
    Scenario: Relative paths
      TODO: cannot be automated              optional, a description line that starts with TODO:
      When I open the project dialog
      And I select the platform <platform>
      Then relative paths are used in the XML

      Examples:
        | platform | lang |
        | native   | C    |
        | unix32   | C++  |

Feature: is the title of the file. Background: steps are done before every scenario of the file.
A scenario has steps that start with Given, When, Then, And, But or *, at least one Then (the
expected result). A step can be followed by a doc string (lines between \"\"\") or a table (| a | b |).
Lines that start with # are comments, they are not kept when the file is saved by web.py.

Tags of a scenario (all optional):
  @id:ID              identifies a test when it is renamed, added when it is missing
  @ticket:NUMBER      the ticket of the test (#NUMBER)
  @project:NAME       a test project, see below; one tag per project
  @edition:NAME       an edition that the test is for, one of EDITIONS; one tag per edition, none: all
  @disabled           the test is not run
Other tags are kept but not used.

A run is for one edition (run.py --edition, with the binary of that edition), the tests that are
not for it are left out. The script is the same for all editions, {edition} in the steps and the
script is the name of the edition of the run.

Examples (Scenario Outline) are the test matrix: the test is run once for every row of the table.
<column> in the steps and {column} in the script are replaced by the value of the row.

"script" is what run.py executes, see SCRIPT_DOC below. It is written by gen.py (an AI explores
the GUI once) or by hand, and is then replayed without any AI. The scripts are kept in
FILE.scripts.json, {"ID": {"script": [...], "script_for": "..."}}, next to the feature file.

@project:NAME are test projects, directories with files and code in DIR/projects/NAME,
shared by all tests next to them. Before every run they are copied to an empty temporary directory,
so a test always starts with the same files: {project:NAME} in steps and scripts is the copy of NAME,
and {project} when the test has exactly one project.

A script is made for the steps (with the Background), todo, examples and test projects (the files in
them) of a test. "script_for" is a hash of those at the time, when they are changed afterwards the
script is OUTDATED: it is kept but not run until it is generated again, edited or approved.

In Python a test file is a dict {"title", "background": [lines], "tests": [test]}, a test is
{"id", "name", "steps": [lines], "todo", "ticket", "enabled", "projects", "editions", "examples":
[{column: value}], "script", "script_for"}. The lines of the steps are Gherkin without indentation.

  testfile.py convert IN.md|IN.json OUT.feature     import a markdown or an older JSON test file
"""

import hashlib
import itertools
import json
import os
import re
import shutil
import sys

AXIS_RE = re.compile(r'^\w+$')
PROJECT_RE = re.compile(r'^[\w-]+$')
PROJECTS = 'projects'     # test projects: one directory per project, next to the test files
EDITIONS = ('opensource', 'premium')
EXT = '.feature'
SCRIPTS_EXT = '.scripts.json'

SCRIPT_DOC = """A script is a list of steps. Every step is an object with "do" (an action) or "expect" (a check),
and an optional "label" that says what the step is for (shown when it fails). Elements are found
with name (substring of the accessible name/text, case-insensitive), role, path, nth, exact, like
the guitest.py commands. {work} is a temporary directory for files, {home} the HOME of the
application, {project:NAME} the copy of the test project NAME ({project} when the test has just one),
use those for the files of a test instead of write steps. {column} values of the Examples row and {edition}
(opensource or premium, the same script is used for both) are replaced before the run.

actions
  {"do": "click|rightclick|doubleclick|focus|hover", "name": "...", "role": "...", "mouse": true}
  {"do": "settext", "name": "...", "value": "..."}      set the text of an edit field
  {"do": "type", "value": "..."}                         keyboard input to the focused element
  {"do": "key", "keys": ["ctrl+o", "Return"]}            xdotool key names
  {"do": "wait", "name": "...", "gone": true, "timeout": 10}
  {"do": "restart"}                                      quit and start the application again
  {"do": "sleep", "seconds": 1}
  {"do": "write", "path": "{work}/a.c", "content": "..."}
  {"do": "copy", "from": "{project:p}/a_v2.c", "to": "{project:p}/a.c"}   change a file of the project
checks, the test fails when one is wrong
  {"expect": "visible", "name": "...", "role": "...", "timeout": 5}
  {"expect": "gone", "name": "..."}
  {"expect": "state", "name": "...", "state": "checked"}   or "not_state": "checked"; also
                                                          showing enabled focused selected expanded
  {"expect": "value", "name": "...", "equals": "1"}        value or text of a field, a spin box...
  {"expect": "exited"}                                   the application has ended by itself, status 0 (Quit)
  {"expect": "text", "contains": "..."}                  or "not_contains": anywhere on screen
  {"expect": "ocr", "contains": "Recheck"}               text on the screen read by OCR (tesseract), or "not_contains";
                                                          only for what "text" cannot see, e.g. a context menu that
                                                          is not in the accessibility tree. Case is ignored.
  {"expect": "file", "path": "{home}/x.xml", "contains": "...", "not_contains": "...", "regex": "..."}
  {"expect": "alive"}
  {"expect": "capture", "name": "...", "role": "frame", "regex": "'Cppcheck (\\d[^']*)'", "as": "version"}
                                                          the regex is searched in the element's info line
                                                          (role 'name' text='...' value=...), group 1 (or the
                                                          whole match) is stored and {version} in the later
                                                          steps is replaced by it; use it instead of hard coding
                                                          text that changes, e.g. the version
A check can have "observed_fail": true when the application was seen not to do what the check says
(a known failure, only for the check that failed). Text checks wait up to "timeout" seconds (default 5).
Actions that need an element wait up to "timeout" seconds (default 5) for it to appear."""

SELECTOR = {'name', 'role', 'path', 'nth', 'exact'}
# step name -> (required keys, allowed optional keys)
STEPS = {
    'do': {
        'click': ((), SELECTOR | {'mouse'}), 'rightclick': ((), SELECTOR), 'doubleclick': ((), SELECTOR),
        'focus': ((), SELECTOR), 'hover': ((), SELECTOR),
        'settext': (('value',), SELECTOR), 'type': (('value',), set()), 'key': (('keys',), set()),
        'wait': ((), SELECTOR | {'gone'}), 'restart': ((), set()),
        'sleep': (('seconds',), set()), 'write': (('path', 'content'), set()), 'copy': (('from', 'to'), set()),
    },
    'expect': {
        'visible': ((), SELECTOR), 'gone': ((), SELECTOR), 'value': (('equals',), SELECTOR), 'exited': ((), set()),
        'state': ((), SELECTOR | {'state', 'not_state'}),
        'text': ((), {'contains', 'not_contains'}), 'file': (('path',), {'contains', 'not_contains', 'regex'}),
        'alive': ((), set()), 'ocr': ((), {'contains', 'not_contains'}),
        'capture': (('regex', 'as'), SELECTOR),
    },
}
NEEDS_SELECTOR = {'click', 'rightclick', 'doubleclick', 'focus', 'hover', 'settext', 'wait', 'visible', 'gone', 'state', 'value',
                  'capture'}
STATES = {'showing', 'enabled', 'focused', 'selected', 'expanded', 'checked', 'editable', 'modal'}


def validate_script(script, where='script'):
    if not isinstance(script, list):
        return [where + ': must be a list of steps']
    errors = []
    for i, step in enumerate(script, 1):
        w = '%s step %d' % (where, i)
        if not isinstance(step, dict):
            errors.append(w + ': not an object')
            continue
        kinds = [k for k in ('do', 'expect') if k in step]
        if len(kinds) != 1:
            errors.append(w + ': needs exactly one of "do" or "expect"')
            continue
        kind, action = kinds[0], step[kinds[0]]
        if action not in STEPS[kind]:
            errors.append('%s: unknown %s "%s", use one of %s' % (w, kind, action, ', '.join(STEPS[kind])))
            continue
        required, optional = STEPS[kind][action]
        allowed = set(required) | optional | {kind, 'label', 'timeout'} | ({'observed_fail'} if kind == 'expect' else set())
        for k in step:
            if k not in allowed:
                errors.append('%s: "%s" is not used by %s %s' % (w, k, kind, action))
        for k in required:
            if k not in step:
                errors.append('%s: %s %s needs "%s"' % (w, kind, action, k))
        if action in NEEDS_SELECTOR and not (SELECTOR - {'nth', 'exact'}) & set(step):
            errors.append('%s: %s %s needs name, role or path' % (w, kind, action))
        if action in ('text', 'ocr') and not {'contains', 'not_contains'} & set(step):
            errors.append(w + ': expect %s needs contains or not_contains' % action)
        if action == 'state' and not {'state', 'not_state'} & set(step):
            errors.append(w + ': expect state needs state or not_state')
        for k in ('state', 'not_state'):
            if k in step and step[k] not in STATES:
                errors.append('%s: unknown state "%s", use one of %s' % (w, step[k], ', '.join(sorted(STATES))))
        if action == 'capture':
            try:
                re.compile(step['regex'])
            except (re.error, TypeError) as e:
                errors.append('%s: bad regex: %s' % (w, e))
            if not re.fullmatch(r'[\w-]+', str(step['as'])):
                errors.append(w + ': "as" must be a name of letters, digits, _ and -')
    if script and not any('expect' in s for s in script if isinstance(s, dict)):
        errors.append(where + ': needs at least one "expect" step, otherwise nothing is tested')
    return errors


STEP_RE = re.compile(r'^(Given|When|Then|And|But|\*)\s+\S')
SCENARIO_RE = re.compile(r'^(Scenario Outline|Scenario Template|Scenario|Example):\s*(.*)$')
TAG_RE = re.compile(r'^@[^\s@]+$')
DOCSTRING = ('"""', '```')


class ParseError(ValueError):
    pass


def validate_steps(lines, where, need_then=True):
    """Errors in the Gherkin lines of a scenario or background."""
    errors, fence, keyword = [], None, None
    for n, line in enumerate(lines, 1):
        s = line.strip()
        if fence:
            if s == fence:
                fence = None
            continue
        if s.startswith(DOCSTRING) or s.startswith('|'):
            if keyword is None:
                errors.append('%s line %d: a doc string or table must come after a step' % (where, n))
            if s.startswith(DOCSTRING):
                fence = s[:3]
            continue
        m = STEP_RE.match(s)
        if not m:
            errors.append('%s line %d: a step must start with Given, When, Then, And, But or *: %s' % (where, n, s))
            continue
        if keyword is None and m.group(1) in ('And', 'But'):
            errors.append('%s line %d: the first step can not start with %s' % (where, n, m.group(1)))
        keyword = keyword if m.group(1) in ('And', 'But', '*') else m.group(1)
    if fence:
        errors.append(where + ': a doc string is not closed with ' + fence)
    if need_then and not any(re.match(r'^Then\s', line.strip()) for line in lines):
        errors.append(where + ': needs a Then step, the expected result')
    return errors


def parse(text):
    """A .feature file as a test file dict. Raises ParseError."""
    doc = {'title': '', 'tests': []}
    tags, section, fence, indent, table = [], None, None, 0, None
    lines = text.splitlines()

    def fail(n, msg):
        raise ParseError('line %d: %s' % (n, msg))

    for n, raw in enumerate(lines, 1):
        s = raw.strip()
        if fence:                                   # inside a doc string, kept as it is
            section['steps'].append(raw[indent:] if raw[:indent].strip() == '' else s)
            if s == fence:
                section['steps'][-1] = s
                fence = None
            continue
        if not s or s.startswith('#'):
            continue
        if s.startswith('@'):
            for tag in s.split('#')[0].split():
                if not TAG_RE.match(tag):
                    fail(n, 'bad tag ' + tag)
                tags.append(tag[1:])
            continue
        table = table if s.startswith('|') else None
        if s.startswith('Feature:'):
            if 'feature' in doc:
                fail(n, 'only one Feature per file')
            doc['feature'] = True
            doc['title'] = s[len('Feature:'):].strip()
            if tags:
                doc['tags'], tags = tags, []
            section = doc
            continue
        if 'feature' not in doc:
            fail(n, 'the file must start with Feature:')
        if s.startswith('Background:'):
            if doc['tests'] or 'background' in doc:
                fail(n, 'the Background must come before the scenarios, once')
            doc['background'] = []
            section = {'steps': doc['background']}
            continue
        m = SCENARIO_RE.match(s)
        if m:
            section = scenario(m.group(2).strip(), tags)
            doc['tests'].append(section)
            tags = []
            continue
        if s.startswith(('Rule:', 'Scenarios:')) or (s.startswith('Examples:') and section is doc):
            fail(n, s.split(':')[0] + ' is not supported here')
        if s.startswith('Examples:'):
            if 'name' not in section:
                fail(n, 'Examples only belong to a scenario')
            section['examples'] = section.get('examples', [])
            table = []
            continue
        if table is not None:                       # a row of Examples
            cells = [c.strip() for c in s.strip('|').split('|')]
            if not s.endswith('|') or len(s) < 2:
                fail(n, 'a table row must start and end with |')
            if not table:
                table.append(cells)
                section['_columns'] = cells
            elif len(cells) != len(table[0]):
                fail(n, 'the row has %d cells, the header has %d' % (len(cells), len(table[0])))
            else:
                section['examples'].append(dict(zip(table[0], cells)))
            continue
        if section is doc:
            doc.setdefault('description', []).append(s)
            continue
        if 'examples' in section:
            fail(n, 'steps must come before the Examples')
        if not section['steps'] and not STEP_RE.match(s) and 'name' in section:
            if s.startswith('TODO:'):               # description lines of a scenario
                section['todo'] = (section.get('todo', '') + '\n' + s[5:].strip()).strip()
            else:
                section.setdefault('description', []).append(s)
            continue
        section['steps'].append(s)
        if s.startswith(DOCSTRING):
            fence, indent = s[:3], len(raw) - len(raw.lstrip())
    if fence:
        fail(len(lines), 'the doc string is not closed')
    if 'feature' not in doc:
        fail(1, 'no Feature: in the file')
    del doc['feature']
    for t in doc['tests']:
        if 'examples' in t and not t['examples']:
            raise ParseError('%s: Examples without rows' % t['name'])
        t.pop('_columns', None)
    return doc


def scenario(name, tags):
    t = {'name': name, 'steps': []}
    other = []
    for tag in tags:
        key, _, value = tag.partition(':')
        if key == 'id' and value:
            t['id'] = value
        elif key == 'ticket' and value:
            t['ticket'] = '#' + value if value.isdigit() else value
        elif key == 'project' and value:
            t.setdefault('projects', []).append(value)
        elif key == 'edition' and value:
            t.setdefault('editions', []).append(value)
        elif tag == 'disabled':
            t['enabled'] = False
        else:
            other.append(tag)
    if other:
        t['tags'] = other
    return t


def write_steps(out, lines, indent):
    """Doc strings and tables are indented below their step."""
    fence = None
    for line in lines:
        s = line.strip()
        inner = fence or s.startswith(DOCSTRING) or s.startswith('|')
        out.append((indent + '  ' * bool(inner) + line) if line else '')
        if fence and s == fence:
            fence = None
        elif not fence and s.startswith(DOCSTRING):
            fence = s[:3]


def table(rows):
    cols = list(rows[0])
    width = {c: max(len(c), *(len(r.get(c, '')) for r in rows)) for c in cols}
    fmt = lambda cells: '| ' + ' | '.join(v.ljust(width[c]) for c, v in zip(cols, cells)) + ' |'
    return [fmt(cols)] + [fmt([r.get(c, '') for c in cols]) for r in rows]


def to_gherkin(doc):
    out = []
    if doc.get('tags'):
        out.append(' '.join('@' + t for t in doc['tags']))
    out.append('Feature: ' + doc.get('title', ''))
    for line in doc.get('description', []):
        out.append('  ' + line)
    if doc.get('background'):
        out += ['', '  Background:']
        write_steps(out, doc['background'], '    ')
    for t in doc['tests']:
        tags = ['id:' + t['id']] if t.get('id') else []
        if t.get('ticket'):
            tags.append('ticket:' + t['ticket'].lstrip('#'))
        tags += ['project:' + p for p in t.get('projects', [])]
        if t.get('editions') and len(t['editions']) < len(EDITIONS):
            tags += ['edition:' + e for e in t['editions']]
        if t.get('enabled') is False:
            tags.append('disabled')
        tags += t.get('tags', [])
        out.append('')
        if tags:
            out.append('  ' + ' '.join('@' + x for x in tags))
        out.append('  %s: %s' % ('Scenario Outline' if t.get('examples') else 'Scenario', t['name']))
        for line in t.get('description', []):
            out.append('    ' + line)
        for line in (t.get('todo') or '').splitlines():
            out.append('    TODO: ' + line.strip())
        write_steps(out, t.get('steps', []), '    ')
        if t.get('examples'):
            out += ['', '    Examples:']
            out += ['      ' + row for row in table(t['examples'])]
    return '\n'.join(out) + '\n'


def legacy_steps(steps, expected):
    """Steps and expected result in plain words (markdown and older JSON files) as Gherkin lines."""
    lines = []
    for s in steps:
        if s.startswith(('* ', '- ')) and lines:
            lines[-1] += ' ' + s[2:]
        else:
            lines.append(('Given ' if not lines else 'When ' if len(lines) == 1 else 'And ') + s)
    if expected:
        lines.append('Then ' + ' '.join(expected.split()))
    return lines


def from_legacy_json(old):
    """An older JSON test file: steps, expected and a matrix of axes."""
    doc = {'title': old.get('title', ''), 'tests': []}
    for o in old.get('tests', []):
        t = {k: o[k] for k in ('id', 'name', 'ticket', 'enabled', 'todo', 'projects', 'editions', 'script', 'script_for')
             if k in o}
        matrix = o.get('matrix') or {}
        steps = legacy_steps(o.get('steps', []), o.get('expected', ''))
        t['steps'] = [re.sub(r'\{(%s)\}' % '|'.join(map(re.escape, matrix)), r'<\1>', s) if matrix else s for s in steps]
        if matrix:
            t['examples'] = [dict(zip(matrix, v)) for v in itertools.product(*matrix.values())]
        doc['tests'].append(t)
    return doc


def from_markdown(text):
    m = re.search(r'^#\s+(.+)$', text, re.M)
    doc = {'title': m.group(1).strip() if m else '', 'tests': []}
    for m in re.finditer(r'^## Test:\s*(.+?)\s*\n(.*?)(?=^## |\Z)', text, re.S | re.M):
        test = {'name': m.group(1), 'steps': [], 'expected': '', 'todo': ''}
        field = None
        for line in m.group(2).splitlines():
            s = line.strip()
            if s.startswith('Ticket:'):
                test['ticket'] = s[len('Ticket:'):].strip()
                field = None
            elif s.startswith('EXPECTED:'):
                test['expected'] = s[len('EXPECTED:'):].strip()
                field = 'expected'
            elif s.startswith('TODO:'):
                test['todo'] = s[len('TODO:'):].strip()
                field = 'todo'
            elif not s:
                field = None
            elif field:
                test[field] += '\n' + s
            else:
                test['steps'].append(re.sub(r'^\d+\.\s+', '', s))
        test['steps'] = legacy_steps(test['steps'], test.pop('expected'))
        if not test['todo']:
            del test['todo']
        doc['tests'].append(test)
    return doc


def validate(doc, root=None):
    """Return a list of error strings, empty when the document is fine. With ROOT (the directory of
    the test file) the test projects must exist."""
    errors = []
    if not isinstance(doc, dict) or not isinstance(doc.get('tests'), list):
        return ['the document needs a "tests" list']
    if not isinstance(doc.get('title', ''), str) or '\n' in doc.get('title', ''):
        errors.append('the title must be one line')
    background = doc.get('background', [])
    if not isinstance(background, list) or not all(isinstance(s, str) for s in background):
        errors.append('the background must be a list of lines')
    else:
        errors += validate_steps(background, 'Background', need_then=False)
    names = set()
    for i, t in enumerate(doc['tests'], 1):
        where = 'test %d' % i
        if not isinstance(t, dict):
            errors.append(where + ': not an object')
            continue
        name = t.get('name')
        if not isinstance(name, str) or not name.strip():
            errors.append(where + ': name is missing')
        elif '\n' in name:
            errors.append(where + ': the name must be one line')
        elif name in names:
            errors.append('%s: duplicate name "%s"' % (where, name))
        else:
            names.add(name)
        steps = t.get('steps', [])
        if not isinstance(steps, list) or not all(isinstance(s, str) and '\n' not in s for s in steps):
            errors.append(where + ': steps must be a list of lines')
        else:
            errors += validate_steps(steps, where)
        for k in ('todo', 'ticket', 'id'):
            if not isinstance(t.get(k, ''), str):
                errors.append('%s: %s must be a string' % (where, k))
        if re.search(r'\s', t.get('ticket', '') or '') or re.search(r'\W', t.get('id', '') or ''):
            errors.append(where + ': the ticket and the id can not have spaces, they are tags')
        projects = t.get('projects', [])
        if not isinstance(projects, list) or not all(isinstance(p, str) and PROJECT_RE.match(p) for p in projects):
            errors.append(where + ': projects must be a list of project names (letters, digits, _ and -)')
        elif root:
            errors += ['%s: the test project "%s" does not exist' % (where, p) for p in projects
                       if not os.path.isdir(os.path.join(root, PROJECTS, p))]
        editions = t.get('editions', list(EDITIONS))
        if (not isinstance(editions, list) or not editions or len(set(editions)) != len(editions)
                or not all(e in EDITIONS for e in editions)):
            errors.append('%s: editions must be a list of one or more of %s' % (where, ', '.join(EDITIONS)))
        errors += validate_script(t.get('script', []), where + ' script')
        examples = t.get('examples', [])
        if not isinstance(examples, list) or not all(isinstance(r, dict) for r in examples):
            errors.append(where + ': examples must be a list of rows')
            continue
        if examples:
            cols = list(examples[0])
            for c in cols:
                if not AXIS_RE.match(c):
                    errors.append('%s: Examples column "%s" must be letters, digits and _' % (where, c))
            if any(list(r) != cols for r in examples):
                errors.append(where + ': every row of the Examples must have the same columns')
            if not all(isinstance(v, str) and v and '|' not in v for r in examples for v in r.values()):
                errors.append(where + ': every cell of the Examples needs a value without |')
            if len({json.dumps(r, sort_keys=True) for r in examples}) != len(examples):
                errors.append(where + ': a row of the Examples is there twice')
    return errors


def scripts_path(path):
    return path[:-len(EXT)] + SCRIPTS_EXT if path.endswith(EXT) else path + SCRIPTS_EXT


def load(path):
    with open(path, encoding='utf-8') as f:
        text = f.read()
    if path.endswith('.md'):
        return from_markdown(text)
    if path.endswith('.json'):
        return from_legacy_json(json.loads(text))
    doc = parse(text)
    try:
        with open(scripts_path(path), encoding='utf-8') as f:
            scripts = json.load(f)
    except FileNotFoundError:
        scripts = {}
    for t in doc['tests']:
        s = scripts.get(t.get('id')) if t.get('id') else None
        if isinstance(s, dict) and s.get('script'):
            t['script'] = s['script']
            if s.get('script_for'):
                t['script_for'] = s['script_for']
    return doc


def combinations(test):
    """The rows of the Examples of a test as a list of {column: value}; [{}] when there are none."""
    return [dict(r) for r in test.get('examples') or []] or [{}]


def editions(test):
    """The editions that a test is for, all of them when it does not say."""
    return test.get('editions') or list(EDITIONS)


def substitute(text, combo):
    return re.sub(r'\{([\w:-]+)\}', lambda m: combo.get(m.group(1), m.group(0)), text)


def substitute_step(text, combo):
    """<column> of the Examples and {edition} in a line of the steps."""
    text = re.sub(r'<(\w+)>', lambda m: combo.get(m.group(1), m.group(0)), text)
    return substitute(text, {k: v for k, v in combo.items() if k == 'edition'})


def body(test, combo, edition=None, background=()):
    """The test as it is shown to Claude: the scenario in Gherkin, with the values of the run."""
    parts = []
    if test.get('ticket'):
        parts.append('Ticket: ' + test['ticket'])
    if edition:
        parts.append('Edition of the application: ' + edition)
    if combo:
        parts.append('Examples row for this run: ' + ', '.join('%s=%s' % kv for kv in combo.items()))
    combo = dict(combo, edition=edition) if edition else combo
    lines = []
    if background:
        lines += ['Background:'] + ['  ' + substitute_step(s, combo) for s in background] + ['']
    lines += ['Scenario: ' + test['name']] + ['  ' + substitute_step(s, combo) for s in test.get('steps', [])]
    parts.append('\n'.join(lines))
    if test.get('todo'):
        parts.append('TODO: ' + substitute_step(test['todo'], combo))
    return '\n\n'.join(parts)


def project_files(root, name):
    """{relative path: bytes} of a test project."""
    base = os.path.join(root, PROJECTS, name)
    out = {}
    for dirpath, _, files in os.walk(base):
        for f in files:
            p = os.path.join(dirpath, f)
            with open(p, 'rb') as fh:
                out[os.path.relpath(p, base)] = fh.read()
    return out


def project_hash(root, name):
    h = hashlib.sha1()
    for path, data in sorted(project_files(root, name).items()):
        h.update(path.encode() + b'\0' + data + b'\0')
    return h.hexdigest()[:12]


def materialize(root, names, work):
    """Copy the test projects to WORK/projects/NAME. Returns the variables for the script."""
    variables = {}
    for name in names:
        src = os.path.join(root, PROJECTS, name)
        if not os.path.isdir(src):
            raise FileNotFoundError('the test project "%s" does not exist' % name)
        dst = os.path.join(work, PROJECTS, name)
        shutil.copytree(src, dst)
        variables['project:' + name] = dst
    if len(names) == 1:
        variables['project'] = variables['project:' + names[0]]
    return variables


def definition_hash(test, root=None, background=()):
    """What a script depends on: changing any of these, the Background of the file or a file of a
    test project makes the script outdated."""
    d = {'steps': list(background or []) + list(test.get('steps') or []), 'todo': test.get('todo') or '',
         'examples': test.get('examples') or []}
    if test.get('projects'):
        d['projects'] = {p: project_hash(root, p) if root and os.path.isdir(os.path.join(root, PROJECTS, p)) else 'missing'
                         for p in test['projects']}
    return hashlib.sha1(json.dumps(d, sort_keys=True, ensure_ascii=False).encode()).hexdigest()[:12]


def ensure_ids(doc):
    for t in doc.get('tests', []):
        if not t.get('id'):
            t['id'] = 't' + hashlib.sha1(str(t.get('name', '')).encode()).hexdigest()[:8]
    return doc


def is_stale(test, root=None, background=()):
    """A script that was made for another version of the test. No script_for: old file, trusted."""
    return bool(test.get('script') and test.get('script_for')
                and test['script_for'] != definition_hash(test, root, background))


def stamp_scripts(doc, old, root=None):
    """Before saving. A new, edited or approved ("_approve") script is current for the test as it is
    now. An untouched script keeps what it was made for, so editing the test makes it outdated."""
    ensure_ids(doc)
    old = ensure_ids(old or {})
    prev = {t['id']: t for t in old.get('tests', [])}
    for t in doc['tests']:
        t.pop('_stale', None)
        approve = t.pop('_approve', False)
        if not t.get('script'):
            t.pop('script_for', None)
            continue
        p = prev.get(t['id'])
        if approve or p is None or p.get('script') != t['script']:
            t['script_for'] = definition_hash(t, root, doc.get('background'))
        else:
            t['script_for'] = p.get('script_for') or definition_hash(p, root, old.get('background'))
    return doc


def subst_deep(obj, combo):
    if isinstance(obj, str):
        return substitute(obj, combo)
    if isinstance(obj, list):
        return [subst_deep(x, combo) for x in obj]
    if isinstance(obj, dict):
        return {k: subst_deep(v, combo) for k, v in obj.items()}
    return obj


def expand(path, edition=None):
    """One dict {name, id, body, script, stale, combo} per Examples row of every enabled test.
    With EDITION only the tests for that edition, and {edition} is replaced."""
    doc = ensure_ids(load(path))
    root = os.path.dirname(os.path.abspath(path))
    errors = validate(doc, root)
    if errors:
        sys.exit('error: %s: %s' % (path, '; '.join(errors)))
    background = doc.get('background', [])
    out = []
    for t in doc['tests']:
        if not t.get('enabled', True) or edition and edition not in editions(t):
            continue
        for combo in combinations(t):
            name = t['name'] + (' [%s]' % ', '.join('%s=%s' % kv for kv in combo.items()) if combo else '')
            values = dict(combo, edition=edition) if edition else combo
            out.append({'name': name, 'id': t['id'], 'body': body(t, combo, edition, background), 'combo': combo,
                        'root': root, 'projects': t.get('projects', []), 'stale': is_stale(t, root, background),
                        'script': subst_deep(t.get('script', []), values)})
    return out


def write_atomic(path, text):
    tmp = path + '.tmp'
    with open(tmp, 'w', encoding='utf-8') as f:
        f.write(text)
    os.replace(tmp, path)


def save(path, doc):
    """Write FILE.feature and the scripts in FILE.scripts.json."""
    if not path.endswith(EXT):
        raise ValueError('a test file is saved as ' + EXT)
    ensure_ids(doc)
    scripts = {t['id']: {k: t[k] for k in ('script', 'script_for') if t.get(k)} for t in doc['tests'] if t.get('script')}
    write_atomic(path, to_gherkin(doc))
    if scripts:
        write_atomic(scripts_path(path), json.dumps(scripts, indent=2, ensure_ascii=False) + '\n')
    elif os.path.exists(scripts_path(path)):
        os.remove(scripts_path(path))


def convert(src, dst):
    """Import a markdown or an older JSON test file. Scripts that were up to date stay up to date."""
    root = os.path.dirname(os.path.abspath(src))
    current = set()
    if src.endswith('.json'):
        with open(src, encoding='utf-8') as f:
            old = json.load(f)
        for o in old.get('tests', []):
            d = {k: o.get(k) or ([] if k == 'steps' else {} if k == 'matrix' else '') for k in ('steps', 'expected', 'todo', 'matrix')}
            if o.get('projects'):
                d['projects'] = {p: project_hash(root, p) if os.path.isdir(os.path.join(root, PROJECTS, p)) else 'missing'
                                 for p in o['projects']}
            h = hashlib.sha1(json.dumps(d, sort_keys=True, ensure_ascii=False).encode()).hexdigest()[:12]
            if o.get('script') and o.get('script_for') in (None, h):
                current.add(o.get('id'))
    doc = ensure_ids(load(src))
    for t in doc['tests']:
        if t['id'] in current:
            t['script_for'] = definition_hash(t, os.path.dirname(os.path.abspath(dst)), doc.get('background'))
    save(dst, doc)


if __name__ == '__main__':
    if len(sys.argv) != 4 or sys.argv[1] != 'convert' or not sys.argv[3].endswith(EXT):
        sys.exit(__doc__)
    convert(sys.argv[2], sys.argv[3])
