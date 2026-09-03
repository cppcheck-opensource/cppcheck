
# python -m pytest vcxproj_unicode_test.py

from testutils import cppcheck

import os
import shutil

__script_dir = os.path.dirname(os.path.abspath(__file__))
__proj_dir = os.path.join(__script_dir, 'vcxproj-unicode')

def _get_dump_for_configuration(tmp_path, configuration):
    proj_dir = tmp_path / 'vcxproj-unicode'
    shutil.copytree(__proj_dir, proj_dir)

    args = [
        '--template=cppcheck1',
        '--project=vcxproj-unicode/vcxproj_unicode.vcxproj',
        f'--project-configuration={configuration}',
        '--no-cppcheck-build-dir',
        '--dump'
    ]
    ret, stdout, stderr = cppcheck(args, cwd=str(tmp_path))
    assert ret == 0, stdout
    assert stderr == '', stderr

    dump_path = proj_dir / 'main.cpp.dump'
    assert dump_path.exists(), f"Dump file not found at {dump_path}"

    with open(dump_path, 'rt') as f:
        return f.read()

def test_vcxproj_unicode_debug(tmp_path):
    dump_content = _get_dump_for_configuration(tmp_path, 'Debug|Win32')

    # the resolved defines are recorded in the <dump cfg="..."> attribute
    assert 'cfg="_WIN32=1;UNICODE=1;_UNICODE=1;_MSC_VER=1900"' in dump_content

def test_vcxproj_unicode_release(tmp_path):
    dump_content = _get_dump_for_configuration(tmp_path, 'Release|Win32')

    assert 'cfg="_WIN32=1;_MSC_VER=1900;__AFXWIN_H__=1"' in dump_content
