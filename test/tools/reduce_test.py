#!/usr/bin/env python
from reduce import Reduce


class ReduceTest(Reduce):
    # pylint: disable-next=super-init-not-called - we do not want the super __init__ to be called
    def __init__(self):
        # super().__init__('', '', '')
        pass

    def runtool(self, filedata=None):
        return True

    def writefile(self, filedata):
        pass

    def writebackupfile(self, filedata):
        pass


def test_removecomments():
    """make sure we keep the \n when removing a comment at the end of a line"""

    reduce = ReduceTest()

    filedata = [
        'int i; // some integer\n',
        'int j;\n'
    ]

    expected = [
        'int i;\n',
        'int j;\n'
    ]

    reduce.removecomments(filedata)
    assert filedata == expected


def test_removedirectives():
    """do not remove any of the #if*, #el* or #endif directives on their own"""

    reduce = ReduceTest()

    filedata = [
        '#if 0\n',
        '#else\n',
        '#endif\n',
        '#ifdef DEF\n',
        '#elif 0\n'
        '#endif\n'
    ]

    expected = [
        '#if 0\n',
        '#else\n',
        '#endif\n',
        '#ifdef DEF\n',
        '#elif 0\n'
        '#endif\n'
    ]

    reduce.removedirectives(filedata)
    assert filedata == expected


def test_combinelines_chunk():
    """do not fail with 'TypeError: slice indices must be integers or None or have an __index__ method'"""

    class ReduceTestFail(ReduceTest):
        def runtool(self, filedata=None):
            print(filedata)
            return False

    reduce = ReduceTestFail()

    # need to have at least 11 lines ending with comma to enter chunked mode and twice as much for second iteration
    filedata = [
        'int i,\n',
        'j,\n',
        'k,\n',
        'l,\n',
        'm,\n',
        'n,\n',
        'o,\n',
        'p,\n',
        'q,\n',
        'r,\n',
        's,\n',
        't;\n',
        'int i1,\n',
        'j1,\n',
        'k1,\n',
        'l1,\n',
        'm1,\n',
        'n1,\n',
        'o1,\n',
        'p1,\n',
        'q1,\n',
        'r1,\n',
        's1,\n',
        't1;\n'
    ]

    reduce.combinelines(filedata)


def test_combinelines_chunk_2():
    """'filedata' is not changed by the function since the data is assigned to a local variable"""

    reduce = ReduceTest()

    # need to have at least 11 lines ending with comma to enter chunked mode
    filedata = [
        'int i,\n',
        'j,\n',
        'k,\n',
        'l,\n',
        'm,\n',
        'n,\n',
        'o,\n',
        'p,\n',
        'q,\n',
        'r,\n',
        's,\n',
        't;\n'
    ]
    filedata_exp = filedata

    filedata2 = reduce.combinelines(filedata)
    assert filedata == filedata_exp
    assert filedata2 == ['int i,j,\n',
                         '',
                         'l,\n',
                         'm,\n',
                         'n,\n',
                         'o,\n',
                         'p,\n',
                         'q,\n',
                         'r,\n',
                         's,\n',
                         't;\n',
                         '']
