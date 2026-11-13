from importlib import import_module

donate_cpu_server = import_module('donate-cpu-server')

def _test_parse_req(req_str, url_exp, queryParams_exp):
    url, queryParams = donate_cpu_server.HttpClientThread.parse_req(req_str)
    assert url == url_exp and queryParams == queryParams_exp

def test_parse_req():
    _test_parse_req("", None, None)
    _test_parse_req("GET / HTTP/1.1", '/', {})
    _test_parse_req("GET /crash.html HTTP/1.1", '/crash.html', {})
    _test_parse_req("GET /head-uninitvar HTTP/1.1", '/head-uninitvar', {})
    _test_parse_req("GET /check_library-std%3A%3Aunordered_set%3A%3Ainsert%28%29 HTTP/1.1", '/check_library-std%3A%3Aunordered_set%3A%3Ainsert%28%29', {})
    _test_parse_req("GET /head-uninitvar?pkgs=1 HTTP/1.1", '/head-uninitvar', {'pkgs': '1'})
    _test_parse_req("GET /crash.html?pkgs=1 HTTP/1.1", '/crash.html', {'pkgs': '1'})
    _test_parse_req("GET /head-uninitvar?pkgs=1&pkgs2=2 HTTP/1.1", '/head-uninitvar', {'pkgs': '1', 'pkgs2': '2'})

class _Connection:
    def __init__(self):
        self.data = b''

    def send(self, data):
        self.data += data
        return len(data)

    def close(self):
        pass

def _http_get(result_path, url):
    conn = _Connection()
    donate_cpu_server.HttpClientThread(conn, 'GET {} HTTP/1.1'.format(url), str(result_path), []).run()
    return conn.data.decode('utf-8')

def test_file_outside_result_path(tmp_path):
    result_path = tmp_path / 'donated-results'
    result_path.mkdir()
    (result_path / 'pkg').write_text('result data')
    (tmp_path / 'secret.txt').write_text('secret data')

    resp = _http_get(result_path, '/pkg')
    assert resp.startswith('HTTP/1.1 200 OK') and resp.endswith('result data')

    resp = _http_get(result_path, '/../secret.txt')
    assert resp.startswith('HTTP/1.1 404 Not Found')
    assert 'secret data' not in resp
