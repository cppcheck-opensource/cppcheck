"""
usage: reduce.py [-h] -exe EXE -exe_args EXE_ARGS -expected EXPECTED -file FILE [-segfault]

options:
  -h, --help            show this help message and exit
  -exe EXE, --exe EXE   cppcheck executable
  -exe_args EXE_ARGS, --exe_args EXE_ARGS
                        cppcheck executable commands
  -expected EXPECTED, --expected EXPECTED
                        expected text output
  -file FILE, --file FILE
                        source file
  -segfault, --segfault
"""

#!/usr/bin/env python3
import subprocess
import sys
import time
import argparse


class Reduce:
    def __init__(self, cmd, expected, file, segfault=None):
        if not "".join(cmd):
            raise RuntimeError('Abort: No --cmd')

        if not segfault and not expected:
            raise RuntimeError('Abort: No --expected')

        if not file:
            raise RuntimeError('Abort: No --file')

        # need to add '--error-exitcode=0' so detected issues will not be interpreted as a crash
        if segfault and '--error-exitcode=0' not in cmd:
            print("Adding '--error-exitcode=0' to --cmd")
            self.__cmd = cmd + ['--error-exitcode=0']
        else:
            self.__cmd = cmd
        self.__expected = expected
        self.__file = file
        self.__segfault = segfault
        self.__origfile = self.__file + '.org'
        self.__backupfile = self.__file + '.bak'
        self.__timeoutfile = self.__file + '.timeout'
        self.__elapsed_time = None

    def print_info(self):
        print('CMD=', " ".join(self.__cmd))
        if self.__segfault:
            print('EXPECTED=SEGFAULT')
        else:
            print('EXPECTED=' + self.__expected)
        print('FILE=' + self.__file)

    def __communicate(self, p, timeout=None, **kwargs):
        return p.communicate(timeout=timeout)

    def runtool(self, filedata=None):
        TimeoutExpired = subprocess.TimeoutExpired

        timeout = None
        if self.__elapsed_time:
            timeout = self.__elapsed_time * 2
        p = subprocess.Popen(self.__cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        try:
            stdout, stderr = self.__communicate(p, timeout=timeout)
        except TimeoutExpired:
            print('timeout')
            p.kill()
            p.communicate()
            if filedata:
                self.writetimeoutfile(filedata)
            return False
        # print(p.returncode)
        # print(comm)
        if self.__segfault:
            if p.returncode != 0:
                return True
        elif p.returncode == 0:
            out = stdout + '\n' + stderr
            if self.__expected in out:
                return True
        else:
            # Something could be wrong, for example the command line for Cppcheck (CMD).
            # Print the output to give a hint how to fix it.
            print('Error: {}\n{}'.format(stdout, stderr))
        return False

    def __writefile(self, filename, filedata):
        with open(filename, 'wt') as f:
            for line in filedata:
                f.write(line)

    def replaceandrun(self, what, filedata, i, line):
        print(what + ' ' + str(i + 1) + '/' + str(len(filedata)) + '..')
        bak = filedata[i]
        filedata[i] = line
        self.writefile(filedata)
        if self.runtool(filedata):
            print('pass')
            self.writebackupfile(filedata)
            return True
        print('fail')
        filedata[i] = bak
        return False

    def replaceandrun2(self, what, filedata, i, line1, line2):
        print(what + ' ' + str(i + 1) + '/' + str(len(filedata)) + '..')
        bak1 = filedata[i]
        bak2 = filedata[i + 1]
        filedata[i] = line1
        filedata[i + 1] = line2
        self.writefile(filedata)
        if self.runtool(filedata):
            print('pass')
            self.writebackupfile(filedata)
        else:
            print('fail')
            filedata[i] = bak1
            filedata[i + 1] = bak2

    def clearandrun(self, what, filedata, i1, i2):
        print(what + ' ' + str(i1 + 1) + '/' + str(len(filedata)) + '..')
        filedata2 = list(filedata)
        i = i1
        while i <= i2 and i < len(filedata2):
            filedata2[i] = ''
            i = i + 1
        self.writefile(filedata2)
        if self.runtool(filedata2):
            print('pass')
            self.writebackupfile(filedata2)
            return filedata2
        print('fail')
        return filedata

    def removecomments(self, filedata):
        for i in range(len(filedata)):
            line = filedata[i]
            if '//' in line:
                self.replaceandrun('remove comment', filedata, i, line[:line.find('//')].rstrip() + '\n')

    def checkpar(self, line):
        par = 0
        for c in line:
            if c == '(' or c == '[':
                par = par + 1
            elif c == ')' or c == ']':
                par = par - 1
                if par < 0:
                    return False
        return par == 0

    def combinelines(self, filedata):
        if len(filedata) < 3:
            return filedata

        lines = []

        for i in range(len(filedata) - 1):
            fd1 = filedata[i].rstrip()
            if fd1.endswith(','):
                fd2 = filedata[i + 1].lstrip()
                if fd2 != '':
                    lines.append(i)

        chunksize = len(lines)
        while chunksize > 10:
            i = 0
            while i < len(lines):
                i1 = i
                i2 = i + chunksize
                i = i2
                i2 = min(i2, len(lines))

                filedata2 = list(filedata)
                for line in lines[i1:i2]:
                    filedata2[line] = filedata2[line].rstrip() + filedata2[line + 1].lstrip()
                    filedata2[line + 1] = ''

                if self.replaceandrun('combine lines (chunk)', filedata2, lines[i1] + 1, ''):
                    filedata = filedata2
                    lines[i1:i2] = []
                    i = i1

            chunksize = int(chunksize / 2)

        for line in lines:
            fd1 = filedata[line].rstrip()
            fd2 = filedata[line + 1].lstrip()
            self.replaceandrun2('combine lines', filedata, line, fd1 + fd2, '')

        return filedata

    def removedirectives(self, filedata):
        for i in range(len(filedata)):
            line = filedata[i].lstrip()
            if line.startswith('#'):
                # these cannot be removed on their own so skip them
                if line.startswith('#if') or line.startswith('#endif') or line.startswith('#el'):
                    continue
                self.replaceandrun('remove preprocessor directive', filedata, i, '')

    def removeblocks(self, filedata):
        if len(filedata) < 3:
            return filedata

        for i in range(len(filedata)):
            strippedline = filedata[i].strip()
            if len(strippedline) == 0:
                continue
            if strippedline[-1] not in ';{}':
                continue

            i1 = i + 1
            while i1 < len(filedata) and filedata[i1].startswith('#'):
                i1 = i1 + 1

            i2 = i1
            indent = 0
            while i2 < len(filedata):
                for c in filedata[i2]:
                    if c == '}':
                        indent = indent - 1
                        if indent == 0:
                            indent = -100
                    elif c == '{':
                        indent = indent + 1
                if indent < 0:
                    break
                i2 = i2 + 1
            if indent == -100:
                indent = 0
            if i2 == i1 or i2 >= len(filedata):
                continue
            if filedata[i2].strip() != '}' and filedata[i2].strip() != '};':
                continue
            if indent < 0:
                i2 = i2 - 1
            filedata = self.clearandrun('remove codeblock', filedata, i1, i2)

        return filedata

    def removeline(self, filedata):
        stmt = True
        for i in range(len(filedata)):
            line = filedata[i]
            strippedline = line.strip()

            if len(strippedline) == 0:
                continue

            if stmt and strippedline[-1] == ';' and self.checkpar(line) and '{' not in line and '}' not in line:
                self.replaceandrun('remove line', filedata, i, '')

            elif stmt and '{' in strippedline and strippedline.find('}') == len(strippedline) - 1:
                self.replaceandrun('remove line', filedata, i, '')

            stmt = strippedline[-1] in ';{}'

    def set_elapsed_time(self, elapsed_time):
        self.__elapsed_time = elapsed_time

    def writefile(self, filedata):
        self.__writefile(self.__file, filedata)

    def writeorigfile(self, filedata):
        self.__writefile(self.__origfile, filedata)

    def writebackupfile(self, filedata):
        self.__writefile(self.__backupfile, filedata)

    def writetimeoutfile(self, filedata):
        self.__writefile(self.__timeoutfile, filedata)


def main():
    # TODO: add --hang option to detect code which impacts the analysis time
    def show_syntax():
        print('Syntax:')
        print('  reduce.py --exe <cppcheck executable> --exe_args <full command> --expected <expected text output> --file <source file> [--segfault]')
        print('')
        print("Example. source file = foo/bar.c")
        print(
            '  reduce.py --exe ./cppcheck --exe_args " --enable=style" --expected "Variable \'x\' is reassigned" --file foo/bar.c')
        sys.exit(1)

    if len(sys.argv) == 1:
        show_syntax()
    parser = argparse.ArgumentParser()
    parser.add_argument('-exe', '--exe', required=True, help="cppcheck executable")
    parser.add_argument('-exe_args', '--exe_args', required=False, default="", help="cppcheck executable commands")
    parser.add_argument('-expected', '--expected', required=True, help="expected text output")
    parser.add_argument('-file', '--file', required=True, help="source file")
    parser.add_argument('-segfault', '--segfault', required=False, action='store_true')
    args = parser.parse_args()

    arg_file = args.file
    arg_cmd = [args.exe] + args.exe_args.split() + [arg_file]
    arg_expected = args.expected
    arg_segfault = args.segfault
    try:
        reduce = Reduce(arg_cmd, arg_expected, arg_file, arg_segfault)
    except RuntimeError as e:
        print(e)
        show_syntax()

    reduce.print_info()

    # reduce..
    print('Make sure error can be reproduced...')
    t = time.time()
    if not reduce.runtool():
        print("Cannot reproduce")
        sys.exit(1)
    elapsed_time = time.time() - t
    reduce.set_elapsed_time(elapsed_time)
    print('elapsed_time: {}'.format(elapsed_time))

    with open(arg_file, 'rt') as f:
        filedata = f.readlines()

    reduce.writeorigfile(filedata)

    while True:
        filedata1 = list(filedata)

        print('remove preprocessor directives...')
        reduce.removedirectives(filedata)

        print('remove blocks...')
        filedata = reduce.removeblocks(filedata)

        print('remove comments...')
        reduce.removecomments(filedata)

        print('combine lines..')
        filedata = reduce.combinelines(filedata)

        print('remove line...')
        reduce.removeline(filedata)

        # if filedata and filedata2 are identical then stop
        if filedata1 == filedata:
            break

    reduce.writefile(filedata)
    print('DONE')


if __name__ == '__main__':
    main()
