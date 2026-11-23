#!/usr/bin/env python
#
# Cppcheck - A tool for static C/C++ code analysis
# Copyright (C) 2007-2026 Cppcheck team.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import io
import tarfile

from donate_cpu_lib import *

def _test_library_includes(tmpdir, libs, content, libinc_obj=None):
    if libinc_obj is None:
        library_includes = LibraryIncludes()
    else:
        library_includes = libinc_obj

    src_file = os.path.join(str(tmpdir), "file.cpp")
    with open(src_file, 'w') as f:
        f.write(content)
    libs.sort()
    libs_found = library_includes.get_libraries(str(tmpdir))
    libs_found.sort()
    assert libs == libs_found

def test_library_includes(tmpdir):
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd'], '')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd'], '#include <stdio.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'boost'], '#include <boost/regex.hpp>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'python'], '#include "Python.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'libcerror', 'lua', 'opengl', 'qt'], '#include <QApplication>\n#include <GL/gl.h>\r#include "lua.h"\r\n#include <libcerror.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'microsoft_sal'], '  #include <sal.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'googletest'], '\t#include <gtest/gtest.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'microsoft_atl'], '  \t  #include <atlbase.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'cairo'], '\t  #include <cairo.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'gtk'], '  \t#include <glib-object.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'gtk'], '  \t #include <glib.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'gtk'], ' # include <glib/gi18n.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'gtk'], '#include <gdk/gdkkeysyms.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'bsd'], '#include <sys/uio.h>\r\n')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'libcurl'], '#include <curl/curl.h>\r')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'sqlite3'], '#include <sqlite3.h>\n')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'openmp'], '#  include <omp.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'mfc'], '#\tinclude <afxwin.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'ruby'], '#  \tinclude "ruby.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'zlib'], '#\t  include <zlib.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'pcre'], '#include<pcre.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'pcre'], '#include  "pcre.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'opengl'], '#include\t  <GL/glut.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'nspr'], '#include\t"prtypes.h"')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'lua'], '#include  \t<lua.h>')
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'microsoft_gsl'], '#include <gsl/gsl>')

def test_match_multiple_time(tmpdir):
    libinc = LibraryIncludes()

    # there was a bug that we would only match each library once successfully
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'zlib'], '#include <zlib.h>', libinc)
    _test_library_includes(tmpdir, ['posix', 'gnu', 'bsd', 'zlib'], '#include <zlib.h>', libinc)

def _add_tar_member(tf, name, linkname=None, linktype=tarfile.SYMTYPE, dirmode=None):
    info = tarfile.TarInfo(name)
    data = b''
    if dirmode is not None:
        info.type = tarfile.DIRTYPE
        info.mode = dirmode
    elif linkname is not None:
        info.type = linktype
        info.linkname = linkname
    else:
        data = b'int x;\n'
        info.size = len(data)
    tf.addfile(info, io.BytesIO(data))

def test_unpack_package_outside_temp(tmpdir):
    work_path = str(tmpdir)
    outside_path = os.path.join(work_path, 'outside')
    os.mkdir(outside_path)
    tgz = os.path.join(work_path, 'temp.tgz')
    with tarfile.open(tgz, 'w:gz') as tf:
        _add_tar_member(tf, 'pkg/main.c')
        _add_tar_member(tf, 'pkg/../../outside/dotdot.c')
        _add_tar_member(tf, 'pkg/inc.h', linkname=outside_path)
        _add_tar_member(tf, 'pkg/inc.h/symlink.c')

    temp_path, source_found = unpack_package(work_path, tgz)
    assert source_found
    assert os.path.isfile(os.path.join(temp_path, 'pkg', 'main.c'))
    assert os.listdir(outside_path) == []

def test_unpack_package_duplicate_name(tmpdir):
    work_path = str(tmpdir)
    outside_path = os.path.join(work_path, 'outside')
    os.mkdir(outside_path)
    outside_file = os.path.join(outside_path, 'target.c')
    with open(outside_file, 'w', encoding='utf-8') as f:
        f.write('int y;\n')
    tgz = os.path.join(work_path, 'temp.tgz')
    with tarfile.open(tgz, 'w:gz') as tf:
        # a harmless member followed by a link with the same name that points outside temp
        _add_tar_member(tf, 'pkg/sym.c')
        _add_tar_member(tf, 'pkg/sym.c', linkname=outside_file)
        _add_tar_member(tf, 'pkg/hard.c')
        _add_tar_member(tf, 'pkg/hard.c', linkname='../outside/target.c', linktype=tarfile.LNKTYPE)
        # a link inside temp followed by a same-named link that only looks inside when resolved through the first one
        _add_tar_member(tf, 'pkg/inc.h', linkname='a/b/c')
        _add_tar_member(tf, 'pkg/inc.h', linkname='../../outside')

    temp_path, source_found = unpack_package(work_path, tgz)
    assert source_found
    for name in ('sym.c', 'hard.c'):
        path = os.path.join(temp_path, 'pkg', name)
        assert os.path.isfile(path)
        assert not os.path.islink(path)
        assert not os.path.samefile(path, outside_file)
    real_temp_path = os.path.realpath(temp_path)
    inc_path = os.path.realpath(os.path.join(temp_path, 'pkg', 'inc.h'))
    assert os.path.commonpath([real_temp_path, inc_path]) == real_temp_path
    assert os.listdir(outside_path) == ['target.c']

