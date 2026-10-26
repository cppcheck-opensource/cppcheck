"""Executes a test script (see testfile.SCRIPT_DOC) with guitest.py commands, no AI involved.

run_script() gets `sub`, a function that runs one guitest.py command in the session of the test:
sub(cmd, *args, capture_output=True) -> CompletedProcess with returncode, stdout, stderr.
"""

import os
import re
import shutil
import time

import testfile

# actions that change the GUI, a screenshot is taken after each of them
GUI_ACTIONS = ('click', 'rightclick', 'doubleclick', 'focus', 'hover', 'settext', 'type', 'key', 'restart')
NO_ELEMENT = ('no matching visible element', 'does not answer on the accessibility bus')
DEFAULT_TIMEOUT = 5


def selector_args(step):
    a = []
    for k in ('name', 'role', 'path'):
        if k in step:
            a += ['--' + k, str(step[k])]
    if 'nth' in step:
        a += ['--nth', str(step['nth'])]
    if step.get('exact'):
        a.append('--exact')
    return a


def describe(step):
    return ' '.join('%s=%r' % (k, v) for k, v in step.items() if k != 'label')


def message(p):
    text = (p.stderr.strip() or p.stdout.strip() or 'exit code %d' % p.returncode).removeprefix('error: ')
    return text.replace('; try "tree --showing"', '')


class StepFailed(Exception):
    pass


def check_path(path, allowed):
    path = os.path.realpath(path)
    if not any(path == d or path.startswith(d + os.sep) for d in (os.path.realpath(x) for x in allowed)):
        raise StepFailed('path %s is outside the test directories' % path)
    return path


def gui(sub, cmd, step, extra=()):
    """Run a command that needs an element. The element may not have appeared yet, so retry."""
    deadline = time.time() + step.get('timeout', DEFAULT_TIMEOUT)
    while True:
        p = sub(cmd, *selector_args(step), *extra, capture_output=True)
        if p.returncode == 0:
            return p
        if not any(m in message(p) for m in NO_ELEMENT) or time.time() > deadline:
            raise StepFailed(message(p))
        time.sleep(0.4)


def run_step(sub, step, dirs, captured):
    action = step.get('do') or step['expect']
    if 'do' in step:
        if action in ('click', 'rightclick', 'doubleclick', 'focus', 'hover'):
            gui(sub, action, step, ['--mouse'] if step.get('mouse') else [])
        elif action == 'settext':
            gui(sub, 'settext', step, [step['value']])
        elif action == 'type':
            ok(sub('type', step['value'], capture_output=True))
        elif action == 'key':
            keys = step['keys'].split() if isinstance(step['keys'], str) else step['keys']
            ok(sub('key', *keys, capture_output=True))
        elif action == 'wait':
            ok(sub('wait', *selector_args(step), '--timeout', str(step.get('timeout', 10)),
                   *(['--gone'] if step.get('gone') else []), capture_output=True))
        elif action == 'restart':
            ok(sub('restart', capture_output=True))
        elif action == 'sleep':
            time.sleep(float(step['seconds']))
        elif action == 'copy':
            src, dst = check_path(step['from'], dirs), check_path(step['to'], dirs)
            if not os.path.isfile(src):
                raise StepFailed('file %s does not exist' % src)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copyfile(src, dst)
        elif action == 'write':
            path = check_path(step['path'], dirs)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, 'w', encoding='utf-8') as f:
                f.write(step['content'])
        return
    if action == 'visible':
        ok(sub('wait', *selector_args(step), '--timeout', str(step.get('timeout', DEFAULT_TIMEOUT)),
               capture_output=True))
    elif action == 'gone':
        ok(sub('wait', *selector_args(step), '--gone', '--timeout', str(step.get('timeout', DEFAULT_TIMEOUT)),
               capture_output=True))
    elif action == 'state':
        info = gui(sub, 'info', step).stdout
        have = set((re.search(r'\[([\w,]*)\]\s*$', info.strip()) or [None, ''])[1].split(','))
        if 'state' in step and step['state'] not in have:
            raise StepFailed('element is not %s, its states are [%s]: %s' % (step['state'], ','.join(sorted(have)), info.strip()))
        if 'not_state' in step and step['not_state'] in have:
            raise StepFailed('element is %s but should not be: %s' % (step['not_state'], info.strip()))
    elif action == 'value':
        info = gui(sub, 'info', step).stdout.strip()
        found = re.findall(r" value=(\S+)", info) + re.findall(r" text='(.*?)'(?= value=| \[|$)", info)
        if not any(same(v, step['equals']) for v in found):
            raise StepFailed('the value is %s, not %r: %s' % (found or 'missing', step['equals'], info))
    elif action == 'exited':
        deadline = time.time() + step.get('timeout', DEFAULT_TIMEOUT)
        while sub('alive', capture_output=True).returncode == 0:
            if time.time() > deadline:
                raise StepFailed('the application is still running')
            time.sleep(0.3)
        status = re.findall(r'application exited with status (\d+)', sub('log', capture_output=True).stdout)
        if status and status[-1] != '0':
            raise StepFailed('the application ended with status %s, not 0' % status[-1])
    elif action == 'text':
        deadline = time.time() + step.get('timeout', DEFAULT_TIMEOUT)    # the screen may take a moment to change
        while True:
            texts = sub('texts', capture_output=True).stdout
            problem = None
            if 'contains' in step and step['contains'] not in texts:
                problem = 'the text %r is not on the screen' % step['contains']
            elif 'not_contains' in step and step['not_contains'] in texts:
                problem = 'the text %r is on the screen but should not be' % step['not_contains']
            if not problem:
                break
            if time.time() > deadline:
                raise StepFailed(problem)
            time.sleep(0.4)
    elif action == 'ocr':
        deadline = time.time() + step.get('timeout', DEFAULT_TIMEOUT)
        norm = lambda x: re.sub(r'\s+', ' ', x).lower()
        while True:
            p = sub('ocr', capture_output=True)
            if p.returncode:
                raise StepFailed(message(p))
            seen = norm(p.stdout)
            problem = None
            if 'contains' in step and norm(step['contains']) not in seen:
                problem = 'the text %r is not on the screen (OCR)' % step['contains']
            elif 'not_contains' in step and norm(step['not_contains']) in seen:
                problem = 'the text %r is on the screen (OCR) but should not be' % step['not_contains']
            if not problem:
                break
            if time.time() > deadline:
                raise StepFailed(problem + '. OCR read: ' + seen[:300])
            time.sleep(0.4)
    elif action == 'file':
        path = check_path(step['path'], dirs)
        try:
            with open(path, encoding='utf-8', errors='replace') as f:
                text = f.read()
        except FileNotFoundError:
            raise StepFailed('file %s does not exist' % path)
        if 'contains' in step and step['contains'] not in text:
            raise StepFailed('%s does not contain %r' % (path, step['contains']))
        if 'not_contains' in step and step['not_contains'] in text:
            raise StepFailed('%s contains %r but should not' % (path, step['not_contains']))
        if 'regex' in step and not re.search(step['regex'], text, re.M):
            raise StepFailed('%s does not match /%s/' % (path, step['regex']))
    elif action == 'alive':
        ok(sub('alive', capture_output=True))
    elif action == 'capture':
        info = gui(sub, 'info', step).stdout.strip()
        m = re.search(step['regex'], info)
        if not m:
            raise StepFailed('/%s/ does not match: %s' % (step['regex'], info))
        captured[step['as']] = m.group(1) if m.groups() else m.group(0)


