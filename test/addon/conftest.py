"""pytest configuration for the addon tests.

The tests exercise addons/cppcheckdata.py against dump files that are
generated on the fly with the cppcheck binary given by --cppcheck-binary.
"""
import os
import shutil
import subprocess
import sys

import pytest

# Make 'import cppcheckdata' resolve to <repo>/addons/cppcheckdata.py
_ADDONS_DIR = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'addons'))
if _ADDONS_DIR not in sys.path:
    sys.path.insert(0, _ADDONS_DIR)


def pytest_addoption(parser):
    parser.addoption('--cppcheck-binary',
                     default='cppcheck',
                     help='path to the cppcheck binary used to generate dump files '
                          '(default: cppcheck found in PATH)')


@pytest.fixture(scope='session')
def cppcheck_binary(request):
    binary = request.config.getoption('--cppcheck-binary')
    resolved = shutil.which(binary)
    if resolved is None:
        pytest.fail("cppcheck binary '%s' not found - point --cppcheck-binary at a cppcheck executable" % binary)
    return os.path.abspath(resolved)


class DumpFactory:
    """Runs 'cppcheck --dump' on a source snippet and parses the result."""

    def __init__(self, binary, tmp_path_factory):
        self.binary = binary
        self.tmp_path_factory = tmp_path_factory

    def create(self, code, filename='test.c', extra_args=()):
        """Write the code to a file, dump it and return the dump file path."""
        directory = self.tmp_path_factory.mktemp('cppcheckdata')
        path = directory / filename
        path.write_text(code)
        cmd = [self.binary, '--dump', '--quiet', str(path)] + list(extra_args)
        proc = subprocess.run(cmd,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE,
                              check=True,
                              universal_newlines=True)
        assert proc.returncode == 0, \
            'cppcheck failed with exit code %d:\n%s\n%s' % (proc.returncode, proc.stdout, proc.stderr)
        return str(path) + '.dump'

    def parse(self, code, filename='test.c', extra_args=()):
        """Write the code to a file, dump it and return the parsed CppcheckData."""
        import cppcheckdata
        return cppcheckdata.parsedump(self.create(code, filename, extra_args))


@pytest.fixture(scope='session')
def dump_factory(cppcheck_binary, tmp_path_factory):
    return DumpFactory(cppcheck_binary, tmp_path_factory)


SAMPLE_C = """#define ANSWER 42
static int add(int a, int b)
{
    return a + b;
}

double half(double d)
{
    return d / 2.0;
}

int main(void)
{
    int x = ANSWER;
    int arr[10];
    arr[0] = add(x, 1);
    int neg = -x;
    return arr[0] + neg;
}
"""

SAMPLE_CPP = """namespace ns {
    int twice(int v) { return 2 * v; }
}

class Shape {
public:
    virtual ~Shape() {}
    virtual double area() const = 0;
protected:
    double mScale;
};

enum Color { RED, GREEN };

const int limit = 5;
bool flag = true;
const char *msg = "hello";
char ch = 'x';

int run()
{
    Color color = RED;
    if (flag && limit > 1) {
        return ns::twice(21);
    }
    return static_cast<int>(color);
}
"""

MULTI_CFG_C = """typedef int myint;
typedef float myfloat;
#if defined(FOO) && FOO > 1
int foo(void) { return 1; }
#endif
myint bar(void) { myint y = 3; return y; }
"""

MATCH_C = """struct Point {
    int x;
    int y;
};

int calc(int a, int b)
{
    int bit_or = a | b;
    int log_or = a || b;
    int mod = a % b;
    int mul = a * b;
    int not_a = !a;
    int neq = a != b;
    int arr[3];
    arr[0] = a;
    a += 1;
    if (a > b) {
        return a;
    }
    return calc(a, b);
}
"""

MATCH_CPP = """class Widget {};

bool use(int i, int j)
{
    std::vector<int> v;
    bool less = i < j;
    return less && v.empty();
}
"""


@pytest.fixture(scope='session')
def sample_data(dump_factory):
    """Parsed dump of the canonical C sample."""
    return dump_factory.parse(SAMPLE_C, filename='sample.c')


@pytest.fixture(scope='session')
def sample_cfg(sample_data):
    cfgs = sample_data.configurations
    assert len(cfgs) == 1
    return cfgs[0]


@pytest.fixture(scope='session')
def sample_cpp_data(dump_factory):
    """Parsed dump of the canonical C++ sample."""
    return dump_factory.parse(SAMPLE_CPP, filename='sample.cpp')


@pytest.fixture(scope='session')
def sample_cpp_cfg(sample_cpp_data):
    cfgs = sample_cpp_data.configurations
    assert len(cfgs) == 1
    return cfgs[0]


@pytest.fixture(scope='session')
def multi_cfg_data(dump_factory):
    """Parsed dump of a file with two preprocessor configurations."""
    return dump_factory.parse(MULTI_CFG_C, filename='multi.c')


@pytest.fixture(scope='session')
def match_cfg(dump_factory):
    """Configuration of a C sample covering the match() pattern syntax."""
    return dump_factory.parse(MATCH_C, filename='match.c').configurations[0]


@pytest.fixture(scope='session')
def match_cpp_cfg(dump_factory):
    """Configuration of a C++ sample with linked '<' tokens for match()."""
    return dump_factory.parse(MATCH_CPP, filename='match.cpp').configurations[0]
