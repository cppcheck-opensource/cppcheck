#!/usr/bin/env python3
"""GUI test helper for Qt applications, driven through AT-SPI.

The application runs in its own Xvfb display with its own D-Bus and
accessibility bus, so the real desktop is never touched. Each subcommand is a
separate process; the session state (environment, pids) lives in a JSON file.

  guitest.py start APP [ARGS...]     start the session and the application
  guitest.py tree                    show the accessibility tree
  guitest.py click --name Recheck    click an element
  guitest.py stop                    kill everything

State file: $GUITEST_STATE, default /tmp/guitest-<uid>/state.json
"""

import argparse
import contextlib
import io
import glob
import json
import os
import re
import shutil
import signal
import subprocess
import struct
import sys
import tempfile
import threading
import time
import warnings

warnings.filterwarnings("ignore")

STATE = os.environ.get('GUITEST_STATE', '/tmp/guitest-%d/state.json' % os.getuid())


# ---------------------------------------------------------------- session

def load_state():
    try:
        with open(STATE) as f:
            return json.load(f)
    except FileNotFoundError:
        sys.exit('error: no session, run "guitest.py start APP" first')


# the shell tells how the application ended in app.log: 139 is SIGSEGV, 134 SIGABRT, 143 killed by us
WRAPPER = '"$@" & echo $! > "$0"; wait $!; echo "application exited with status $?" >&2'


def launch(cmd, env, tmp, log):
    """Start the application. Returns the shell that waits for it and the pid of the application."""
    pidfile = os.path.join(tmp, 'app.pid')
    if os.path.exists(pidfile):
        os.remove(pidfile)
    p = subprocess.Popen(['sh', '-c', WRAPPER, pidfile] + cmd, env=env, start_new_session=True,
                         stdin=subprocess.DEVNULL, stdout=log, stderr=subprocess.STDOUT)
    for _ in range(100):
        try:
            with open(pidfile) as f:
                return p, int(f.read())
        except (OSError, ValueError):
            time.sleep(0.05)
    sys.exit('error: the application did not start')


def exit_info(state):
    """' (exited with status 139, SIGSEGV)' when the log says how the application ended, else ''."""
    try:
        with open(os.path.join(state['tmp'], 'app.log'), errors='replace') as f:
            m = re.findall(r'application exited with status (\d+)', f.read())
    except OSError:
        return ''
    if not m:
        return ''
    code = int(m[-1])
    sig = ', ' + signal.Signals(code - 128).name if 128 < code < 160 else ''
    return ' (exited with status %d%s)' % (code, sig)