def same(a, b):
    try:
        return float(a) == float(b)       # a spin box says 1.0
    except ValueError:
        return a == b


def ok(p):
    if p.returncode:
        raise StepFailed(message(p))


def context(sub, step):
    """What the screen looks like when a step failed, to make a renamed button easy to find."""
    out = []
    if {'name', 'role', 'path'} & set(step):
        cmd = ['suggest'] + ['--%s=%s' % (k, step[k]) for k in ('name', 'role') if k in step]
        s = sub(*cmd, capture_output=True).stdout.strip()
        if s:
            out.append('most similar visible elements:\n' + '\n'.join('    ' + l for l in s.splitlines()))
    w = sub('windows', capture_output=True).stdout.strip()
    if w:
        out.append('windows:\n' + '\n'.join('    ' + l.split(' @')[0] for l in w.splitlines()[:6]))
    return '\n  '.join(out)



def run_script(sub, script, work, home, variables=None, progress=None, shot=None):
    """Returns (ok, message, log). On failure the message names the step and what was on screen.
    shot(i, label) is called after each step i that is one of GUI_ACTIONS and returns the name of the
    screenshot it saved, or None."""
    dirs = [work, home]
    variables = dict(variables or {}, work=work, home=home)
    captured = {}       # {name} values of the capture steps so far
    log = []
    for i, step in enumerate(script, 1):
        step = testfile.subst_deep(step, dict(variables, **captured))
        t0 = time.time()
        label = step.get('label')
        if progress:
            progress(i, len(script), label or describe(step))
        head = 'step %d/%d%s: %s' % (i, len(script), ' "%s"' % label if label else '', describe(step))
        try:
            run_step(sub, step, dirs, captured)
        except StepFailed as e:
            log.append('FAIL %s (%.1f s)' % (head, time.time() - t0))
            msg = '%s\n  %s' % (head, e)
            ctx = context(sub, step)
            return False, msg + ('\n  ' + ctx if ctx else ''), '\n'.join(log) + '\n' + msg + '\n'
        log.append('ok   %s (%.1f s)' % (head, time.time() - t0))
        if shot and step.get('do') in GUI_ACTIONS:
            time.sleep(0.2)    # let the application paint
            name = shot(i, label or describe(step))
            if name:
                log[-1] += ' screenshot ' + name
    return True, 'all %d steps passed' % len(script), '\n'.join(log) + '\n'
