import os
import sys
import pytest
import shutil
import json

from testutils import cppcheck_ex, cppcheck, __lookup_cppcheck_exe

def __remove_std_lookup_log(l : list, exepath):
    l.remove("looking for library 'std.cfg'")
    l.remove("looking for library '{}/std.cfg'".format(exepath))
    l.remove("looking for library '{}/cfg/std.cfg'".format(exepath))
    return l


def __create_gui_project(tmpdir):
    file_name = 'test.c'
    test_file = os.path.join(tmpdir, file_name)
    with open(test_file, 'wt'):
        pass

    project_file = os.path.join(tmpdir, 'project.cppcheck')
    with open(project_file, 'wt') as f:
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
    with open(test_file, 'wt'):
        pass

    compilation_db = [
        {
            "directory": str(tmpdir),
            "command": "c++ -o {}.o -c {}".format(os.path.basename(file_name), file_name),
            "file": file_name,
            "output": "{}.o".format(os.path.basename(file_name))
        }
    ]

    compile_commands = os.path.join(tmpdir, 'compile_commands.json')
    with open(compile_commands, 'wt') as f:
        f.write(json.dumps(compilation_db))

    return compile_commands, test_file


def test_lib_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        "looking for library '{}/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/gnu.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


def test_lib_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=gnu.cfg', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'gnu.cfg'",
        "looking for library '{}/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/gnu.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


def test_lib_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "looking for library '{}/none.cfg'".format(exepath),
        "looking for library '{}/cfg/none.cfg'".format(exepath),
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
    ]


def test_lib_lookup_notfound_project(tmpdir):  # #13938
    project_file, _ = __create_gui_project(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library=none', '--project={}'.format(project_file)])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        # TODO: needs to look relative to the project first
        # TODO: specify which folder is actually used for lookup here
        "looking for library 'none.cfg'",
        "looking for library '{}/none.cfg'".format(exepath),
        "looking for library '{}/cfg/none.cfg'".format(exepath),
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
    ]


def test_lib_lookup_notfound_compdb(tmpdir):
    compdb_file, _ = __create_compdb(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library=none', '--project={}'.format(compdb_file)])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        # TODO: specify which folder is actually used for lookup here
        "looking for library 'none.cfg'",
        "looking for library '{}/none.cfg'".format(exepath),
        "looking for library '{}/cfg/none.cfg'".format(exepath),
        "library not found: 'none'",
        "cppcheck: Failed to load library configuration file 'none'. File not found"
    ]


def test_lib_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=none.cfg', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'none.cfg'",
        "looking for library '{}/none.cfg'".format(exepath),
        "looking for library '{}/cfg/none.cfg'".format(exepath),
        "library not found: 'none.cfg'",
        "cppcheck: Failed to load library configuration file 'none.cfg'. File not found"
    ]


def test_lib_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=config/gnu.xml', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'config/gnu.xml'",
        "looking for library '{}/config/gnu.xml'".format(exepath),
        "looking for library '{}/cfg/config/gnu.xml'".format(exepath),
        "library not found: 'config/gnu.xml'",
        "cppcheck: Failed to load library configuration file 'config/gnu.xml'. File not found"
    ]


def test_lib_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=config/gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'config/gnu'",
        "looking for library '{}/config/gnu'".format(exepath),
        "looking for library '{}/cfg/config/gnu'".format(exepath),
        "library not found: 'config/gnu'",
        "cppcheck: Failed to load library configuration file 'config/gnu'. File not found"
    ]


# TODO: this can never be found - bail out early?
def test_lib_lookup_relative_noext_trailing_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=config/gnu/', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'config/gnu/'",
        "looking for library '{}/config/gnu/'".format(exepath),
        "looking for library '{}/cfg/config/gnu/'".format(exepath),
        "library not found: 'config/gnu/'",
        "cppcheck: Failed to load library configuration file 'config/gnu/'. File not found"
    ]


