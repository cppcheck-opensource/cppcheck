
import subprocess
import sys

TRACDB = 'trac.db'


def readdb():
    cmds = ['sqlite3', TRACDB, 'SELECT id,keywords FROM ticket WHERE status<>"closed";']
    with subprocess.Popen(cmds, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p:
        # TODO: handle p.returncode?
        stdout, _ = p.communicate()
    data = stdout
    ret = {}
    for line in data.splitlines():
        pos1 = line.find('|')
        if pos1 <= 0:
            continue
        nr = line[:pos1]
        for kw in line[pos1 + 1:].split(' '):
            if kw == '':
                continue
            if kw not in ret:
                ret[kw] = []
            ret[kw].append(nr)
    return ret


for arg in sys.argv[1:]:
    if arg.endswith('/trac.db'):
        TRACDB = arg

data = readdb()
for kw in sorted(data.keys()):
    out = kw + ':'
    for ticket in data[kw]:
        out = out + ' ' + ticket
    print(out)