def start(args):
    if os.path.exists(STATE):
        sys.exit('error: session already running (%s), run stop first' % STATE)
    app = os.path.abspath(args.app)
    if not os.access(app, os.X_OK):
        sys.exit('error: not executable: ' + app)
    for tool in ('Xvfb', 'xdotool', 'dbus-daemon', '/usr/libexec/at-spi-bus-launcher'):
        if not shutil.which(tool) and not os.path.exists(tool):
            sys.exit('error: missing ' + tool)

    tmp = tempfile.mkdtemp(prefix='guitest-')
    runtime = os.path.join(tmp, 'run')
    home = args.home or os.path.join(tmp, 'home')
    os.makedirs(runtime, mode=0o700)
    os.makedirs(home, exist_ok=True)
    # Cppcheck Premium needs its license file in HOME
    lic = os.path.join('.cppcheckpremium', 'cppcheck-premium.lic')
    src_lic = os.path.join(os.path.expanduser('~'), lic)
    dst_lic = os.path.join(home, lic)
    if os.path.isfile(src_lic) and os.path.realpath(src_lic) != os.path.realpath(dst_lic):
        os.makedirs(os.path.dirname(dst_lic), exist_ok=True)
        shutil.copy2(src_lic, dst_lic)
    pids = []

    def spawn(cmd, env, **kw):
        kw.setdefault('stdout', subprocess.DEVNULL)
        kw.setdefault('stderr', subprocess.DEVNULL)
        p = subprocess.Popen(cmd, env=env, start_new_session=True, stdin=subprocess.DEVNULL, **kw)
        pids.append(p.pid)
        return p

    env = dict(os.environ)
    env.update(XDG_RUNTIME_DIR=runtime, HOME=home)
    for k in ('XDG_CONFIG_HOME', 'XDG_DATA_HOME', 'XDG_CACHE_HOME', 'XDG_CONFIG_DIRS'):
        env.pop(k, None)    # QSettings must not find the real preferences
    env.pop('WAYLAND_DISPLAY', None)
    env.pop('DBUS_SESSION_BUS_ADDRESS', None)

    # session bus
    out = subprocess.run(['dbus-daemon', '--session', '--fork', '--print-address', '--print-pid'],
                         env=env, capture_output=True, text=True, check=True).stdout.split()
    env['DBUS_SESSION_BUS_ADDRESS'] = out[0]
    pids.append(int(out[1]))

    # X server
    r, w = os.pipe()
    spawn(['Xvfb', '-displayfd', str(w), '-screen', '0', args.size + 'x24', '-fbdir', tmp],
          env, pass_fds=(w,))
    os.close(w)
    env['DISPLAY'] = ':' + os.read(r, 16).decode().strip()
    os.close(r)

    # accessibility bus
    spawn(['/usr/libexec/at-spi-bus-launcher', '--launch-immediately'], env)
    for _ in range(100):
        if glob.glob(os.path.join(runtime, 'at-spi', 'bus_*')):
            break
        time.sleep(0.1)
    else:
        kill(pids, tmp)
        sys.exit('error: accessibility bus did not start')

    env.update(QT_QPA_PLATFORM='xcb', QT_ACCESSIBILITY='1',
               QT_LINUX_ACCESSIBILITY_ALWAYS_ON='1', QT_NO_XDG_DESKTOP_PORTAL='1',
               QT_QPA_PLATFORMTHEME='generic',      # not the KDE file dialog, its KIO jobs crash in this session
               LANG='C', LC_ALL='C')
    log = open(os.path.join(tmp, 'app.log'), 'w')
    appproc, app_pid = launch([app] + args.args, env, tmp, log)
    pids.append(appproc.pid)

    state = dict(env={k: env[k] for k in ('XDG_RUNTIME_DIR', 'HOME', 'DBUS_SESSION_BUS_ADDRESS', 'DISPLAY',
                                          'QT_QPA_PLATFORM', 'QT_ACCESSIBILITY',
                                          'QT_LINUX_ACCESSIBILITY_ALWAYS_ON', 'QT_NO_XDG_DESKTOP_PORTAL', 'QT_QPA_PLATFORMTHEME',
                                          'LANG', 'LC_ALL')},
                 pids=pids, app_pid=app_pid, app_pgid=appproc.pid, app_cmd=[app] + args.args, tmp=tmp)
    os.makedirs(os.path.dirname(STATE), exist_ok=True)
    with open(STATE, 'w') as f:
        json.dump(state, f)

    # wait for the application to show up on the accessibility bus
    os.environ.update(state['env'])
    for _ in range(100):
        if appproc.poll() is not None:
            print(open(os.path.join(tmp, 'app.log')).read())
            kill(pids, tmp)
            os.remove(STATE)
            sys.exit('error: application exited with code %s' % appproc.returncode)
        if find_app(state) is not None:
            print('started, display %s, app log %s/app.log' % (env['DISPLAY'], tmp))
            return
        time.sleep(0.2)
    sys.exit('error: application did not appear on the accessibility bus (see %s/app.log)' % tmp)


def restart(args):
    """Quit the application and start it again, same HOME so settings are kept."""
    state = load_state()
    try:
        os.killpg(state['app_pgid'], signal.SIGTERM)
    except ProcessLookupError:
        pass
    time.sleep(1)
    env = dict(os.environ)
    env.update(state['env'])
    log = open(os.path.join(state['tmp'], 'app.log'), 'a')
    p, state['app_pid'] = launch(state['app_cmd'], env, state['tmp'], log)
    state['app_pgid'] = p.pid
    state['pids'].append(p.pid)
    with open(STATE, 'w') as f:
        json.dump(state, f)
    for _ in range(100):
        if find_app(state) is not None:
            print('restarted')
            return
        time.sleep(0.2)
    sys.exit('error: application did not come back')


