# tweak-compile-commands.py

## NAME

tweak-compile-commands.py - tweak `-isystem`/`--sysroot`/`-I` options in a `compile_commands.json` file

## SYNOPSIS

```
tools/tweak-compile-commands.py COMPILE_COMMANDS [-o OUTPUT | -i]
                                         [--isystem-to-i] [--exclude-folder FOLDER ...]
                                         [--remove-include-path PATH ...]
```

## DESCRIPTION

In many cases the system headers should not be included in Cppcheck
analysis, it is preferable to use `--library` instead. The headers
do not provide the knowledge needed for static analysis, i.e. they
can say what types the arguments to a function has but the header
do not provide the semantics of the functions.

However sometimes you do want to include system headers in Cppcheck
analysis. And you need to have handling of `--sysroot` and
`-isystem`. This script will tweak the compile_commands.json file.

### SYSROOT

Example build command such as:

```
gcc --sysroot /a/b -isystem /opt/x -c foo.c
```

gcc searches both `/opt/x` *and* `/a/b/opt/x` for headers.

`tweak-compile-commands.py` rewrites each build command in a
`compile_commands.json` file so this implicit behaviour is spelled out
explicitly: for every command that has a `--sysroot` argument, every
existing `-isystem PATH` argument gets a matching, explicit
`-isystem SYSROOT/PATH` argument added right after it, and the `--sysroot`
argument is then removed (it is no longer needed since the sysroot-relative
paths are now spelled out explicitly). Commands without a `--sysroot`
argument are left unchanged.

### ISYSTEM

The script has an option `--isystem-to-i`, this tells the script to
convert `-isystem` arguments to `-I`.

The option `--exclude-folder` can be used to skip certain folders. Use
that for a folder if Cppcheck option `--library` can be used instead.

### REMOVE -I

The script also has `--remove-include-path`, the script will remove
any `-I PATH` argument whose path contains a given string. This is
useful for stripping include paths that Cppcheck should not see at all.

## ARGUMENTS

`COMPILE_COMMANDS`
: Path to the `compile_commands.json` file to read.

## OPTIONS

`-o OUTPUT`, `--output OUTPUT`
: Write the result to `OUTPUT` instead of stdout. Cannot be combined with
  `-i`.

`-i`, `--in-place`
: Overwrite `COMPILE_COMMANDS` with the result. Cannot be combined with
  `-o`.

`--isystem-to-i`
: Also convert `-isystem PATH` arguments to `-I PATH`, except for paths
  excluded with `--exclude-folder`. Has no effect on its own if not given
  (the sysroot tweak still applies).

`--exclude-folder FOLDER`
: When used with `--isystem-to-i`, keep any `-isystem` argument as
  `-isystem` (instead of converting it to `-I`) if `FOLDER` is one of the
  path's folder components (an exact match of a path segment, not a
  substring). May be given multiple times. Ignored if `--isystem-to-i` is
  not given.

`--remove-include-path PATH`
: Remove any `-I` argument whose path contains `PATH` as a substring. May be
  given multiple times; a path is removed if it matches any of them.
  Independent of `--isystem-to-i`/`--exclude-folder`, and applies after them,
  so a path converted from `-isystem` to `-I` can also be removed by this
  option.

With neither `-o` nor `-i`, the resulting JSON is written to stdout, and
the input file is left untouched. A summary (`tweaked N of M entries`) is
always printed to stderr.

## EXAMPLES

Preview the sysroot tweak without touching any file:

```
$ tools/tweak-compile-commands.py compile_commands.json
```

Apply the sysroot tweak in place:

```
$ tools/tweak-compile-commands.py -i compile_commands.json
```

Apply the sysroot tweak and convert `-isystem` to `-I`, keeping any path
that goes through a `lib1` or `lib2` folder as `-isystem`:

```
$ tools/tweak-compile-commands.py -i compile_commands.json \
      --isystem-to-i --exclude-folder lib1 --exclude-folder lib2
```

Given this input entry:

```json
{
  "command": "gcc --sysroot /a/b -isystem /opt/x -isystem /path/lib1/include -c foo.c -o foo.o"
}
```

the last command above produces:

```json
{
  "command": "gcc -I /opt/x -I /a/b/opt/x -isystem /path/lib1/include -isystem /a/b/path/lib1/include -c foo.c -o foo.o"
}
```

Note that `/path/lib1/include` is kept as `-isystem` (matching
`--exclude-folder lib1`), and so is its sysroot-relative duplicate
`/a/b/path/lib1/include`, since it also contains a `lib1` folder component.

Remove all `-I` include paths that go through `/path/lib1`:

```
$ tools/tweak-compile-commands.py -i compile_commands.json \
      --remove-include-path /path/lib1
```

Given this input entry:

```json
{
  "command": "gcc -I /opt/x -I /path/lib1/include -c foo.c -o foo.o"
}
```

the command above produces:

```json
{
  "command": "gcc -I /opt/x -c foo.c -o foo.o"
}
```

## EXIT STATUS

Exits with a non-zero status and a traceback if `COMPILE_COMMANDS` cannot
be read or does not contain valid JSON. Otherwise exits 0, even if no
entries needed changes.