def test_lib_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    cfg_file = os.path.join(tmpdir, 'test.cfg')
    with open(cfg_file, 'wt') as f:
        f.write('''
<?xml version="1.0"?>
<def format="2">
</def>
        ''')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library={}'.format(cfg_file), test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library '{}'".format(cfg_file),
        'Checking {} ...'.format(test_file)
    ]


def test_lib_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    cfg_file = os.path.join(tmpdir, 'test.cfg')

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library={}'.format(cfg_file), test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library '{}'".format(cfg_file),
        "library not found: '{}'".format(cfg_file),
        "cppcheck: Failed to load library configuration file '{}'. File not found".format(cfg_file)
    ]


def test_lib_lookup_absolute_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    cfg_file = os.path.join(tmpdir, 'test')

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library={}'.format(cfg_file), test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library '{}'".format(cfg_file),
        "library not found: '{}'".format(cfg_file),
        "cppcheck: Failed to load library configuration file '{}'. File not found".format(cfg_file)
    ]


# TODO: this can never be found - bail out early?
def test_lib_lookup_absolute_noext_trailing_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    cfg_file = os.path.join(tmpdir, 'test')
    cfg_file_trail = cfg_file + os.path.sep

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=library', '--library={}'.format(cfg_file_trail), test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library '{}'".format(cfg_file_trail),
        "library not found: '{}'".format(cfg_file_trail),
        "cppcheck: Failed to load library configuration file '{}'. File not found".format(cfg_file_trail)
    ]


def test_lib_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "looking for library '{}/gtk.cfg'".format(exepath),
        "looking for library '{}/cfg/gtk.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


# make sure we bail out when we encounter an invalid file
def test_lib_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    gnu_cfg_file = os.path.join(tmpdir, 'gnu.cfg')
    with open(gnu_cfg_file, 'wt') as f:
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
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=library', '--library=posix,gnu', test_file])
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        exepath = exepath.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = __remove_std_lookup_log(stdout.splitlines(), exepath)
    assert lines == [
        "looking for library 'posix.cfg'",
        "looking for library '{}/posix.cfg'".format(exepath),
        "looking for library '{}/cfg/posix.cfg'".format(exepath),
        "looking for library 'gnu.cfg'",
        "looking for library '{}/gnu.cfg'".format(exepath),
        "looking for library '{}/cfg/gnu.cfg'".format(exepath),
        'Checking {} ...'.format(test_file)
    ]


def test_platform_lookup_builtin(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=unix64', test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    # built-in platform are not being looked up
    assert lines == [
        'Checking {} ...'.format(test_file)
    ]


@pytest.mark.skip  # TODO: fails when not run from the root folder
def test_platform_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=avr8', test_file])
    cwd = os.getcwd()
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8'",
        "try to load platform file '{}/avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/avr8.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/avr8.xml' ... Success".format(cwd),
        'Checking {} ...'.format(test_file)
    ]


@pytest.mark.skip  # TODO: fails when not run from the root folder
def test_platform_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=avr8.xml', test_file])
    cwd = os.getcwd()
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8.xml'",
        "try to load platform file '{}/avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/avr8.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/avr8.xml' ... Success".format(cwd),
        'Checking {} ...'.format(test_file)
    ]


def test_platform_lookup_path(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        return "try to load platform file '{}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}".format(f, f)
    def try_success(f):
        f = format_path(f)
        return "try to load platform file '{}' ... Success".format(f)
    lines = stdout.replace('\\', '/').replace('"', '\'').splitlines()
    assert lines == [
        "looking for platform 'avr8.xml'",
        try_fail(os.path.join(tmpdir, 'avr8.xml')),
        try_fail(os.path.join(tmpdir, 'platforms', 'avr8.xml')),
        try_fail(os.path.join(path, 'avr8.xml')),
        try_success(os.path.join(path, 'platforms', 'avr8.xml')),
        'Checking {} ...'.format(format_path(test_file))
    ]


def test_platform_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(cwd, cwd),
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'none'."
    ]


# TODO: test with invalid file in project path
# TODO: test with non-file in project path
def test_platform_lookup_notfound_project(tmpdir):  # #13939
    project_file, _ = __create_gui_project(tmpdir)
    project_path = os.path.dirname(project_file)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none', '--project={}'.format(project_file)])
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
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(project_path, project_path),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(project_path, project_path),
        # TODO: the following lookups are in CWD - is this intended?
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(cwd, cwd),
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'none'."
    ]