def test_unpack_package_link_redirected_later(tmpdir):
    work_path = str(tmpdir)
    outside_path = os.path.join(work_path, 'outside')
    os.mkdir(outside_path)
    with open(os.path.join(outside_path, 'secret.c'), 'w', encoding='utf-8') as f:
        f.write('int secret;\n')
    tgz = os.path.join(work_path, 'temp.tgz')
    with tarfile.open(tgz, 'w:gz') as tf:
        _add_tar_member(tf, 'pkg/src/main.c')
        # 'd.h' does not exist yet, so this target is resolved as temp/outside and looks harmless
        _add_tar_member(tf, 'pkg/l.h', linkname='d.h/../../outside')
        # turns the missing component into a link, which redirects 'l.h' out of temp
        _add_tar_member(tf, 'pkg/d.h', linkname='.')
        # a link that stays inside the package is still extracted
        _add_tar_member(tf, 'pkg/include/main.c', linkname='../src/main.c')

    temp_path, source_found = unpack_package(work_path, tgz)
    assert source_found
    real_temp_path = os.path.realpath(temp_path)
    for name in ('l.h', 'd.h'):
        path = os.path.join(temp_path, 'pkg', name)
        if os.path.lexists(path):
            assert os.path.commonpath([real_temp_path, os.path.realpath(path)]) == real_temp_path
    inc_path = os.path.join(temp_path, 'pkg', 'include', 'main.c')
    if os.path.islink(inc_path):
        assert os.path.samefile(inc_path, os.path.join(temp_path, 'pkg', 'src', 'main.c'))
    assert os.listdir(outside_path) == ['secret.c']

def test_unpack_package_hardlink_missing_target(tmpdir):
    work_path = str(tmpdir)
    tgz = os.path.join(work_path, 'temp.tgz')
    with tarfile.open(tgz, 'w:gz') as tf:
        _add_tar_member(tf, 'pkg/main.c')
        # the target is neither on disk nor in the archive, so tarfile raises KeyError instead of OSError
        _add_tar_member(tf, 'pkg/copy.c', linkname='pkg/missing.c', linktype=tarfile.LNKTYPE)

    temp_path, source_found = unpack_package(work_path, tgz)
    assert source_found
    assert os.path.isfile(os.path.join(temp_path, 'pkg', 'main.c'))
    assert not os.path.lexists(os.path.join(temp_path, 'pkg', 'copy.c'))

def test_unpack_package_link_in_readonly_dir(tmpdir):
    work_path = str(tmpdir)
    outside_path = os.path.join(work_path, 'outside')
    os.mkdir(outside_path)
    with open(os.path.join(outside_path, 'secret.c'), 'w', encoding='utf-8') as f:
        f.write('int secret;\n')
    tgz = os.path.join(work_path, 'temp.tgz')
    with tarfile.open(tgz, 'w:gz') as tf:
        _add_tar_member(tf, 'pkg/main.c')
        # redirected out of temp by the next member, like in test_unpack_package_link_redirected_later
        _add_tar_member(tf, 'pkg/x.h/l.h', linkname='d.h/../../../outside')
        _add_tar_member(tf, 'pkg/x.h/d.h', linkname='.')
        # a directory member with a header-like name makes the folder read-only after the links exist
        _add_tar_member(tf, 'pkg/x.h', dirmode=0o555)

    temp_path, source_found = unpack_package(work_path, tgz)
    assert source_found
    assert os.path.isfile(os.path.join(temp_path, 'pkg', 'main.c'))
    real_temp_path = os.path.realpath(temp_path)
    for name in ('l.h', 'd.h'):
        path = os.path.join(temp_path, 'pkg', 'x.h', name)
        if os.path.lexists(path):
            assert os.path.commonpath([real_temp_path, os.path.realpath(path)]) == real_temp_path
    assert os.listdir(outside_path) == ['secret.c']
