import argparse
import contextlib
import multiprocessing
import os
import tempfile
import shutil
import subprocess

@contextlib.contextmanager
def mkdtemp():
    d = tempfile.mkdtemp()
    yield d
    shutil.rmtree(d, ignore_errors=True)

def print_lines(lines):
    for line in lines:
        print(line)

def write_to(file, lines):
    content = list((line + "\n" for line in lines))
    if (len(content) > 0):
        with open(file, 'w') as f:
            f.writelines(content)

def make_executable(p):
    os.chmod(p, 509)

def quote(s):
    text = s.replace("'", "'\"'\"'")
    return "'{}'".format(text)

class ScriptBuilder:
    def __init__(self):
        self.commands = ['#!/bin/bash', '', 'rm -rf .tmp.output', '']

    def blank(self):
        self.commands.append('')

    def add_command(self, cmd, save=False):
        if save:
            self.commands.append(cmd + ' 2>&1 | tee .tmp.output')
        else:
            self.commands.append(cmd)

    def grep(self, text, file=None):
        self.add_command("grep -q -F {} {}".format(quote(text), file or '.tmp.output'))

    def check(self, equal_zero=False, result=1):
        op = 'eq' if equal_zero else 'ne'
        cmds = ['RES=$?',
                'if [ $RES -{} "0" ]; then'.format(op),
                '    exit {}'.format(result),
                'fi']
        self.commands.extend(cmds)

    def write(self, p):
        write_to(p, self.commands)
        make_executable(p)

    def show(self):
        print_lines(self.commands)


parser = argparse.ArgumentParser()
parser.add_argument('--file', '-f', help='file to reduce', required=True)
parser.add_argument('--text', '-t', action='append', help='expected output text')
parser.add_argument('--keep', '-k', action='append', help='text to keep in source file')
parser.add_argument('--check', '-c', action='append', help='command to check syntax')
parser.add_argument('--dry-run', '-d', action='store_true', default=False, help='dry run')
parser.add_argument('--timeout', action='store', default=300, help='Interestingness test timeout in seconds')
parser.add_argument('--no-cache', action='store_true', default=False, help="Don't cache behavior of passes")
parser.add_argument('--no-give-up', action='store_true', default=False, help="Don't give up on a pass that hasn't made progress for 50000 iterations")
parser.add_argument('--sllooww', '--slow', action='store_true', default=False, help="Try harder to reduce, but perhaps take a long time to do so")
parser.add_argument('command')
parser.add_argument('args', nargs=argparse.REMAINDER)
args = parser.parse_args()

file = os.path.basename(args.file)
sb = ScriptBuilder()
for keep in args.keep or []:
    sb.grep(keep, file=file)
    sb.check()
    sb.blank()
for check in args.check or []:
    sb.add_command(check+' '+file)
    sb.check()
    sb.blank()
command = ' '.join([os.path.abspath(args.command)]+args.args+[file])
sb.add_command(command, save=args.text)
if args.text:
    for t in args.text:
        sb.grep(t)
        sb.check()
else:
    sb.check(equal_zero=True)
sb.show()
with mkdtemp() as d:
    script = os.path.join(d, 'reduce.sh')
    sb.write(script)
    if args.dry_run:
        shutil.copy(args.file, d)
        subprocess.check_call([script], cwd=d)
    else:
        creduce = ['creduce', '--n', str(multiprocessing.cpu_count())]
        creduce.append("--timeout")
        creduce.append(str(args.timeout))
        if args.no_cache:
            creduce.append("--no-cache")
        if args.no_give_up:
            creduce.append("--no-give-up")
        if args.sllooww:
            creduce.append("--sllooww")
        subprocess.check_call(creduce + [script, args.file])
