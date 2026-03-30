import os
import sys
import pytest
import shutil
import json

from testutils import cppcheck_ex, cppcheck, __lookup_cppcheck_exe

def __remove_std_lookup_log(l : list, exepath):
    l.remove("looking for library 'std.cfg'")
    l.remove(f"looking for library '{exepath}/std.cfg'")
    l.remove(f"looking for library '{exepath}/cfg/std.cfg'")
    return l


def __create_gui_project(tmpdir):
    file_name = 'test.c'
    test_file = os.path.join(tmpdir, file_name)
    with open(test_file, 'w'):
        pass

    project_file = os.path.join(tmpdir, 'project.cppcheck')
    with open(project_file, 'w') as f:
        f.write(
"""<?xml version="1.0" encoding="UTF-8"?>
<project version="1">
    <paths>
        <dir name="{}"/>
    </paths>
</project>""".format(test_file)
        )

    return project_file, test_file


def __create_compdb(tmpdir):
    file_name = 'test.c'
    test_file = os.path.join(tmpdir, file_name)
    with open(test_file, 'w'):
        pass

    compilation_db = [
        {
            "directory": str(tmpdir),
            "command": f"c++ -o {os.path.basename(file_name)}.o -c {file_name}",
            "file": file_name,
            "output": f"{os.path.basename(file_name)}.o"
        }
    ]

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    with open(compile_commands, 'w') as f:
        f.write(json.dumps(compilation_db))

    return compile_commands, test_file


def test_lib_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        f"looking for library '{exepath}/gnu.cfg'",
        f"looking for library '{exepath}/cfg/gnu.cfg'",
        f'Checking {test_file} ...'
    ]


def test_lib_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu.cfg', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        f"looking for library '{exepath}/gnu.cfg'",
        f"looking for library '{exepath}/cfg/gnu.cfg'",
        f'Checking {test_file} ...'
    ]


def test_lib_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library=none', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        # TODO: specify which folder is actually used for lookup here
        "looking for library 'none.cfg'",
        f"looking for library '{exepath}/none.cfg'",
        f"looking for library '{exepath}/cfg/none.cfg'",
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
    ]


def test_lib_lookup_notfound_project(tmpdir):  # #13938
    project_file, _ = __create_gui_project(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library=none', f'--project={project_file}'])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        # TODO: needs to look relative to the project first
        # TODO: specify which folder is actually used for lookup here
        "looking for library 'none.cfg'",
        f"looking for library '{exepath}/none.cfg'",
        f"looking for library '{exepath}/cfg/none.cfg'",
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
    ]


def test_lib_lookup_notfound_compdb(tmpdir):
    compdb_file, _ = __create_compdb(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library=none', f'--project={compdb_file}'])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        # TODO: specify which folder is actually used for lookup here
        "looking for library 'none.cfg'",
        f"looking for library '{exepath}/none.cfg'",
        f"looking for library '{exepath}/cfg/none.cfg'",
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
    ]


def test_lib_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=none.cfg', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'none.cfg'",
        f"looking for library '{exepath}/none.cfg'",
        f"looking for library '{exepath}/cfg/none.cfg'",
        "library not found: 'none.cfg'",
        "cppcheck: Failed to load library configuration file 'none.cfg'. File not found"
    ]


def test_lib_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=config/gnu.xml', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'config/gnu.xml'",
        f"looking for library '{exepath}/config/gnu.xml'",
        f"looking for library '{exepath}/cfg/config/gnu.xml'",
        "library not found: 'config/gnu.xml'",
        "cppcheck: Failed to load library configuration file 'config/gnu.xml'. File not found"
    ]


def test_lib_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=config/gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'config/gnu.cfg'",
        f"looking for library '{exepath}/config/gnu.cfg'",
        f"looking for library '{exepath}/cfg/config/gnu.cfg'",
        "library not found: 'config/gnu'",
        "cppcheck: Failed to load library configuration file 'config/gnu'. File not found"
    ]


def test_lib_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    cfg_file = os.path.join(tmpdir, 'test.cfg')
    with open(cfg_file, 'w') as f:
        f.write('''
<?xml version="1.0"?>
<def format="2">
</def>
        ''')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', f'--library={cfg_file}', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        f"looking for library '{cfg_file}'",
        f'Checking {test_file} ...'
    ]