def kill(pids, tmp):
    for pid in reversed(pids):
        try:
            os.killpg(pid, signal.SIGTERM)
        except (ProcessLookupError, PermissionError):
            try:
                os.kill(pid, signal.SIGTERM)
            except (ProcessLookupError, PermissionError):
                pass
    time.sleep(0.3)
    shutil.rmtree(tmp, ignore_errors=True)


def stop(args):
    try:
        state = load_state()
    except SystemExit:
        return
    kill(state['pids'], state['tmp'])
    os.remove(STATE)
    print('stopped')


# ---------------------------------------------------------------- AT-SPI

_atspi = None


def atspi():
    global _atspi
    if _atspi is None:
        os.environ.update(load_state()['env'])
        import gi
        gi.require_version('Atspi', '2.0')
        from gi.repository import Atspi
        _atspi = Atspi
    return _atspi


def find_app(state):
    A = atspi()
    desktop = A.get_desktop(0)
    for i in range(desktop.get_child_count()):
        try:
            app = desktop.get_child_at_index(i)
            if app is not None and app.get_process_id() == state['app_pid']:
                return app
        except Exception:
            pass
    return None


def app_running(state):
    """The process is there (a zombie is a crashed application nobody has collected)."""
    try:
        with open('/proc/%d/stat' % state['app_pid']) as f:
            return f.read().rsplit(')', 1)[1].split()[0] != 'Z'
    except OSError:
        return False


def get_app():
    state = load_state()
    for _ in range(25):     # the application does not answer on the accessibility bus while a modal dialog opens
        app = find_app(state)
        if app is not None:
            return app
        if not app_running(state):
            break
        time.sleep(0.2)
    if app_running(state):
        sys.exit('error: the application is running but does not answer on the accessibility bus (busy or hung?)')
    sys.exit('error: the application is not running%s. log: %s/app.log' % (exit_info(state) or ' (crashed?)', state['tmp']))


def children(node):
    try:
        for i in range(node.get_child_count()):
            c = node.get_child_at_index(i)
            if c is not None:
                yield i, c
    except Exception:
        return


def states(node):
    A = atspi()
    st = node.get_state_set()
    names = []
    for label, s in (('showing', A.StateType.SHOWING), ('enabled', A.StateType.ENABLED),
                     ('focused', A.StateType.FOCUSED), ('checked', A.StateType.CHECKED),
                     ('selected', A.StateType.SELECTED), ('expanded', A.StateType.EXPANDED),
                     ('editable', A.StateType.EDITABLE), ('modal', A.StateType.MODAL)):
        if st.contains(s):
            names.append(label)
    return names


def walk(node, path=(), depth=0):
    yield path, node, depth
    for i, c in children(node):
        yield from walk(c, path + (i,), depth + 1)


def node_text(node):
    # the interface functions take the accessible as first argument, node.get_text_iface() does not work here
    A = atspi()
    try:
        if 'Text' in A.Accessible.get_interfaces(node):
            return A.Text.get_text(node, 0, min(A.Text.get_character_count(node), 2000))
    except Exception:
        pass
    return None


def node_value(node):
    A = atspi()
    try:
        if 'Value' in A.Accessible.get_interfaces(node):
            return A.Value.get_current_value(node)
    except Exception:
        pass
    return None


def extents(node):
    A = atspi()
    try:
        e = node.get_component_iface().get_extents(A.CoordType.SCREEN)
        return e.x, e.y, e.width, e.height
    except Exception:
        return None


def describe(path, node, with_geom=True):
    name = node.get_name() or ''
    s = '%s %s %r' % ('.'.join(map(str, path)) or '-', node.get_role_name(), name)
    text = node_text(node)
    if text and text != name:
        s += ' text=%r' % (text if len(text) < 200 else text[:200] + '...')
    val = node_value(node)
    if val is not None:
        s += ' value=%s' % val
    st = states(node)
    if st:
        s += ' [%s]' % ','.join(st)
    g = extents(node)
    if with_geom and g and g[2] > 0:
        s += ' @%d,%d %dx%d' % g
    return s