def test_platform_lookup_notfound_compdb(tmpdir):
    compdb_file, _ = __create_compdb(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=none', '--project={}'.format(compdb_file)])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'none'",
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(cwd, cwd),
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'none'."
    ]


def test_platform_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(cwd, cwd),
        "try to load platform file '{}/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'none.xml'."
    ]


def test_platform_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "try to load platform file '{}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none.xml".format(cwd, cwd),
        "try to load platform file '{}/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none.xml".format(exepath, exepath),
        "try to load platform file '{}/platforms/platform/none.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none.xml".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'platform/none.xml'."
    ]


def test_platform_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "try to load platform file '{}/platform/none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none".format(cwd, cwd),
        "try to load platform file '{}/platforms/platform/none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none".format(cwd, cwd),
        "try to load platform file '{}/platform/none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none".format(exepath, exepath),
        "try to load platform file '{}/platforms/platform/none' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'platform/none'."
    ]


# TODO: this can never be found - bail out early?
def test_platform_lookup_relative_noext_trailing_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=platform', '--platform=platform/none/', test_file])
    cwd = os.getcwd()
    exepath = os.path.dirname(exe)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
        exepath = exepath.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'platform/none/'",
        "try to load platform file '{}/platform/none/' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none/".format(cwd, cwd),
        "try to load platform file '{}/platforms/platform/none/' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none/".format(cwd, cwd),
        "try to load platform file '{}/platform/none/' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platform/none/".format(exepath, exepath),
        "try to load platform file '{}/platforms/platform/none/' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/platforms/platform/none/".format(exepath, exepath),
        "cppcheck: error: unrecognized platform: 'platform/none/'."
    ]


def test_platform_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    platform_file = os.path.join(tmpdir, 'test.xml')
    with open(platform_file, 'wt') as f:
        f.write('''
<platform format="2"/>
        ''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform={}'.format(platform_file), test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform '{}'".format(platform_file),
        "try to load platform file '{}' ... Success".format(platform_file),
        'Checking {} ...'.format(test_file)
    ]


def test_platform_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    platform_file = os.path.join(tmpdir, 'test.xml')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform={}'.format(platform_file), test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform '{}'".format(platform_file),
        "try to load platform file '{}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}".format(platform_file, platform_file),
        "cppcheck: error: unrecognized platform: '{}'.".format(platform_file)
    ]


def test_platform_lookup_absolute_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    platform_file = os.path.join(tmpdir, 'test')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform={}'.format(platform_file), test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform '{}'".format(platform_file),
        "try to load platform file '{}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}".format(platform_file, platform_file),
        "cppcheck: error: unrecognized platform: '{}'.".format(platform_file)
    ]


# TODO: this can never be found - bail out early?
def test_platform_lookup_absolute_noext_trailing_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    platform_file = os.path.join(tmpdir, 'test')
    platform_file_trail = platform_file + os.path.sep

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform={}'.format(platform_file_trail), test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform '{}'".format(platform_file_trail),
        "try to load platform file '{}' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}".format(platform_file_trail, platform_file_trail),
        "cppcheck: error: unrecognized platform: '{}'.".format(platform_file_trail)
    ]


@pytest.mark.skip  # TODO: fails when not run from the root folder
def test_platform_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "try to load platform file '{}/avr8.xml' ... Error=XML_ERROR_FILE_NOT_FOUND ErrorID=3 (0x3) Line number=0: filename={}/avr8.xml".format(cwd, cwd),
        "try to load platform file '{}/platforms/avr8.xml' ... Success".format(cwd),
        'Checking {}1 ...'.format(test_file)
    ]


def test_platform_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    avr8_file = os.path.join(tmpdir, 'avr8.xml')
    with open(avr8_file, 'wt') as f:
        f.write('''{}''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=platform', '--platform=avr8', test_file], cwd=tmpdir)
    cwd = str(tmpdir)
    if sys.platform == 'win32':
        cwd = cwd.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for platform 'avr8'",
        "try to load platform file '{}/avr8.xml' ... Error=XML_ERROR_PARSING_TEXT ErrorID=8 (0x8) Line number=1".format(cwd),
        "cppcheck: error: unrecognized platform: 'avr8'."
    ]