def test_lib_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    cfg_file = os.path.join(tmpdir, 'test.cfg')

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', f'--library={cfg_file}', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        f"looking for library '{cfg_file}'",
        f"library not found: '{cfg_file}'",
        f"cppcheck: Failed to load library configuration file '{cfg_file}'. File not found"
    ]


def test_lib_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    # make sure we do not produce an error when the attempted lookup path is a directory and not a file
    gtk_cfg_dir = os.path.join(tmpdir, 'gtk.cfg')
    os.mkdir(gtk_cfg_dir)

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gtk', test_file], cwd=tmpdir)
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gtk.cfg'",
        f"looking for library '{exepath}/gtk.cfg'",
        f"looking for library '{exepath}/cfg/gtk.cfg'",
        f'Checking {test_file} ...'
    ]


# make sure we bail out when we encounter an invalid file
def test_lib_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    gnu_cfg_file = os.path.join(tmpdir, 'gnu.cfg')
    with open(gnu_cfg_file, 'w') as f:
        f.write('''{}''')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu', test_file], cwd=tmpdir)
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        "library not found: 'gnu'",
        "Error=XML_ERROR_PARSING_TEXT ErrorID=8 (0x8) Line number=1",  # TODO: log the failure before saying the library was not found
        "cppcheck: Failed to load library configuration file 'gnu'. Bad XML"
    ]


def test_lib_lookup_multi(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=posix,gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'posix.cfg'",
        f"looking for library '{exepath}/posix.cfg'",
        f"looking for library '{exepath}/cfg/posix.cfg'",
        "looking for library 'gnu.cfg'",
        f"looking for library '{exepath}/gnu.cfg'",
        f"looking for library '{exepath}/cfg/gnu.cfg'",
        f'Checking {test_file} ...'
    ]


def test_platform_lookup_builtin(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=unix64', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    # built-in platform are not being looked up
    assert lines == [
        f'Checking {test_file} ...'
    ]


@pytest.mark.skip  # TODO: fails when not run from the root folder
def test_platform_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=avr8', test_file])
    cwd = os.getcwd()
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8'",
        f"try to load platform file '{cwd}/avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/avr8.xml",
        f"try to load platform file '{cwd}/platforms/avr8.xml' ... Success",
        f'Checking {test_file} ...'
    ]


@pytest.mark.skip  # TODO: fails when not run from the root folder
def test_platform_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=avr8.xml', test_file])
    cwd = os.getcwd()
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8.xml'",
        f"try to load platform file '{cwd}/avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/avr8.xml",
        f"try to load platform file '{cwd}/platforms/avr8.xml' ... Success",
        f'Checking {test_file} ...'
    ]


def test_platform_lookup_path(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    cppcheck = 'cppcheck' # No path
    path = os.path.dirname(__lookup_cppcheck_exe())
    env = os.environ.copy()
    env['PATH'] = path
    exitcode, stdout, stderr, _ = cppcheck_ex(args=['--debug-lookup=platform', '--platform=avr8.xml', test_file], cppcheck_exe=cppcheck, cwd=str(tmpdir), env=env)
    assert exitcode == 0, stdout if stdout else stderr
    def format_path(p):
        return p.replace('\\', '/').replace('"', '\'')
    def try_fail(f):
        f = format_path(f)
        return f"try to load platform file '{f}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={f}"
    def try_success(f):
        f = format_path(f)
        return f"try to load platform file '{f}' ... Success"
    lines = stdout.replace('\\', '/').replace('"', '\'').splitlines()
    assert lines == [
        "looking for platform 'avr8.xml'",
        try_fail(os.path.join(tmpdir, 'avr8.xml')),
        try_fail(os.path.join(tmpdir, 'platforms', 'avr8.xml')),
        try_fail(os.path.join(path, 'avr8.xml')),
        try_success(os.path.join(path, 'platforms', 'avr8.xml')),
        f'Checking {format_path(test_file)} ...'
    ]


def test_platform_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none', test_file])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'none'",
        f"try to load platform file '{cwd}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/none.xml",
        f"try to load platform file '{cwd}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platforms/none.xml",
        f"try to load platform file '{exepath}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/none.xml",
        f"try to load platform file '{exepath}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platforms/none.xml",
        "cppcheck: error: unrecognized platform: 'none'."
    ]