def cmd_tree(args):
    app = get_app()
    root, rootpath = app, ()
    if args.path:
        rootpath = tuple(int(x) for x in args.path.split('.'))
        for i in rootpath:
            root = root.get_child_at_index(i)
    for path, node, depth in walk(root, rootpath):
        d = len(path) - len(rootpath)
        if d > args.depth:
            continue
        if args.showing and 'showing' not in states(node) and d > 0:
            continue
        print('  ' * d + describe(path, node))


def cmd_windows(args):
    app = get_app()
    for i, c in children(app):
        print(describe((i,), c))


def cmd_texts(args):
    """All visible text on screen, one line per element, for checking EXPECTED."""
    app = get_app()
    for path, node, depth in walk(app):
        if 'showing' not in states(node):
            continue
        role = node.get_role_name()
        name = node.get_name() or ''
        text = node_text(node)
        if role in ('label', 'push button', 'check box', 'radio button', 'menu item', 'text', 'entry',
                    'page tab', 'combo box', 'table cell', 'tree item', 'list item', 'status bar',
                    'title bar', 'spin button', 'frame', 'dialog', 'alert') or text:
            line = '%s %s %r' % ('.'.join(map(str, path)), role, name)
            if text and text != name:
                line += ' text=%r' % text
            if 'checked' in states(node):
                line += ' [checked]'
            print(line)


# ---------------------------------------------------------------- finding

def select(args, need_showing=True):
    app = get_app()
    if args.path:
        node = app
        for i in (int(x) for x in args.path.split('.')):
            node = node.get_child_at_index(i)
            if node is None:
                sys.exit('error: no such path')
        return node
    matches = []
    for path, node, depth in walk(app):
        role = node.get_role_name()
        if args.role and args.role.lower() != role.lower():
            continue
        name = node.get_name() or ''
        text = ''       # found by name only: the text of a panel would match the message of an error
        if args.name is not None:
            want = args.name.lower()
            if args.exact:
                if name.lower() != want and text.lower() != want:
                    continue
            elif want not in name.lower() and want not in text.lower():
                continue
        elif not args.role:
            sys.exit('error: give --name, --role or --path')
        if need_showing and 'showing' not in states(node):
            continue
        matches.append((path, node))
    if not matches:
        sys.exit('error: no matching visible element (name=%r role=%r); try "tree --showing"' % (args.name, args.role))
    if args.nth is None and len(matches) > 1:
        lines = '\n'.join('  ' + describe(p, n) for p, n in matches[:15])
        sys.exit('error: %d matches, narrow with --role/--exact or pick with --nth N (0-based):\n%s' % (len(matches), lines))
    nth = args.nth or 0
    if nth >= len(matches):
        sys.exit('error: no matching visible element: only %d match(es) for name=%r role=%r, nth=%d is too high'
                 % (len(matches), args.name, args.role, nth))
    return matches[nth][1]


def xdo(*a):
    env = dict(os.environ)
    env.update(load_state()['env'])
    subprocess.run(['xdotool'] + [str(x) for x in a], env=env, check=True)


def center(node):
    g = extents(node)
    if not g or g[2] <= 0:
        sys.exit('error: element has no geometry')
    return g[0] + g[2] // 2, g[1] + g[3] // 2


def mouse(node, button=1, count=1):
    x, y = center(node)
    xdo('mousemove', x, y)
    time.sleep(0.05)
    xdo('click', '--repeat', count, '--delay', 80, button)


def do_action(node):
    A = atspi()
    try:
        act = node.get_action_iface()
        if act is None:
            return False
        for i in range(act.get_n_actions()):
            if act.get_action_name(i).lower() in ('click', 'press', 'toggle', 'activate', 'check', 'uncheck'):
                return act.do_action(i)
    except Exception:
        pass
    return False


def cmd_click(args):
    node = select(args)
    desc = describe((), node, False)
    if args.mouse:
        mouse(node)
    else:
        # the action of a button that opens a modal dialog does not return until the dialog is closed
        done = []
        t = threading.Thread(target=lambda: done.append(do_action(node)), daemon=True)
        t.start()
        t.join(args.settle + 1)
        if done == [False]:
            mouse(node)
        elif not done:
            time.sleep(args.settle)
            print('clicked ' + desc, flush=True)
            os._exit(0)
    time.sleep(args.settle)
    print('clicked ' + desc)


