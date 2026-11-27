from importlib import import_module
import time
import warnings

import pytest

# the tests need the requirements of the server (donate-cpu-server-requirements.txt) and httpx
pytest.importorskip('fastapi')
pytest.importorskip('jinja2')
pytest.importorskip('uvicorn')
pytest.importorskip('httpx')

with warnings.catch_warnings():
    # newer Starlette versions warn that 'httpx2' should be used instead of 'httpx'
    warnings.filterwarnings('ignore', message='.*httpx2')
    from fastapi.testclient import TestClient  # pylint: disable=wrong-import-position

server = import_module('donate-cpu-server-rest')

PKG_A = 'ftp://ftp.de.debian.org/debian/pool/main/a/aaa/aaa_1.0.orig.tar.gz'
PKG_B = 'ftp://ftp.de.debian.org/debian/pool/main/b/bbb/bbb_1.0.orig.tar.gz'
PKG_C = 'ftp://ftp.de.debian.org/debian/pool/main/c/ccc/ccc_1.0.orig.tar.xz'

RESULT_CRASH = '''{url}
cppcheck-options: --library=posix
platform: Linux-7
python: 3.14.4
client-version: 1.3.67
compiler: g++ 15
cppcheck: head {old}
head-info: abc
count: Crash! 5
elapsed-time: -1.0 3.0
head results:
Program received signal SIGSEGV, Segmentation fault.
0x000055 in Foo::bar (this=0x0) at lib/foo.cpp:12
12	  return x->y;
#0  0x000055 in Foo::bar (this=0x0) at lib/foo.cpp:12
#1  0x000066 in main (argc=1) at cli/main.cpp:3

aaa/x.c:1:1: error: <script>alert(1)</script> [syntaxError]
diff:
'''.format(url=PKG_A, old=server.OLD_VERSION)

INFO = '''{url}
cppcheck: head {old}
info messages:
aaa/x.c:1:1: information: --check-library: There is no matching configuration for function foo() [checkLibraryFunction]
aaa/x.c:2:1: information: --check-library: There is no matching configuration for function foo() [checkLibraryFunction]
aaa/x.c:3:1: information: --check-library: Function bar() should have <noreturn> configuration [checkLibraryNoReturn]
aaa/x.c:4:1: information: --check-library: There is no matching configuration for unsigned int [checkLibraryCheckType]
'''.format(url=PKG_A, old=server.OLD_VERSION)

# result file as written by donate-cpu-server.py
RESULT_FILE_C = '''2026-11-01 10:00
{url}
cppcheck: head {old}
count: 3 2
elapsed-time: 10.0 2.0
head results:
ccc/a.c:1:2: error: Array 'a[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]
ccc/b.c:4:1: error: There is an unknown macro here somewhere. Configuration is required. If FOO is a macro then please configure it. [unknownMacro]

diff:
head ccc/a.c:1:2: error: Array 'a[2]' accessed at index 2, which is out of bounds. [arrayIndexOutOfBounds]
{old} ccc/c.c:1:2: style: blah [knownConditionTrueFalse]
'''.format(url=PKG_C, old=server.OLD_VERSION)


@pytest.fixture(name='work_path')
def fixture_work_path(tmp_path):
    (tmp_path / 'donated-results').mkdir()
    (tmp_path / 'packages.txt').write_text('\n'.join([PKG_A, PKG_B, PKG_C]) + '\n')
    (tmp_path / 'donated-results' / 'ccc').write_text(RESULT_FILE_C)
    return tmp_path


@pytest.fixture(name='client')
def fixture_client(work_path):
    app = server.create_app(str(work_path), rescan_interval=0.2)
    with TestClient(app) as client:
        for _ in range(100):
            if client.get('/api/v1/status').json()['index_ready']:
                break
            time.sleep(0.05)
        yield client


def test_parse_result():
    entry = server.parse_result(RESULT_FILE_C)
    assert entry['date'] == '2026-11-01 10:00'
    assert entry['url'] == PKG_C
    assert entry['versions'] == ['head', server.OLD_VERSION]
    assert entry['count'] == ['3', '2']
    assert entry['elapsed'] == [10.0, 2.0]
    assert entry['head_ids'] == {'arrayIndexOutOfBounds': 1, 'unknownMacro': 1}
    assert entry['diff'] == {'arrayIndexOutOfBounds': [0, 1], 'knownConditionTrueFalse': [1, 0]}
    assert entry['names'] == {'unknownMacro': {'FOO': 1}}
    assert entry['crash'] is None


def test_parse_crash():
    entry = server.parse_result('2026-11-27 10:00\n' + RESULT_CRASH)
    assert entry['crash'] == {
        'crash_line': '0x000055 in Foo::bar (this=0x0) at lib/foo.cpp:12',
        'code_line': '12\t  return x->y;',
        'stack_trace': ['#0 Foo::bar(...) at lib/foo.cpp:12', '#1 main(...) at cli/main.cpp:3'],
    }
    assert entry['head_ids'] == {'syntaxError': 1}


