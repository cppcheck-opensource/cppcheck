# Example usage:
# python3 compare_ast.py lib/token.cpp
# python3 compare_ast.py "--project=compile_commands.json --file-filter=*/lib/somefile.c"
# check all files in a compile_commands.json:
# grep '"file":' compile_commands.json | grep -v test | sed 's|.*/curl/|"--project=compile_commands.json --file-filter=*|' | xargs python3 ~/cppcheck/tools/compare_ast.py

import os
import re
import sys
import subprocess

CPPCHECK = os.path.expanduser('~/cppcheck/cppcheck')

def run_cppcheck(cppcheck_parameters:str, clang:str):
    cmd = '{} {} {} --debug --verbose'.format(CPPCHECK, cppcheck_parameters, clang)
    #print(cmd)
    with subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        # TODO: handle p.returncode?
        stdout, _ = p.communicate()
    return stdout.decode('utf-8', 'ignore')


def get_ast(debug):
    debug = re.sub(r' f:0x[0-9a-fA-F]+', '', debug)
    debug = re.sub(r" '[a-zA-Z0-9: *]+'", '', debug)

    pos1 = debug.rfind('\n##AST\n')
    if pos1 < 0:
        return ''
    pos1 = debug.find('\n', pos1) + 1
    assert pos1 > 0
    pos2 = debug.find("\n##", pos1)
    if pos2 < 0:
        return debug[pos1:]
    return debug[pos1:pos2-1]


def get_symdb(debug):
    debug = re.sub(r'0x[0-9a-fA-F]+', '0x12345678', debug)
    debug = re.sub(r'nestedIn: Struct', 'nestedIn: Class', debug)
    debug = re.sub(r'classDef: struct', 'classDef: class', debug)
    debug = re.sub(r'isInline: [a-z]+', 'isInline: ---', debug)
    debug = re.sub(r'definedType: .*', 'definedType: ---', debug)
    debug = re.sub(r'needInitialization: .*', 'needInitialization: ---', debug)
    debug = re.sub(r'functionOf: .*', 'functionOf: ---', debug)
    debug = re.sub(r'0x12345678 Struct', '0x12345678 Class', debug)

    pos1 = debug.rfind('\n### Symbol database ###\n')
    if pos1 < 0:
        return ''
    pos1 = debug.find('\n', pos1) + 1
    assert pos1 > 0
    pos2 = debug.find("\n##", pos1)
    if pos2 < 0:
        return debug[pos1:]
    return debug[pos1:pos2-1]


def compare_ast_symdb(cppcheck_parameters: str):
    same = True
    debug1 = run_cppcheck(cppcheck_parameters, '')
    debug2 = run_cppcheck(cppcheck_parameters, '--clang')

    ast1 = get_ast(debug1)
    ast2 = get_ast(debug2)
    if ast1 != ast2:
        print("ast is not the same: {}".format(cppcheck_parameters))
        with open('cppcheck.ast', 'wt') as f:
            f.write(ast1)
        with open('clang.ast', 'wt') as f:
            f.write(ast2)
        same = False

    symdb1 = get_symdb(debug1)
    symdb2 = get_symdb(debug2)
    if symdb1 != symdb2:
        print("symdb is not the same: {}".format(cppcheck_parameters))
        with open('cppcheck.symdb', 'wt') as f:
            f.write(symdb1)
        with open('clang.symdb', 'wt') as f:
            f.write(symdb2)
        same = False

    if not same:
        sys.exit(1)

for arg in sys.argv[1:]:
    print(arg)
    compare_ast_symdb(arg)