def cmd_rightclick(args):
    node = select(args)
    mouse(node, 3)
    time.sleep(args.settle)
    print('right-clicked ' + describe((), node, False))


def cmd_doubleclick(args):
    node = select(args)
    mouse(node, 1, 2)
    time.sleep(args.settle)
    print('double-clicked ' + describe((), node, False))


def cmd_focus(args):
    node = select(args)
    ok = node.get_component_iface().grab_focus()
    if not ok:
        mouse(node)
    print('focused ' + describe((), node, False))


def cmd_settext(args):
    node = select(args)
    et = node.get_editable_text_iface()
    if et is None:
        sys.exit('error: element is not editable text')
    et.set_text_contents(args.value)
    time.sleep(args.settle)
    print('set text of ' + describe((), node, False))


def cmd_type(args):
    xdo('type', '--delay', 20, '--', args.value)
    time.sleep(args.settle)


def cmd_key(args):
    xdo('key', *args.keys)
    time.sleep(args.settle)


def cmd_hover(args):
    x, y = center(select(args))
    xdo('mousemove', x, y)


def cmd_wait(args):
    deadline = time.time() + args.timeout
    while True:
        try:
            node = select(args)
            found = describe((), node, False)
        except SystemExit as e:
            if 'matches' in str(e):          # ambiguous means it exists
                found = str(e).split('\n')[0]
            else:
                found = None
        if args.gone != (found is not None):
            print('gone' if args.gone else 'found ' + found)
            return
        if time.time() > deadline:
            sys.exit('error: timeout waiting for element %s' % ('to disappear' if args.gone else 'to appear'))
        time.sleep(0.3)


def cmd_info(args):
    """One line about the element: role, name, text, value and [states]."""
    print(describe((), select(args), False))


def cmd_suggest(args):
    """Visible named elements, the ones most like --name first (for error messages)."""
    import difflib
    want = (args.name or '').lower()
    items = []
    for path, node, depth in walk(get_app()):
        name = (node.get_name() or '').strip()
        if name and 'showing' in states(node):
            items.append((node.get_role_name(), name))
    items.sort(key=lambda it: ((it[0].lower() != (args.role or '').lower()) if args.role else 0,
                               -difflib.SequenceMatcher(None, want, it[1].lower()).ratio()))
    seen = []
    for it in items:
        if it not in seen:
            seen.append(it)
            print('%s %r' % it)
        if len(seen) >= args.limit:
            break


def cmd_screenshot(args):
    from PIL import Image
    state = load_state()
    src = os.path.join(state['tmp'], 'Xvfb_screen0')
    data = open(src, 'rb').read()
    (hdr, _ver, _fmt, _depth, w, h, _xoff, _bo, _bu, _bbo, _bp, bpp, bpl, _vc,
     rm, gm, bm, _bpr, _ncm, ncolors) = struct.unpack('>20I', data[:80])
    off = hdr + ncolors * 12
    img = Image.frombuffer('RGB', (w, h), data[off:off + bpl * h], 'raw', 'BGRX', bpl, 1)
    img.save(args.file)
    print('saved %s (%dx%d)' % (args.file, w, h))


def cmd_ocr(args):
    """Text on the screen read by tesseract, also what AT-SPI does not show (some context menus)."""
    import tempfile
    png = os.path.join(tempfile.mkdtemp(prefix='guitest-ocr-'), 'screen.png')
    args.file = png
    with contextlib.redirect_stdout(io.StringIO()):
        cmd_screenshot(args)
    from PIL import Image
    img = Image.open(png)
    img = img.resize((img.width * 2, img.height * 2), Image.LANCZOS)
    img.save(png)
    p = subprocess.run(['tesseract', png, '-', '--psm', '11'], capture_output=True, text=True)
    if p.returncode:
        sys.exit('error: tesseract failed: ' + p.stderr.strip())
    shutil.rmtree(os.path.dirname(png), ignore_errors=True)
    print(p.stdout)


def cmd_log(args):
    print(open(os.path.join(load_state()['tmp'], 'app.log')).read())


def cmd_alive(args):
    sys.exit(0 if app_running(load_state()) else 'application is not running')


# ---------------------------------------------------------------- cli