def test_parse_info():
    entry = server.parse_info('2026-11-27 10:00\n' + INFO)
    assert entry['info_ids'] == {'checkLibraryFunction': 2, 'checkLibraryNoReturn': 1, 'checkLibraryCheckType': 1}
    assert entry['names'] == {
        'checkLibraryFunction': {'foo()': 2},
        'checkLibraryNoReturn': {'bar()': 1},
        'checkLibraryCheckType': {'unsigned int': 1},
    }


def test_existing_results_are_indexed(client):
    assert client.get('/api/v1/status').json()['results'] == 1
    data = client.get('/api/v1/reports/time').json()
    assert [p['package'] for p in data['packages']] == ['ccc']
    assert data['packages'][0]['factor'] == 5.0


def test_packages(client):
    assert client.get('/api/v1/versions').json() == {'versions': ['head', server.OLD_VERSION]}
    assert client.get('/api/v1/packages').json() == {'count': 3}
    assert client.post('/api/v1/packages/next').json() == {'index': 0, 'url': PKG_A}
    assert client.get('/api/v1/packages/2').json() == {'index': 2, 'url': PKG_C}
    assert client.get('/api/v1/packages/9').status_code == 404
    assert client.post('/api/v1/nodata', json={'url': PKG_B}).status_code == 200
    # PKG_B is not handed out any more
    assert [client.post('/api/v1/packages/next').json()['index'] for _ in range(3)] == [2, 0, 2]


def test_upload(client):
    r = client.post('/api/v1/results', content=RESULT_CRASH + '\nDONE')
    assert r.status_code == 201 and r.json()['package'] == 'aaa'
    assert client.post('/api/v1/info', content=INFO).status_code == 201

    crash = client.get('/api/v1/reports/crash').json()
    assert [p['package'] for p in crash['packages']] == ['aaa']
    assert crash['stack_traces'][0]['packages'] == ['aaa']
    assert client.get('/api/v1/reports/crash?packages_only=true').text == PKG_A + '\n'

    names = client.get('/api/v1/reports/names/checkLibraryFunction').json()
    assert names['names'] == [{'name': 'foo()', 'count': 2}]


def test_upload_errors(client, work_path):
    wrong_version = RESULT_CRASH.replace('head ' + server.OLD_VERSION, 'head 1.0')
    assert client.post('/api/v1/results', content=wrong_version).status_code == 422
    assert client.post('/api/v1/results', content='ftp://evil/x/../../y.tar.gz\nfoo').status_code == 400
    unknown = RESULT_CRASH.replace('/aaa/aaa_1.0', '/zzz/zzz_1.0')
    assert client.post('/api/v1/results', content=unknown).status_code == 404

    # too large data removes the existing result
    big = PKG_C + '\ncppcheck: head ' + server.OLD_VERSION + '\n' + 'x' * server.MAX_RESULT_SIZE
    assert client.post('/api/v1/results', content=big).status_code == 413
    assert not (work_path / 'donated-results' / 'ccc').exists()


def test_files_written_by_old_server_are_picked_up(client, work_path):
    (work_path / 'donated-results' / 'aaa').write_text('2026-11-27 10:00\n' + RESULT_CRASH)
    for _ in range(100):
        if client.get('/api/v1/status').json()['results'] == 2:
            break
        time.sleep(0.05)
    assert client.get('/api/v1/status').json()['results'] == 2


def test_html_pages(client):
    client.post('/api/v1/results', content=RESULT_CRASH)
    client.post('/api/v1/info', content=INFO)
    pages = ['/', '/crash.html', '/timeout.html', '/stale.html', '/diff.html', '/diff-knownConditionTrueFalse',
             '/head.html', '/headinfo.html', '/head-syntaxError', '/headtoday-syntaxError', '/headinfo-checkLibraryFunction',
             '/latest.html', '/time_lt.html', '/time_gt.html', '/time_slow.html', '/clients.html',
             '/check_library_function_report.html', '/check_library_check_type_report.html', '/unknown_macro.html',
             '/value_flow_bailout_incomplete_var.html']
    for page in pages:
        r = client.get(page)
        assert r.status_code == 200, page
        assert r.headers['content-type'].startswith('text/html'), page
        assert '<script>' not in r.text, page

    assert '&lt;script&gt;' in client.get('/head-syntaxError').text
    assert client.get('/head-syntaxError?pkgs=1').text == PKG_A + '\n'
    assert 'unknownMacro' in client.get('/head.html').text


def test_html_name_pages(client):
    client.post('/api/v1/info', content=INFO)
    assert 'Function bar() should have' in client.get('/check_library-bar%28%29').text
    # '+' is a space, as in the links of donate-cpu-server.py
    assert 'configuration for unsigned int' in client.get('/check_library-unsigned+int').text
    assert 'please configure it' in client.get('/unknown_macro-FOO').text


def test_raw_files(client):
    client.post('/api/v1/info', content=INFO)
    r = client.get('/ccc')
    assert r.status_code == 200 and r.text == RESULT_FILE_C
    assert client.get('/info_output/aaa').status_code == 200
    assert client.get('/nonexistent').status_code == 404
    assert client.get('/..%2F..%2Fetc%2Fpasswd').status_code == 404
