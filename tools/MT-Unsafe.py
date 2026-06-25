#!/usr/bin/python -u

"""
Generates a list of MT unsafe symbols for a cppcheck addon.

The cppcheck addon threadsafety.py uses a list
   id_MTunsafe_full
of symbols - mostly functions - which are not multi-thread safe.

This script generates such a list by parsing (the troff source of)
a man page, or a directory tree of man pages,
looking for the attributes described in 'man 7 attributes'.

Typical example use:
  MT-Unsafe.py /usr/share/man/man3
The output must then be merged into the threadsafety.py addon.
"""

import gzip
import os
import re
import sys

debug = 0
verbose = 0

unsafe_apis = set()
unsafe_types = set()


def dprint(level, fmt, varlist=()):
    """Print messages for someone debugging this script. This wraps print()."""
    if debug < level:
        return
    if varlist:
        print(fmt % varlist, file=sys.stderr)
    else:
        print(fmt, file=sys.stderr)


def vprint(level, fmt, varlist=()):
    """Print messages for someone running this script. This wraps print()."""
    if verbose < level:
        return
    if varlist:
        print(fmt % varlist, file=sys.stderr)
    else:
        print(fmt, file=sys.stderr)


def man_search(manpage):
    """Search one manpage for tokens  in the attributes table."""
    vprint(1, '-- %s --' % (manpage))

    try:
        if manpage.endswith('.gz'):
            MANPAGE = gzip.open(manpage, 'r')
        else:
            MANPAGE = open(manpage, 'r')
    except OSError as filename:
        print('cannot open %s' % filename, file=sys.stderr)
        return  # None, None

    vprint(1, '%s opened' % (manpage))

    TSmatch = None
    for lineread in MANPAGE:
        vprint(4, 'type %s', type(lineread))
        lineread = str(lineread)
        vprint(3, '--%s' % lineread)
        # TSmatch = lineread.startswith('.TS')
        TSmatch = re.search('\\.TS', lineread)
        if TSmatch:
            dprint(1, '%s:\treached .TS' % (manpage))
            break

    # dprint(2, '%s', lineread)

    if not TSmatch:
        dprint(1, '.TS not found in %s' % manpage)
        return  # None, None

    vprint(1, 'Started reading the attribute table')

    apis = set()
    for lineread in MANPAGE:
        lineread = str(lineread)
        dprint(2, '%s' % (lineread))
        if 'MT-Safe' in lineread:
            vprint(1, 'clearing MT-Safe %s', lineread)
            apis.clear()

        res = re.search(r'\.BR\s+(\w+)\s', lineread)
        # vprint(1, '%s for %s' % (res, lineread))
        if res:
            apis.add(res.group(1))
            dprint(1, 'found api %s in %s' % (res.group(1), lineread))
            continue

        if 'MT-Unsafe' in lineread:
            resUnsafe = re.search("MT-Unsafe\\s+(.*)(\\n\'|$)", lineread)

            if resUnsafe:
                values = resUnsafe.group(1)
                dprint(1, 'a %s' % values)
                values = re.sub(r'\\n\'$', '', values)
                #
                values = values.split(' ')
                dprint(1, 'values %s' % list(values))
                for val in values:
                    unsafe_types.add(val)

            # dprint(1, 'pushing ', list(apis), sep=',')
            dprint(1, 'new apis %s' % list(apis))
            for api in apis:
                unsafe_apis.add(api)
                continue

        #  if lineread.startswith('.TE'):
        if re.search('.TE', lineread):
            dprint(1, '%s:\treached .TE' % (manpage))
            break

    dprint(1, 'Finished reading the attribute table')

    MANPAGE.close()

    return  # list(unsafe_types), list(unsafe_apis)


def do_man_page(manpage):
    """Wrap man_search(), with logging."""
    dprint(1, 'do_man_page(%s)' % (manpage))
    man_search(manpage)
    if unsafe_types:
        dprint(1, '%d new types in %s' % (len(unsafe_types), manpage))
    else:
        dprint(1, 'No new types in %s' % (manpage))

    if unsafe_apis:
        dprint(1, '%d unsafe_apis in %s' % (len(unsafe_apis), manpage))
    else:
        dprint(1, 'No new apis in %s' % (manpage))


def do_man_dir(directory):
    """Recursively process a directory of man-pages."""
    dprint(1, 'do_man_dir(%s)' % (directory))
    if os.path.isfile(directory):
        do_man_page(directory)
        return

    for path, _, files in os.walk(directory):
        for file in files:
            dprint(2, 'calling do_man_page(%s)' % (
                os.path.join(path, file)))
            do_man_page(os.path.join(path, file))


manpages = set()
for arg in sys.argv[1:]:
    if arg.startswith('-'):
        if re.match('^-+debug', arg):
            debug = debug+1
            dprint(1, 'debug %d' % debug)
            continue
    else:
        if os.access(arg, os.R_OK):
            manpages.add(arg)
            dprint(1, 'manpages+= %s' % (arg))
        else:
            dprint(0, 'skipping arg - not readable')

dprint(2, 'manpages: %s' % manpages)


for manpage in manpages:
    do_man_dir(manpage)


dprint(1, '-----------------------------------------\n')
dprint(1, '%d unsafe_types' % len(unsafe_types))
dprint(1, '%d unsafe_apis' % len(unsafe_apis))
dprint(1, 'type: %s' % type(unsafe_apis))

print('{\n    # Types marked MT-Unsafe')
# unsafe_types is not the whole of the list,
# so the last item *is* followed by a comma:
for u_type in sorted(unsafe_types):
    print("    '%s'," % u_type)


print('    # APIs marked MT-Unsafe')
# unsafe_apis completes the list,
# so we ought to remove the last comma.
for u_api in sorted(unsafe_apis):
    print("    '%s'," % u_api)

print('}\n')

# print(sorted(unsafe_apis), sep=',\n  ', end='\n}\n')
