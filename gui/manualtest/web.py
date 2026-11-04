#!/usr/bin/env python3
"""Web interface for editing, generating and running the Gherkin tests.

  web.py [--opensource APP] [--premium APP] [--src DIR] [--dir tests] [--out results] [--port 8080]

Serves on 127.0.0.1 only. Reads and writes DIR/*.feature, with the scripts in DIR/*.scripts.json,
and starts gen.py and run.py with the application that is tested, one binary per edition. SRC is the git checkout that both are built
from, for the git sha in the reports. --app is the same as --opensource. Without any APP the
tests can be edited but not generated or run.
"""

import argparse
import json
import mimetypes
import os
import re
import signal
import subprocess
import sys
import shutil
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, quote, unquote, urlparse

import testfile

HERE = os.path.dirname(os.path.abspath(__file__))
NAME_RE = re.compile(r'^[\w.-]+\.feature$')
ID_RE = re.compile(r'^\w+$')


class Job:
    """gen.py or run.py in the background, one at a time. The output is shown in the web page.
    A run of several editions is one run.py after the other, each with its own report."""

    def __init__(self, kind, file, test_id, cmds, out_dir):
        self.kind, self.file, self.test_id, self.out_dir = kind, file, test_id, out_dir
        self.editions = [e for e, _ in cmds]
        self.lines, self.reports, self.rc = [], [], None
        self.started = time.time()
        self.progress = {'total': 0, 'index': 0, 'test': '', 'testfile': '', 'step': 0, 'steps': 0, 'label': '', 'base': None,
                         'edition': None, 'results': []}
        self.proc, self.stopped, self.lock = None, False, threading.Lock()
        threading.Thread(target=self.work, args=(cmds,), daemon=True).start()

    def work(self, cmds):
        rc = 0
        for edition, cmd in cmds:
            with self.lock:
                if self.stopped:
                    break
                self.edition = edition
                self.proc = subprocess.Popen(cmd, cwd=HERE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                                             env=dict(os.environ, PYTHONUNBUFFERED='1'), start_new_session=True)
            if len(cmds) > 1:
                self.lines.append('=== %s ===' % edition)
            self.read()
            rc = max(rc, self.proc.wait())
        self.rc = rc

    def stop(self):
        with self.lock:
            self.stopped = True
            if self.proc and self.proc.poll() is None:
                self.proc.send_signal(signal.SIGINT)   # run.py writes its report, gen.py cleans up

    def read(self):
        for line in self.proc.stdout:
            line = line.rstrip('\n')
            if line.startswith('@{') and self.event(line[1:]):
                continue
            self.lines.append(line)
            m = re.match(r'report: (.+)$', line)
            if m:
                self.reports.append({'edition': self.edition,
                                     'url': '/results/' + os.path.relpath(m.group(1), self.out_dir)})

    def event(self, text):
        """A progress line from run.py --events. False when it is not one."""
        try:
            e = json.loads(text)
            kind, p = e['kind'], self.progress
        except (ValueError, KeyError, TypeError):
            return False
        if kind == 'start':
            p.update(total=e['total'], index=0, test='', testfile='', step=0, steps=0, label='', edition=e.get('edition'),
                     base='/results/' + os.path.relpath(e['out'], self.out_dir))
        elif kind == 'test':
            p.update(index=e['index'], test=e['name'], testfile=e.get('file', ''), step=0, steps=e['steps'], label='')
        elif kind == 'step':
            p.update(step=e['step'], steps=e['steps'], label=e['label'])
        elif kind == 'result':
            p['results'].append(dict({k: e[k] for k in ('name', 'status', 'reason', 'seconds', 'crash', 'slug')},
                                     file=e.get('file', ''),
                                     shots=e.get('shots', []), edition=p['edition'], base=p['base']))
        return True

    def info(self):
        return {'kind': self.kind, 'file': self.file, 'id': self.test_id, 'running': self.rc is None,
                'rc': self.rc, 'output': '\n'.join(self.lines), 'reports': self.reports, 'editions': self.editions,
                'progress': self.progress, 'started': self.started, 'now': time.time()}