# TODO: test with invalid file in project path
# TODO: test with non-file in project path
def test_platform_lookup_notfound_project(tmpdir):  # #13939
    project_file, _ = __create_gui_project(tmpdir)
    project_path = os.path.dirname(project_file)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none', f'--project={project_file}'])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
        project_path = project_path.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'none'",
        f"try to load platform file '{project_path}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={project_path}/none.xml",
        f"try to load platform file '{project_path}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={project_path}/platforms/none.xml",
        # TODO: the following lookups are in CWD - is this intended?
        f"try to load platform file '{cwd}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/none.xml",
        f"try to load platform file '{cwd}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platforms/none.xml",
        f"try to load platform file '{exepath}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/none.xml",
        f"try to load platform file '{exepath}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platforms/none.xml",
        "cppcheck: error: unrecognized platform: 'none'."
    ]


def test_platform_lookup_notfound_compdb(tmpdir):
    compdb_file, _ = __create_compdb(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none', f'--project={compdb_file}'])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'none'",
        f"try to load platform file '{cwd}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/none.xml",
        f"try to load platform file '{cwd}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platforms/none.xml",
        f"try to load platform file '{exepath}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/none.xml",
        f"try to load platform file '{exepath}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platforms/none.xml",
        "cppcheck: error: unrecognized platform: 'none'."
    ]


def test_platform_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none.xml', test_file])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'none.xml'",
        f"try to load platform file '{cwd}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/none.xml",
        f"try to load platform file '{cwd}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platforms/none.xml",
        f"try to load platform file '{exepath}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/none.xml",
        f"try to load platform file '{exepath}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platforms/none.xml",
        "cppcheck: error: unrecognized platform: 'none.xml'."
    ]


def test_platform_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=platform/none.xml', test_file])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'platform/none.xml'",
        f"try to load platform file '{cwd}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platform/none.xml",
        f"try to load platform file '{cwd}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platforms/platform/none.xml",
        f"try to load platform file '{exepath}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platform/none.xml",
        f"try to load platform file '{exepath}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platforms/platform/none.xml",
        "cppcheck: error: unrecognized platform: 'platform/none.xml'."
    ]


def test_platform_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=platform/none', test_file])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'platform/none'",
        f"try to load platform file '{cwd}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platform/none.xml",
        f"try to load platform file '{cwd}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/platforms/platform/none.xml",
        f"try to load platform file '{exepath}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platform/none.xml",
        f"try to load platform file '{exepath}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={exepath}/platforms/platform/none.xml",
        "cppcheck: error: unrecognized platform: 'platform/none'."
    ]


def test_platform_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    platform_file = os.path.join(tmpdir, 'test.xml')
    with open(platform_file, 'w') as f:
        f.write('''
<platform format="2"/>
        ''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', f'--platform={platform_file}', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        f"looking for platform '{platform_file}'",
        f"try to load platform file '{platform_file}' ... Success",
        f'Checking {test_file} ...'
    ]


def test_platform_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    platform_file = os.path.join(tmpdir, 'test.xml')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', f'--platform={platform_file}', test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        f"looking for platform '{platform_file}'",
        f"try to load platform file '{platform_file}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={platform_file}",
        f"cppcheck: error: unrecognized platform: '{platform_file}'."
    ]


@pytest.mark.skip  # TODO: fails when not run from the root folder
def test_platform_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    # make sure we do not produce an error when the attempted lookup path is a directory and not a file
    avr8_cfg_dir = os.path.join(tmpdir, 'avr8.xml')
    os.mkdir(avr8_cfg_dir)

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=avr8', test_file])
    cwd = os.getcwd()
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8'",
        f"try to load platform file '{cwd}/avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={cwd}/avr8.xml",
        f"try to load platform file '{cwd}/platforms/avr8.xml' ... Success",
        f'Checking {test_file}1 ...'
    ]


def test_platform_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    avr8_file = os.path.join(tmpdir, 'avr8.xml')
    with open(avr8_file, 'w') as f:
        f.write('''{}''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=avr8', test_file], cwd=tmpdir)
    cwd = str(tmpdir)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8'",
        f"try to load platform file '{cwd}/avr8.xml' ... Error=XML_ERROR_PARSING_TEXT ErrorID=8 (0x8) Line number=1",
        "cppcheck: error: unrecognized platform: 'avr8'."
    ]


