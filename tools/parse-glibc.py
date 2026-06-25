#!/usr/bin/env python3

import glob
import os


def checknonnull(cfg, functionName, nonnull):
    pos1 = cfg.find('<function name="' + functionName + '">')
    if pos1 < 0:
        return
    pos2 = cfg.find('</function>', pos1)
    if pos2 < 0:
        return
    functionCfg = cfg[pos1:pos2]
    s = None
    for argnr in range(10):
        argpos1 = functionCfg.find('<arg nr="' + str(argnr) + '">')
        if argpos1 < 0:
            continue
        argpos2 = functionCfg.find('</arg>', argpos1)
        notnullpos = functionCfg.find('not-null', argpos1)
        if 0 <= notnullpos < argpos2:
            if s:
                s += ', ' + str(argnr)
            else:
                s = str(argnr)
    if s != nonnull:
        if not nonnull:
            nonnull = ''
        if not s:
            s = ''
        print(functionName + '\tglibc:' + nonnull + '\tcfg:' + s)


def parseheader(cppcheckpath, filename):
    with open(filename, 'rt') as f:
        data = f.read()

    with open(cppcheckpath + '/cfg/std.cfg', 'rt') as f:
        stdcfg = f.read()

    with open(cppcheckpath + '/cfg/posix.cfg', 'rt') as f:
        posixcfg = f.read()

    while '/*' in data:
        pos1 = data.find('/*')
        pos2 = data.find('*/', pos1 + 2)
        data = data[:pos1] + data[pos2 + 2:]

    data = data.replace('\\\n', '')

    while '\n#' in data:
        pos1 = data.find('\n#')
        pos2 = data.find('\n', pos1 + 1)
        data = data[:pos1] + data[pos2:]

    while '\n__BEGIN' in data:
        pos1 = data.find('\n__BEGIN')
        pos2 = data.find('\n', pos1 + 1)
        data = data[:pos1] + data[pos2:]

    while '\n__END' in data:
        pos1 = data.find('\n__END')
        pos2 = data.find('\n', pos1 + 1)
        data = data[:pos1] + data[pos2:]

    data = data.replace('\n\n', '\n')
    data = data.replace('\t', '    ')
    data = data.replace(',\n  ', ',')
    data = data.replace(')\n  ', ',')
    data = data.replace('  ', ' ')

    output = []

    for line in data.split('\n'):
        if (line[:7] != 'extern ' and ' extern ' not in line) or line[-1] != ';':
            continue

        functionNameEnd = line.find('(') - 1
        if functionNameEnd < 0:
            continue
        while line[functionNameEnd] == ' ':
            functionNameEnd -= 1
        if functionNameEnd < 10:
            continue
        functionNameStart = functionNameEnd
        while line[functionNameStart] == '_' or line[functionNameStart].isalnum():
            functionNameStart -= 1
        if functionNameStart < 10:
            continue
        if line[functionNameStart] != '*' and line[functionNameStart] != ' ':
            continue
        functionNameStart += 1
        if not line[functionNameStart].isalpha():
            continue

        functionName = line[functionNameStart:functionNameEnd + 1]

        nonnull = None

        nonnullStart = line.find('__nonnull')
        if nonnullStart >= 0:
            nonnullStart += 9
            while nonnullStart < len(line) and line[nonnullStart] == ' ':
                nonnullStart += 1
            if nonnullStart >= len(line) or line[nonnullStart] != '(':
                continue
            while line[nonnullStart] == '(':
                nonnullStart += 1
            nonnullEnd = line.find(')', nonnullStart)
            nonnull = line[nonnullStart:nonnullEnd]

        checknonnull(stdcfg, functionName, nonnull)
        checknonnull(posixcfg, functionName, nonnull)

        if nonnull:
            s = functionName + ' ' + nonnull
            if s not in output:
                output.append(s)


for f in glob.glob('/usr/include/*.h'):
    parseheader(os.path.expanduser('~/cppcheck'), f)
