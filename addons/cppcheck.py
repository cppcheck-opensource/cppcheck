
import cppcheckdata
import sys
import os

__checkers__ = []

def checker(f):
    __checkers__.append(f)
    return f


__errorid__ = ''
__addon_name__ = ''
def reportError(location, severity, message, errorId=None):
    cppcheckdata.reportError(location, severity, message, __addon_name__, errorId or __errorid__)

def runcheckers():
    # If there are no checkers then don't run
    if len(__checkers__) == 0:
        return
    global __addon_name__
    global __errorid__
    addon = sys.argv[0]
    parser = cppcheckdata.ArgumentParser()
    args = parser.parse_args()

    __addon_name__ = os.path.splitext(os.path.basename(addon))[0]

    for dumpfile in args.dumpfile:
        if not args.quiet:
            print('Checking %s...' % dumpfile)

        data = cppcheckdata.CppcheckData(dumpfile)

        for cfg in data.iterconfigurations():
            if not args.quiet:
                print('Checking %s, config %s...' % (dumpfile, cfg.name))
            for c in __checkers__:
                __errorid__ = c.__name__
                c(cfg, data)
