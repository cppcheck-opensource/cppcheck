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

def _add_tar_member(tf, name, linkname=None):
    info = tarfile.TarInfo(name)
    data = b''
    if linkname is not None:
        info.type = tarfile.SYMTYPE
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