class Handler(BaseHTTPRequestHandler):
    directory = None
    out_dir = None
    apps = {}       # edition -> path of the binary
    src = None
    job = None
    lock = threading.Lock()

    def log_message(self, fmt, *args):
        pass

    def reply(self, code, data, ctype='application/json'):
        if ctype == 'application/json':
            data = json.dumps(data, ensure_ascii=False)
        if isinstance(data, str):
            data = data.encode()
        self.send_response(code)
        self.send_header('Content-Type', ctype + ('; charset=utf-8' if ctype.startswith(('text/', 'application/json')) else ''))
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Cache-Control', 'no-store')
        self.end_headers()
        self.wfile.write(data)

    def allowed(self):
        # only localhost names as Host (DNS rebinding) and JSON for writes (cross-site forms)
        host = self.headers.get('Host', '').rsplit(':', 1)[0]
        if host not in ('127.0.0.1', 'localhost'):
            self.reply(403, {'error': 'bad host'})
            return False
        if self.command in ('PUT', 'POST') and self.headers.get('Content-Type', '').split(';')[0] != 'application/json':
            self.reply(415, {'error': 'application/json required'})
            return False
        return True

    def path_of(self, query):
        name = parse_qs(query).get('name', [''])[0]
        if not NAME_RE.match(name):
            self.reply(400, {'error': 'bad file name (letters, digits, _ - . and .feature)'})
            return None
        return os.path.join(self.directory, name)

    def project_path(self, query):
        name = parse_qs(query).get('name', [''])[0]
        if not testfile.PROJECT_RE.match(name):
            self.reply(400, {'error': 'bad project name (letters, digits, _ and -)'})
            return None
        return os.path.join(self.directory, testfile.PROJECTS, name)

    def used_by(self, project):
        used = []
        for f in sorted(os.listdir(self.directory)):
            if NAME_RE.match(f):
                try:
                    tests = testfile.load(os.path.join(self.directory, f)).get('tests', [])
                except (ValueError, OSError):
                    continue
                used += ['%s: %s' % (f, t.get('name')) for t in tests if project in t.get('projects', [])]
        return used

    def reports(self):
        """The earlier runs in the out directory, newest first."""
        found = []
        for d in os.listdir(self.out_dir) if os.path.isdir(self.out_dir) else []:
            if not os.path.isfile(os.path.join(self.out_dir, d, 'report.html')):
                continue
            try:
                with open(os.path.join(self.out_dir, d, 'results.json'), encoding='utf-8') as f:
                    run = json.load(f)
                counts = {}
                for r in run.get('results', []):
                    counts[r['status']] = counts.get(r['status'], 0) + 1
            except (ValueError, OSError, KeyError, TypeError):
                run, counts = {}, {}
            found.append({'id': d, 'url': '/results/%s/report.html' % quote(d), 'started': run.get('started', ''),
                          'edition': run.get('edition'), 'branch': run.get('branch'), 'sha': (run.get('sha') or '')[:8],
                          'dirty': run.get('dirty'), 'aborted': run.get('aborted'), 'counts': counts,
                          'files': [f.get('title') or f.get('file') for f in run.get('files', [])]})
        return sorted(found, key=lambda r: r['id'], reverse=True)

    def annotate(self, doc):
        """Add _stale (the script is outdated) and _errors (what is wrong with the test or its script)
        to every test, for the web page. Not saved, see stamp_scripts."""
        tests = doc.get('tests') if isinstance(doc, dict) else None
        background = doc.get('background') if isinstance(doc, dict) and isinstance(doc.get('background'), list) else []
        for t in tests if isinstance(tests, list) else []:
            if not isinstance(t, dict):
                continue
            t['_stale'] = testfile.is_stale(t, self.directory, background)
            t['_errors'] = testfile.validate({'tests': [t]}, self.directory)

    def file_status(self):
        """{file: {invalid: n, stale: n}}: tests with errors and tests whose script must be generated again.
        invalid is 1 for a file that is not valid Gherkin."""
        out = {}
        for name in os.listdir(self.directory):
            if not NAME_RE.match(name):
                continue
            try:
                doc = testfile.ensure_ids(testfile.load(os.path.join(self.directory, name)))
                self.annotate(doc)
                tests = [t for t in doc['tests'] if isinstance(t, dict)]
                bad = sum(1 for t in tests if t['_errors']) + (len(tests) != len(doc['tests'])) + bool(
                    testfile.validate(dict(doc, tests=[]), self.directory))
                out[name] = {'invalid': bad, 'stale': sum(1 for t in tests if t['_stale'])}
            except (ValueError, OSError, KeyError, TypeError, AttributeError):
                out[name] = {'invalid': 1, 'stale': 0}
        return out

    def body(self):
        return json.loads(self.rfile.read(int(self.headers.get('Content-Length', 0))) or b'null')

    def do_GET(self):
        if not self.allowed():
            return
        u = urlparse(self.path)
        if u.path == '/':
            with open(os.path.join(HERE, 'web', 'index.html'), 'rb') as f:
                self.reply(200, f.read(), 'text/html')
        elif u.path == '/api/files':
            self.reply(200, sorted(f for f in os.listdir(self.directory) if NAME_RE.match(f)))
        elif u.path == '/api/file-status':
            self.reply(200, self.file_status())
        elif u.path == '/api/script-doc':
            self.reply(200, testfile.SCRIPT_DOC, 'text/plain')
        elif u.path == '/api/config':
            self.reply(200, {'apps': self.apps, 'editions': list(testfile.EDITIONS), 'can_run': bool(self.apps)})
        elif u.path == '/api/job':
            self.reply(200, self.job.info() if self.job else None)
        elif u.path == '/api/projects':
            base = os.path.join(self.directory, testfile.PROJECTS)
            names = sorted(n for n in os.listdir(base) if os.path.isdir(os.path.join(base, n))) if os.path.isdir(base) else []
            self.reply(200, [{'name': n, 'used_by': self.used_by(n)} for n in names])
        elif u.path == '/api/project':
            path = self.project_path(u.query)
            if path:
                if not os.path.isdir(path):
                    return self.reply(404, {'error': 'no such project'})
                files, binary = {}, []
                for rel, data in sorted(testfile.project_files(self.directory, os.path.basename(path)).items()):
                    try:
                        files[rel] = data.decode('utf-8')
                    except UnicodeDecodeError:
                        binary.append(rel)
                self.reply(200, {'files': files, 'binary': binary, 'used_by': self.used_by(os.path.basename(path))})
        elif u.path == '/api/reports':
            self.reply(200, self.reports())
        elif u.path.startswith('/results/'):
            self.static_file(self.out_dir, unquote(u.path[len('/results/'):]))
        elif u.path.startswith('/docs/'):
            self.static_file(os.path.join(HERE, 'docs'), unquote(u.path[len('/docs/'):]))
        elif u.path == '/api/file':
            path = self.path_of(u.query)
            if path:
                try:
                    doc = testfile.ensure_ids(testfile.load(path))
                except FileNotFoundError:
                    return self.reply(404, {'error': 'no such file'})
                except ValueError as e:
                    return self.reply(400, {'error': '%s is not valid Gherkin (fix it in a text editor): %s' % (
                        os.path.basename(path), e)})
                self.annotate(doc)
                self.reply(200, doc)
        else:
            self.reply(404, {'error': 'not found'})

    def static_file(self, base, rel):
        """A file below BASE: the results of the runs or the handbook."""
        base = os.path.realpath(base)
        path = os.path.realpath(os.path.join(base, rel))
        if not path.startswith(base + os.sep) or not os.path.isfile(path):
            return self.reply(404, {'error': 'not found'})
        ctype = 'text/plain' if path.endswith('.log') else mimetypes.guess_type(path)[0] or 'text/plain'
        with open(path, 'rb') as f:
            self.reply(200, f.read(), ctype)

    def do_PUT(self):
        if not self.allowed():
            return
        u = urlparse(self.path)
        if u.path == '/api/project':
            return self.put_project(u.query)
        path = self.path_of(u.query) if u.path == '/api/file' else None
        if not path:
            return
        try:
            doc = self.body()
        except ValueError:
            return self.reply(400, {'errors': ['not valid JSON']})
        errors = testfile.validate(doc, self.directory)
        if errors:
            return self.reply(400, {'errors': errors})
        try:
            old = testfile.load(path)
        except (FileNotFoundError, ValueError):
            old = None
        testfile.save(path, testfile.stamp_scripts(doc, old, self.directory))
        self.reply(200, {'saved': os.path.basename(path)})

    def put_project(self, query):
        path = self.project_path(query)
        if not path:
            return
        try:
            files = self.body()['files']
            assert isinstance(files, dict) and all(isinstance(v, str) for v in files.values())
        except (ValueError, KeyError, TypeError, AssertionError):
            return self.reply(400, {'errors': ['expected {"files": {"path": "content"}}']})
        clean = {}
        for rel, content in files.items():
            norm = os.path.normpath(rel)
            if (not rel or os.path.isabs(norm) or norm.startswith('..') or '\0' in rel or norm == '.'
                    or norm.split(os.sep)[0] == '..'):
                return self.reply(400, {'errors': ['bad file path: %r (relative, inside the project)' % rel]})
            clean[norm] = content
        if sum(len(c) for c in clean.values()) > 4_000_000:
            return self.reply(400, {'errors': ['the project is too big for this editor']})
        os.makedirs(path, exist_ok=True)
        # text files that are not in the request were removed in the editor, binary files stay
        for rel, data in testfile.project_files(self.directory, os.path.basename(path)).items():
            if rel not in clean:
                try:
                    data.decode('utf-8')
                except UnicodeDecodeError:
                    continue
                os.remove(os.path.join(path, rel))
        for rel, content in clean.items():
            target = os.path.join(path, rel)
            os.makedirs(os.path.dirname(target), exist_ok=True)
            with open(target + '.tmp', 'w', encoding='utf-8', newline='') as f:
                f.write(content)
            os.replace(target + '.tmp', target)
        for dirpath, dirs, fs in os.walk(path, topdown=False):      # directories that became empty
            if dirpath != path and not os.listdir(dirpath):
                os.rmdir(dirpath)
        self.reply(200, {'saved': os.path.basename(path), 'files': len(clean)})

    def do_POST(self):
        if not self.allowed():
            return
        route = urlparse(self.path).path
        try:
            data = self.body()
            if route == '/api/job':
                self.start_job(data)
            elif route == '/api/job/stop':
                if self.job and self.job.rc is None:
                    self.job.stop()
                self.reply(200, {'stopping': True})
            else:
                self.reply(404, {'error': 'not found'})
        except (ValueError, KeyError, TypeError):
            self.reply(400, {'errors': ['bad request']})

    def start_job(self, data):
        action, name, test_id, edition = data['action'], data['file'], data.get('id'), data.get('edition')
        if not self.apps:
            return self.reply(400, {'errors': ['web.py was started without --opensource or --premium']})
        everything = action == 'run' and name == '*'     # every test file in the directory
        if action not in ('generate', 'run') or not (everything or NAME_RE.match(name)) or (
                test_id and not ID_RE.match(test_id)):
            return self.reply(400, {'errors': ['bad request']})
        if action == 'generate' and not test_id:
            return self.reply(400, {'errors': ['generate needs a test id']})
        if everything:
            paths = sorted(os.path.join(self.directory, n) for n in os.listdir(self.directory) if NAME_RE.match(n))
            if not paths or test_id:
                return self.reply(404, {'errors': ['no test files']})
        else:
            paths = [os.path.join(self.directory, name)]
            if not os.path.isfile(paths[0]):
                return self.reply(404, {'errors': ['no such file']})
        path = paths[0]
        # the editions of the job: the chosen ones that web.py has a binary for and, for one test, that it is for
        editions = [e for e in testfile.EDITIONS if e in self.apps and edition in (e, 'both')]
        if test_id:
            test = next((t for t in testfile.ensure_ids(testfile.load(path)).get('tests', []) if t['id'] == test_id), None)
            if not test:
                return self.reply(404, {'errors': ['no such test']})
            allowed = [e for e in testfile.editions(test) if e in self.apps]
            if action == 'generate' and not set(editions) & set(allowed):
                editions = allowed      # the script is the same for all editions, make it with one that we have
            editions = [e for e in editions if e in allowed]
        if not editions:
            return self.reply(400, {'errors': ['the test is not for the edition %s, or web.py has no application for it' % (edition or '')]})
        if action == 'generate':
            editions = editions[:1]
        cmds = []
        for e in editions:
            app = self.apps[e]
            if action == 'generate':
                cmd = [sys.executable, 'gen.py', app, path, '--force', '--id', test_id, '--edition', e]
            else:
                cmd = [sys.executable, 'run.py', app, *paths, '--out', self.out_dir, '--events', '--edition', e]
                cmd += ['--src', self.src] if self.src else []
                cmd += ['--id', test_id] if test_id else []
            cmds.append((e, cmd))
        with self.lock:
            if self.job and self.job.rc is None:
                return self.reply(409, {'errors': ['a job is already running']})
            Handler.job = Job(action, name, test_id, cmds, self.out_dir)
        self.reply(200, self.job.info())

    def do_DELETE(self):
        if not self.allowed():
            return
        u = urlparse(self.path)
        if u.path == '/api/project':
            path = self.project_path(u.query)
            if path:
                used = self.used_by(os.path.basename(path))
                if used:
                    return self.reply(409, {'errors': ['the project is used by: ' + '; '.join(used)]})
                if not os.path.isdir(path):
                    return self.reply(404, {'error': 'no such project'})
                shutil.rmtree(path)
                self.reply(200, {'deleted': os.path.basename(path)})
            return
        path = self.path_of(u.query) if u.path == '/api/file' else None
        if path:
            try:
                os.remove(path)
                if os.path.exists(testfile.scripts_path(path)):
                    os.remove(testfile.scripts_path(path))
                self.reply(200, {'deleted': os.path.basename(path)})
            except FileNotFoundError:
                self.reply(404, {'error': 'no such file'})


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    for e in testfile.EDITIONS:
        ap.add_argument('--' + e, *(['--app'] if e == 'opensource' else []), metavar='APP',
                        help='the %s edition of the application to test' % e)
    ap.add_argument('--src', help='git checkout of the application, the same for all editions (default: the directory of APP)')
    ap.add_argument('--dir', default=os.path.join(HERE, 'tests'))
    ap.add_argument('--out', default=os.path.join(HERE, 'results'))
    ap.add_argument('--port', type=int, default=8080)
    args = ap.parse_args()
    for e in testfile.EDITIONS:
        app = getattr(args, e)
        if app:
            if not os.access(app, os.X_OK):
                sys.exit('error: not executable: ' + app)
            Handler.apps[e] = os.path.abspath(app)
    Handler.src = os.path.abspath(args.src) if args.src else None
    Handler.directory = os.path.abspath(args.dir)
    Handler.out_dir = os.path.realpath(args.out)
    os.makedirs(Handler.directory, exist_ok=True)
    os.makedirs(Handler.out_dir, exist_ok=True)
    Handler.out_dir = os.path.realpath(Handler.out_dir)
    server = ThreadingHTTPServer(('127.0.0.1', args.port), Handler)
    print('http://127.0.0.1:%d/  (%s)' % (args.port, Handler.directory), flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    main()