def test_addon_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        f"looking for addon '{exepath_sep}misra.py'",
        f"looking for addon '{exepath_sep}addons/misra.py'",  # TODO: mixed separators
        f'Checking {test_file} ...'
    ]


def test_addon_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        f"looking for addon '{exepath_sep}misra.py'",
        f"looking for addon '{exepath_sep}addons/misra.py'",  # TODO: mixed separators
        f'Checking {test_file} ...'
    ]


def test_addon_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        f"looking for addon '{exepath_sep}none.py'",
        f"looking for addon '{exepath_sep}addons/none.py'",  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_notfound_project(tmpdir):  # #13940 / #13941
    project_file, _ = __create_gui_project(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none', f'--project={project_file}'])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        # TODO: needs to look relative to the project file first
        "looking for addon 'none.py'",
        f"looking for addon '{exepath_sep}none.py'",
        f"looking for addon '{exepath_sep}addons/none.py'",  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_notfound_compdb(tmpdir):
    compdb_file, _ = __create_compdb(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none', f'--project={compdb_file}'])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        f"looking for addon '{exepath_sep}none.py'",
        f"looking for addon '{exepath_sep}addons/none.py'",  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        f"looking for addon '{exepath_sep}none.py'",
        f"looking for addon '{exepath_sep}addons/none.py'",  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=addon/misra.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'addon/misra.py'",
        f"looking for addon '{exepath_sep}addon/misra.py'",
        f"looking for addon '{exepath_sep}addons/addon/misra.py'",  # TODO: mixed separators
        'Did not find addon addon/misra.py'
    ]


def test_addon_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=addon/misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'addon/misra.py'",
        f"looking for addon '{exepath_sep}addon/misra.py'",
        f"looking for addon '{exepath_sep}addons/addon/misra.py'",  # TODO: mixed separators
        'Did not find addon addon/misra.py'
    ]


def test_addon_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    addon_file = os.path.join(tmpdir, 'test.py')
    with open(addon_file, 'w') as f:
        f.write('''''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', f'--addon={addon_file}', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        f"looking for addon '{addon_file}'",
        f'Checking {test_file} ...'
    ]


def test_addon_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    addon_file = os.path.join(tmpdir, 'test.py')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', f'--addon={addon_file}', test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        f"looking for addon '{addon_file}'",
        f'Did not find addon {addon_file}'
    ]


def test_addon_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    # make sure we do not produce an error when the attempted lookup path is a directory and not a file
    misra_dir = os.path.join(tmpdir, 'misra')
    os.mkdir(misra_dir)
    misra_cfg_dir = os.path.join(tmpdir, 'misra.py')
    os.mkdir(misra_cfg_dir)

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        f"looking for addon '{exepath_sep}misra.py'",
        f"looking for addon '{exepath_sep}addons/misra.py'",  # TODO: mixed separators
        f'Checking {test_file} ...'
    ]


# make sure we bail out when we encounter an invalid file
def test_addon_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    misra_py_file = os.path.join(tmpdir, 'misra.py')
    with open(misra_py_file, 'w') as f:
        f.write('''<def/>''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon=misra', test_file], cwd=tmpdir)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        f'Checking {test_file} ...'  # TODO: should bail out
    ]


def test_config_lookup(tmpdir):
    cppcheck_exe = __lookup_cppcheck_exe()
    bin_dir = os.path.dirname(cppcheck_exe)
    tmp_cppcheck_exe = shutil.copy2(cppcheck_exe, tmpdir)
    if sys.platform == 'win32':
        shutil.copy2(os.path.join(bin_dir, 'cppcheck-core.dll'), tmpdir)
    shutil.copytree(os.path.join(bin_dir, 'cfg'), os.path.join(tmpdir, 'cfg'))

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    config_file = os.path.join(tmpdir, 'cppcheck.cfg')
    with open(config_file, 'w') as f:
        f.write('{}')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', test_file], cwd=tmpdir, cppcheck_exe=tmp_cppcheck_exe)
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    exepath_sep = exepath_sep.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        f"looking for '{exepath_sep}cppcheck.cfg'",
        f'Checking {test_file} ...'
    ]