def selector(p):
    p.add_argument('--name', help='substring of accessible name or text (case-insensitive)')
    p.add_argument('--role', help='role, e.g. "push button", "menu item", "tree item", "check box"')
    p.add_argument('--path', help='path from "tree", e.g. 0.1.2')
    p.add_argument('--nth', type=int, help='pick the Nth match (0-based)')
    p.add_argument('--exact', action='store_true', help='--name must match exactly')


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest='cmd', required=True)

    p = sub.add_parser('start', help='start session and application')
    p.add_argument('app')
    p.add_argument('args', nargs='*')
    p.add_argument('--home', help='HOME for the application (default: empty temporary dir)')
    p.add_argument('--size', default='1280x900')
    p.set_defaults(f=start)

    sub.add_parser('stop', help='stop session').set_defaults(f=stop)
    sub.add_parser('restart', help='quit and start the application again (settings are kept)').set_defaults(f=restart)
    sub.add_parser('alive', help='exit 0 if the application is running').set_defaults(f=cmd_alive)
    sub.add_parser('log', help='application stdout/stderr').set_defaults(f=cmd_log)
    sub.add_parser('windows', help='top level windows and dialogs').set_defaults(f=cmd_windows)
    sub.add_parser('texts', help='all visible texts').set_defaults(f=cmd_texts)

    p = sub.add_parser('tree', help='accessibility tree')
    p.add_argument('--depth', type=int, default=99)
    p.add_argument('--path', help='only this subtree')
    p.add_argument('--showing', action='store_true', help='only visible elements')
    p.set_defaults(f=cmd_tree)

    for name, f, h in (('click', cmd_click, 'click (uses the accessibility action when there is one)'),
                       ('rightclick', cmd_rightclick, 'right click (opens context menu)'),
                       ('doubleclick', cmd_doubleclick, 'double click'),
                       ('focus', cmd_focus, 'give keyboard focus'),
                       ('hover', cmd_hover, 'move the mouse over the element')):
        p = sub.add_parser(name, help=h)
        selector(p)
        p.add_argument('--settle', type=float, default=0.5, help='seconds to wait afterwards')
        if name == 'click':
            p.add_argument('--mouse', action='store_true', help='use a real mouse click')
        p.set_defaults(f=f)

    p = sub.add_parser('settext', help='replace the text of an edit field')
    selector(p)
    p.add_argument('value')
    p.add_argument('--settle', type=float, default=0.3)
    p.set_defaults(f=cmd_settext)

    p = sub.add_parser('type', help='type text with the keyboard into the focused element')
    p.add_argument('value')
    p.add_argument('--settle', type=float, default=0.3)
    p.set_defaults(f=cmd_type)

    p = sub.add_parser('key', help='press keys, xdotool syntax, e.g. Return ctrl+o Escape')
    p.add_argument('keys', nargs='+')
    p.add_argument('--settle', type=float, default=0.3)
    p.set_defaults(f=cmd_key)

    p = sub.add_parser('wait', help='wait until a (visible) element exists')
    selector(p)
    p.add_argument('--timeout', type=float, default=10)
    p.add_argument('--gone', action='store_true', help='wait until it disappears')
    p.set_defaults(f=cmd_wait)

    p = sub.add_parser('info', help='role, name, text, value and states of one element')
    selector(p)
    p.set_defaults(f=cmd_info)

    p = sub.add_parser('suggest', help='visible elements most similar to --name/--role')
    p.add_argument('--name')
    p.add_argument('--role')
    p.add_argument('--limit', type=int, default=8)
    p.set_defaults(f=cmd_suggest)

    p = sub.add_parser('ocr', help='print the text on the screen as read by tesseract (for what "texts" does not show)')
    p.set_defaults(f=cmd_ocr)
    p = sub.add_parser('screenshot', help='save a PNG of the screen')
    p.add_argument('file')
    p.set_defaults(f=cmd_screenshot)

    args = ap.parse_args()
    try:  # command history next to the state file, so a watcher can see what a test is doing
        with open(os.path.join(os.path.dirname(STATE), 'commands.log'), 'a') as f:
            f.write(time.strftime('%H:%M:%S ') + ' '.join(sys.argv[1:]) + '\n')
    except OSError:
        pass
    args.f(args)


if __name__ == '__main__':
    main()
