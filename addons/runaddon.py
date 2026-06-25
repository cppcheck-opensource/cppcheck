import cppcheckdata
import cppcheck
import runpy
import sys
import os

if __name__ == '__main__':
    addon = sys.argv[1]
    __addon_name__ = os.path.splitext(os.path.basename(addon))[0]
    sys.argv.pop(0)

    runpy.run_path(addon, run_name='__main__')

    # Run registered checkers
    cppcheck.runcheckers()
    sys.exit(cppcheckdata.EXIT_CODE)