def test_config_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    exepath_sep = exepath_sep.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        f"looking for '{exepath_sep}cppcheck.cfg'",
        'no configuration found',
        f'Checking {test_file} ...'
    ]


def test_config_invalid(tmpdir):
    cppcheck_exe = __lookup_cppcheck_exe()
    bin_dir = os.path.dirname(cppcheck_exe)
    tmp_cppcheck_exe = shutil.copy2(cppcheck_exe, tmpdir)
    if sys.platform == 'win32':
        shutil.copy2(os.path.join(bin_dir, 'cppcheck-core.dll'), tmpdir)
    shutil.copytree(os.path.join(bin_dir, 'cfg'), os.path.join(tmpdir, 'cfg'))

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    config_file = os.path.join(tmpdir, 'cppcheck.cfg')
    with open(config_file, 'w'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', test_file], cwd=tmpdir, cppcheck_exe=tmp_cppcheck_exe)
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    exepath_sep = exepath_sep.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        f"looking for '{exepath_sep}cppcheck.cfg'",
        'cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 1 near: '
    ]

# TODO: test with FILESDIR

@pytest.mark.parametrize("type,file", [("addon", "misra.py"), ("config", "cppcheck.cfg"), ("library", "gnu.cfg"), ("platform", "avr8.xml")])
def test_lookup_path(tmpdir, type, file):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'w'):
        pass

    cppcheck = 'cppcheck' # No path
    path = os.path.dirname(__lookup_cppcheck_exe())
    env = os.environ.copy()
    env['PATH'] = path + (';' if sys.platform == 'win32' else ':') + env.get('PATH', '')
    if type == 'config':
        with open(os.path.join(path, "cppcheck.cfg"), 'w') as f:
            f.write('{}')
        exitcode, stdout, stderr, _ = cppcheck_ex(args=[f'--debug-lookup={type}', test_file], cppcheck_exe=cppcheck, cwd=str(tmpdir), env=env)
        os.remove(os.path.join(path, "cppcheck.cfg")) # clean up otherwise other tests may fail
    else:
        exitcode, stdout, stderr, _ = cppcheck_ex(args=[f'--debug-lookup={type}', f'--{type}={file}', test_file], cppcheck_exe=cppcheck, cwd=str(tmpdir), env=env)
    assert exitcode == 0, stdout if stdout else stderr
    def format_path(p):
        return p.replace('\\', '/').replace('"', '\'')
    lines = format_path(stdout).splitlines()

    if type == 'addon':
        def try_fail(f):
            return f"looking for {type} '{format_path(f)}'"
        def try_success(f):
            return f"looking for {type} '{format_path(f)}'"
        assert lines == [
            f"looking for {type} '{file}'",
            try_fail(os.path.join(path, file)),
            try_success(os.path.join(path, 'addons', file)),
            f'Checking {format_path(test_file)} ...'
        ]
    elif type == 'config':
        def try_success(f):
            return f"looking for '{format_path(f)}'"
        assert lines == [
            try_success(os.path.join(path, file)),
            f'Checking {format_path(test_file)} ...'
        ]
    elif type == 'platform':
        def try_fail(f):
            f = format_path(f)
            return f"try to load {type} file '{f}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={f}"
        def try_success(f):
            f = format_path(f)
            return f"try to load {type} file '{f}' ... Success"
        assert lines == [
            f"looking for {type} '{file}'",
            try_fail(os.path.join(tmpdir, file)),
            try_fail(os.path.join(tmpdir, 'platforms', file)),
            try_fail(os.path.join(path, file)),
            try_success(os.path.join(path, 'platforms', file)),
            f'Checking {format_path(test_file)} ...'
        ]
    elif type == 'library':
        def try_fail(f):
            return f"looking for {type} '{format_path(f)}'"
        def try_success(f):
            return f"looking for {type} '{format_path(f)}'"
        assert lines == [
            f"looking for {type} 'std.cfg'",
            try_fail(os.path.join(path, 'std.cfg')),
            try_success(os.path.join(path, 'cfg', 'std.cfg')),
            f"looking for {type} '{file}'",
            try_fail(os.path.join(path, file)),
            try_success(os.path.join(path, 'cfg', file)),
            f'Checking {format_path(test_file)} ...'
        ]
    else:
        assert False, type + " not tested properly"