def test_addon_lookup(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        "looking for addon '{}misra.py'".format(exepath_sep),
        "looking for addon '{}addons/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Checking {} ...'.format(test_file)
    ]


def test_addon_lookup_ext(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=misra.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        "looking for addon '{}misra.py'".format(exepath_sep),
        "looking for addon '{}addons/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Checking {} ...'.format(test_file)
    ]


def test_addon_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        "looking for addon '{}none.py'".format(exepath_sep),
        "looking for addon '{}addons/none.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_notfound_project(tmpdir):  # #13940 / #13941
    project_file, _ = __create_gui_project(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none', '--project={}'.format(project_file)])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        # TODO: needs to look relative to the project file first
        "looking for addon 'none.py'",
        "looking for addon '{}none.py'".format(exepath_sep),
        "looking for addon '{}addons/none.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_notfound_compdb(tmpdir):
    compdb_file, _ = __create_compdb(tmpdir)

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none', '--project={}'.format(compdb_file)])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        "looking for addon '{}none.py'".format(exepath_sep),
        "looking for addon '{}addons/none.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_ext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=none.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'none.py'",
        "looking for addon '{}none.py'".format(exepath_sep),
        "looking for addon '{}addons/none.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon none.py'
    ]


def test_addon_lookup_relative_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=addon/misra.py', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'addon/misra.py'",
        "looking for addon '{}addon/misra.py'".format(exepath_sep),
        "looking for addon '{}addons/addon/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon addon/misra.py'
    ]


# FIXME: an addon requires a file extension as we need to differentiate between .py and .json addons
def test_addon_lookup_relative_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=addon/misra', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'addon/misra.py'",
        "looking for addon '{}addon/misra.py'".format(exepath_sep),
        "looking for addon '{}addons/addon/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon addon/misra.py'
    ]


