#!/usr/bin/env python3
#
# Tweaks a compile_commands.json file: for every build command that has a
# --sysroot argument, each existing -isystem argument gets a matching extra
# -isystem argument pointing into the sysroot, and the --sysroot argument
# itself is then removed. This is useful when a compiler resolves -isystem
# paths relative to --sysroot internally (as part of its built-in search
# path handling) but a tool consuming compile_commands.json (such as
# Cppcheck) does not, so the sysroot-relative path needs to be spelled out
# explicitly instead.
#
# Example:
#   --sysroot /a/b -isystem /opt/x
#   =>
#   -isystem /opt/x -isystem /a/b/opt/x
#
# Optionally, --isystem-to-i converts -isystem arguments to -I, which can be
# useful since Cppcheck otherwise treats -isystem headers as "system"
# headers and skips some checks in them. Paths whose folder name matches one
# of the --exclude-folder values are left as -isystem. For example, with
# --isystem-to-i --exclude-folder lib1:
#   -isystem /opt/x -isystem /path/lib1/include
#   =>
#   -I /opt/x -isystem /path/lib1/include
#
# Optionally, --remove-include-path removes -I arguments whose path contains
# the given path. For example, with --remove-include-path /path/lib1:
#   -I /opt/x -I /path/lib1/include
#   =>
#   -I /opt/x
#
# Usage:
#   tools/tweak-compile-commands.py compile_commands.json -o out.json
#   tools/tweak-compile-commands.py -i compile_commands.json
#   tools/tweak-compile-commands.py -i compile_commands.json --isystem-to-i --exclude-folder lib1
#   tools/tweak-compile-commands.py -i compile_commands.json --remove-include-path /path/lib1
#
# With neither -o nor -i, the result is written to stdout.

import argparse
import json
import shlex
import sys


def find_sysroot(tokens):
    for i, tok in enumerate(tokens):
        if tok == '--sysroot' and i + 1 < len(tokens):
            return tokens[i + 1]
        if tok.startswith('--sysroot='):
            return tok[len('--sysroot='):]
    return None


def join_sysroot(sysroot, path):
    return sysroot.rstrip('/') + '/' + path.lstrip('/')


def remove_sysroot_arg(tokens):
    result = []
    i = 0
    n = len(tokens)
    while i < n:
        tok = tokens[i]
        if tok == '--sysroot' and i + 1 < n:
            i += 2
            continue
        if tok.startswith('--sysroot='):
            i += 1
            continue
        result.append(tok)
        i += 1
    return result


def add_isystem_sysroot(tokens, sysroot):
    result = []
    i = 0
    n = len(tokens)
    while i < n:
        tok = tokens[i]
        if tok == '-isystem' and i + 1 < n:
            path = tokens[i + 1]
            result.append(tok)
            result.append(path)
            result.append('-isystem')
            result.append(join_sysroot(sysroot, path))
            i += 2
            continue
        if tok.startswith('-isystem') and tok != '-isystem':
            path = tok[len('-isystem'):]
            result.append(tok)
            result.append('-isystem' + join_sysroot(sysroot, path))
            i += 1
            continue
        result.append(tok)
        i += 1
    return result


def is_excluded(path, exclude_folders):
    parts = path.replace('\\', '/').split('/')
    return any(part in exclude_folders for part in parts if part)


def convert_isystem_to_i(tokens, exclude_folders):
    result = []
    i = 0
    n = len(tokens)
    while i < n:
        tok = tokens[i]
        if tok == '-isystem' and i + 1 < n:
            path = tokens[i + 1]
            result.append(tok if is_excluded(path, exclude_folders) else '-I')
            result.append(path)
            i += 2
            continue
        if tok.startswith('-isystem') and tok != '-isystem':
            path = tok[len('-isystem'):]
            result.append(tok if is_excluded(path, exclude_folders) else '-I' + path)
            i += 1
            continue
        result.append(tok)
        i += 1
    return result


def matches_remove_path(path, remove_paths):
    normalized = path.replace('\\', '/')
    return any(remove_path.replace('\\', '/') in normalized for remove_path in remove_paths)


def remove_include_paths(tokens, remove_paths):
    result = []
    i = 0
    n = len(tokens)
    while i < n:
        tok = tokens[i]
        if tok == '-I' and i + 1 < n:
            path = tokens[i + 1]
            if matches_remove_path(path, remove_paths):
                i += 2
                continue
            result.append(tok)
            result.append(path)
            i += 2
            continue
        if tok.startswith('-I') and tok != '-I':
            path = tok[len('-I'):]
            if matches_remove_path(path, remove_paths):
                i += 1
                continue
            result.append(tok)
            i += 1
            continue
        result.append(tok)
        i += 1
    return result


def tweak_entry(entry, isystem_to_i, exclude_folders, remove_include_path):
    if 'command' in entry:
        tokens = shlex.split(entry['command'])
    elif 'arguments' in entry:
        tokens = entry['arguments']
    else:
        return False

    changed = False

    sysroot = find_sysroot(tokens)
    if sysroot is not None:
        tokens = add_isystem_sysroot(tokens, sysroot)
        tokens = remove_sysroot_arg(tokens)
        changed = True

    if isystem_to_i:
        new_tokens = convert_isystem_to_i(tokens, exclude_folders)
        if new_tokens != tokens:
            tokens = new_tokens
            changed = True

    if remove_include_path:
        new_tokens = remove_include_paths(tokens, remove_include_path)
        if new_tokens != tokens:
            tokens = new_tokens
            changed = True

    if not changed:
        return False

    if 'command' in entry:
        entry['command'] = shlex.join(tokens)
    else:
        entry['arguments'] = tokens
    return True


def main():
    parser = argparse.ArgumentParser(
        description='Add sysroot-relative -isystem arguments to build commands in a compile_commands.json file.')
    parser.add_argument('compile_commands', help='path to the compile_commands.json file to read')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-o', '--output', help='write the result to this file instead of stdout')
    group.add_argument('-i', '--in-place', action='store_true', help='overwrite the input file with the result')
    parser.add_argument('--isystem-to-i', action='store_true',
                         help='convert -isystem arguments to -I (except excluded folders)')
    parser.add_argument('--exclude-folder', action='append', default=[], metavar='FOLDER',
                         help='folder name to keep as -isystem when using --isystem-to-i; can be given multiple times')
    parser.add_argument('--remove-include-path', action='append', default=[], metavar='PATH',
                         help='remove -I arguments whose path contains PATH; can be given multiple times')
    args = parser.parse_args()

    with open(args.compile_commands, encoding='utf-8') as f:
        entries = json.load(f)

    changed = 0
    for entry in entries:
        if tweak_entry(entry, args.isystem_to_i, args.exclude_folder, args.remove_include_path):
            changed += 1

    out = json.dumps(entries, indent=2) + '\n'

    if args.in_place:
        with open(args.compile_commands, 'w', encoding='utf-8') as f:
            f.write(out)
    elif args.output:
        with open(args.output, 'w', encoding='utf-8') as f:
            f.write(out)
    else:
        sys.stdout.write(out)

    print('tweaked {} of {} entries'.format(changed, len(entries)), file=sys.stderr)


if __name__ == '__main__':
    main()