# TODO: this can never be found - bail out early?
def test_addon_lookup_relative_noext_trailing_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, _, exe = cppcheck_ex(['--debug-lookup=addon', '--addon=addon/misra/', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    assert exitcode == 1, stdout
    lines = stdout.splitlines()
    assert lines == [
        # TODO: should not append extension
        "looking for addon 'addon/misra/.py'",
        "looking for addon '{}addon/misra/.py'".format(exepath_sep),
        "looking for addon '{}addons/addon/misra/.py'".format(exepath_sep),  # TODO: mixed separators
        'Did not find addon addon/misra/.py'
    ]


def test_addon_lookup_absolute(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    addon_file = os.path.join(tmpdir, 'test.py')
    with open(addon_file, 'wt') as f:
        f.write('''''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon={}'.format(addon_file), test_file])
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon '{}'".format(addon_file),
        'Checking {} ...'.format(test_file)
    ]


def test_addon_lookup_absolute_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    addon_file = os.path.join(tmpdir, 'test.py')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon={}'.format(addon_file), test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon '{}'".format(addon_file),
        'Did not find addon {}'.format(addon_file)
    ]


# FIXME: an addon requires a file extension as we need to differentiate between .py and .json addons
def test_addon_lookup_absolute_noext_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    addon_file = os.path.join(tmpdir, 'test')
    addon_file_py = os.path.join(tmpdir, 'test.py')  # TODO: do not add extension

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon={}'.format(addon_file), test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon '{}'".format(addon_file_py),
        'Did not find addon {}'.format(addon_file_py)
    ]


# TODO: this can never be found - bail out early?
def test_addon_lookup_absolute_noext_trailing_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    addon_file = os.path.join(tmpdir, 'test')
    addon_file_trail = addon_file + os.path.sep
    addon_file_trail_py = addon_file_trail + '.py'  # TODO: do not add extension

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon={}'.format(addon_file_trail), test_file])
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon '{}'".format(addon_file_trail_py),
        'Did not find addon {}'.format(addon_file_trail_py)
    ]


def test_addon_lookup_nofile(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
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
        "looking for addon '{}misra.py'".format(exepath_sep),
        "looking for addon '{}addons/misra.py'".format(exepath_sep),  # TODO: mixed separators
        'Checking {} ...'.format(test_file)
    ]


# make sure we bail out when we encounter an invalid file
def test_addon_lookup_invalid(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    misra_py_file = os.path.join(tmpdir, 'misra.py')
    with open(misra_py_file, 'wt') as f:
        f.write('''<def/>''')

    exitcode, stdout, stderr = cppcheck(['--debug-lookup=addon', '--addon=misra', test_file], cwd=tmpdir)
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for addon 'misra.py'",
        'Checking {} ...'.format(test_file)  # TODO: should bail out
    ]


def test_config_lookup(tmpdir):
    cppcheck_exe = __lookup_cppcheck_exe()
    bin_dir = os.path.dirname(cppcheck_exe)
    tmp_cppcheck_exe = shutil.copy2(cppcheck_exe, tmpdir)
    if sys.platform == 'win32':
        shutil.copy2(os.path.join(bin_dir, 'cppcheck-core.dll'), tmpdir)
    shutil.copytree(os.path.join(bin_dir, 'cfg'), os.path.join(tmpdir, 'cfg'))

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    config_file = os.path.join(tmpdir, 'cppcheck.cfg')
    with open(config_file, 'wt') as f:
        f.write('{}')

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', test_file], cwd=tmpdir, cppcheck_exe=tmp_cppcheck_exe)
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    exepath_sep = exepath_sep.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for '{}cppcheck.cfg'".format(exepath_sep),
        'Checking {} ...'.format(test_file)
    ]


def test_config_lookup_notfound(tmpdir):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', test_file])
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    exepath_sep = exepath_sep.replace('\\', '/')
    assert exitcode == 0, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for '{}cppcheck.cfg'".format(exepath_sep),
        'no configuration found',
        'Checking {} ...'.format(test_file)
    ]


def test_config_invalid(tmpdir):
    cppcheck_exe = __lookup_cppcheck_exe()
    bin_dir = os.path.dirname(cppcheck_exe)
    tmp_cppcheck_exe = shutil.copy2(cppcheck_exe, tmpdir)
    if sys.platform == 'win32':
        shutil.copy2(os.path.join(bin_dir, 'cppcheck-core.dll'), tmpdir)
    shutil.copytree(os.path.join(bin_dir, 'cfg'), os.path.join(tmpdir, 'cfg'))

    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    config_file = os.path.join(tmpdir, 'cppcheck.cfg')
    with open(config_file, 'wt'):
        pass

    exitcode, stdout, stderr, exe = cppcheck_ex(['--debug-lookup=config', test_file], cwd=tmpdir, cppcheck_exe=tmp_cppcheck_exe)
    exepath = os.path.dirname(exe)
    exepath_sep = exepath + os.path.sep
    exepath_sep = exepath_sep.replace('\\', '/')
    assert exitcode == 1, stdout if stdout else stderr
    lines = stdout.splitlines()
    assert lines == [
        "looking for '{}cppcheck.cfg'".format(exepath_sep),
        'cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 1 near: '
    ]

# TODO: test with FILESDIR

@pytest.mark.parametrize("type,file", [("addon", "misra.py"), ("config", "cppcheck.cfg"), ("library", "gnu.cfg"), ("platform", "avr8.xml")])
def test_lookup_path(tmpdir, type, file):
    test_file = os.path.join(tmpdir, 'test.c')
    with open(test_file, 'wt'):
        pass

    cppcheck = 'cppcheck' # No path
    path = os.path.dirname(__lookup_cppcheck_exe())
    env = os.environ.copy()
    env['PATH'] = path + (';' if sys.platform == 'win32' else ':') + env.get('PATH', '')
    if type == 'config':
        with open(os.path.join(path, "cppcheck.cfg"), 'wt') as f:
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