
# python -m pytest performance_test.py

import os
import sys

import pytest

from testutils import cppcheck



@pytest.mark.timeout(10)
def test_slow_array_many_floats(tmpdir):
    # 11649
    # cppcheck valueflow takes a long time when an array has many floats
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("const float f[] = {\n")
        for _ in range(20000):
            f.write('    13.6f,\n')
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(10)
def test_slow_array_many_strings(tmpdir):
    # 11901
    # cppcheck valueflow takes a long time when analyzing a file with many strings
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("const char *strings[] = {\n")
        for _ in range(20000):
            f.write('    "abc",\n')
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(10)
def test_slow_long_line(tmpdir):
    # simplecpp #314
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("#define A() static const int a[] = {\\\n")
        for _ in range(5000):
            f.write(" -123, 456, -789,\\\n")
        f.write("};\n")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(60)
def test_slow_large_constant_expression(tmpdir):
    # 12182
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("""
#define FLAG1 0
#define FLAG2 0
#define FLAG3 0
#define FLAG4 0
#define FLAG5 0
#define FLAG6 0
#define FLAG7 0
#define FLAG8 0
#define FLAG9 0
#define FLAG10 0
#define FLAG11 0
#define FLAG12 0
#define FLAG13 0
#define FLAG14 0
#define FLAG15 0
#define FLAG16 0
#define FLAG17 0
#define FLAG18 0
#define FLAG19 0
#define FLAG20 0
#define FLAG21 0
#define FLAG22 0
#define FLAG23 0
#define FLAG24 0

#define maxval(x, y) ((x) > (y) ? (x) : (y))

#define E_SAMPLE_SIZE   maxval( FLAG1,                \
                                  maxval( FLAG2,      \
                                  maxval( FLAG3,      \
                                  maxval( FLAG4,      \
                                  maxval( FLAG5,      \
                                  maxval( FLAG6,      \
                                  maxval( FLAG7,      \
                                  maxval( FLAG8,      \
                                  maxval( FLAG9,      \
                                  maxval( FLAG10,     \
                                  maxval( FLAG11,     \
                                  maxval( FLAG12,     \
                                  maxval( FLAG13,     \
                                  maxval( FLAG14,     \
                                  FLAG15 ))))))))))))))

#define SAMPLE_SIZE       maxval( E_SAMPLE_SIZE,      \
                                  maxval( sizeof(st), \
                                  maxval( FLAG16,     \
                                  maxval( FLAG17,     \
                                  maxval( FLAG18,     \
                                  maxval( FLAG19,     \
                                  maxval( FLAG20,     \
                                  maxval( FLAG21,     \
                                  maxval( FLAG22,     \
                                  maxval( FLAG23,     \
                                          FLAG24 ))))))))))

typedef struct {
    int n;
} st;

x = SAMPLE_SIZE;
        """)

    cppcheck([filename])

@pytest.mark.timeout(10)
def test_slow_exprid(tmpdir):
    # 11885
    filename = os.path.join(tmpdir, 'hang.c')
    with open(filename, 'wt') as f:
        f.write("""
int foo(int a, int b)
{
#define A0(a, b)  ((a) + (b))
#define A1(a, b)  ((a) > (b)) ? A0((a) - (b), (b)) : A0((b) - (a), (a))
#define A2(a, b)  ((a) > (b)) ? A1((a) - (b), (b)) : A1((b) - (a), (a))
#define A3(a, b)  ((a) > (b)) ? A2((a) - (b), (b)) : A2((b) - (a), (a))
#define A4(a, b)  ((a) > (b)) ? A3((a) - (b), (b)) : A3((b) - (a), (a))
#define A5(a, b)  ((a) > (b)) ? A4((a) - (b), (b)) : A4((b) - (a), (a))
#define A6(a, b)  ((a) > (b)) ? A5((a) - (b), (b)) : A5((b) - (a), (a))
#define A7(a, b)  ((a) > (b)) ? A6((a) - (b), (b)) : A6((b) - (a), (a))
#define A8(a, b)  ((a) > (b)) ? A7((a) - (b), (b)) : A7((b) - (a), (a))
#define A9(a, b)  ((a) > (b)) ? A8((a) - (b), (b)) : A8((b) - (a), (a))
#define A10(a, b) ((a) > (b)) ? A9((a) - (b), (b)) : A9((b) - (a), (a))
#define A11(a, b) ((a) > (b)) ? A10((a) - (b), (b)) : A10((b) - (a), (a))
	return A8(a, b);
}
        """)

    my_env = os.environ.copy()
    my_env["DISABLE_VALUEFLOW"] = "1"
    cppcheck([filename], env=my_env)

@pytest.mark.timeout(35)
def test_stack_overflow_AST(tmpdir):
    # 14435
    filename = os.path.join(tmpdir, 'hang.cpp')
    with open(filename, 'wt') as f:
        f.write("""
#define ROW 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
#define ROW8 ROW ROW ROW ROW ROW ROW ROW ROW
#define ROW64 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8
#define ROW512 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64
#define ROW4096 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512
#define ROW32768 ROW4096 ROW4096 ROW4096 ROW4096 ROW4096 ROW4096 ROW4096 ROW4096
void f(std::vector<int>& v) {
	v = {
		ROW32768 0
	};
}
        """)

    my_env = os.environ.copy()
    my_env["DISABLE_VALUEFLOW"] = "1"
    cppcheck([filename], env=my_env)


@pytest.mark.timeout(10)
def test_slow_initlist_varchanged(tmpdir):
    # #12235
    filename = os.path.join(tmpdir, 'hang.cpp')
    with open(filename, 'wt') as f:
        f.write(r"""
                struct T {
                    int* q;
                    int nx, ny;
                };
                struct S {
                    void f();
                    int n;
                    T* p;
                };
                #define ROW 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 ,
                #define ROW4 ROW ROW ROW ROW
                #define ROW16 ROW4 ROW4 ROW4 ROW4
                #define ROW64 ROW16 ROW16 ROW16 ROW16
                #define ROW256 ROW64 ROW64 ROW64 ROW64
                #define ROW1K ROW256 ROW256 ROW256 ROW256
                #define ROW4K ROW1K ROW1K ROW1K ROW1K
                const int A[] = {
                    ROW4K
                };
                void S::f() {
                    for (int i = 0; i < n; ++i) {
                        T& t = p[i];
                        for (int y = 0; y < t.ny; y += 4) {
                            int* row0 = t.q + y * t.nx;
                            for (int x = 0; x < t.nx; x += 4) {
                                int s[16] = {};
                                memcpy(row0, &s[0], 4);
                                row0 += 4;
                            }
                        }
                    }
                }""")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.timeout(10)
def test_slow_many_scopes(tmpdir):
    # #12038
    filename = os.path.join(tmpdir, 'hang.cpp')
    with open(filename, 'wt') as f:
        f.write(r"""
                #define BLOCK {\
                    char buf[sizeof("x") + 5 * 3 + 16];\
                    int rc;\
                    memset(buf, cmp[0], sizeof buf);\
                    rc = sprintf(buf + 5, "x");\
                    assert(rc == sizeof("x") - 1);\
                    assert(memcmp(buf, cmp, 5) == 0);\
                    if ((rc) >= 0) {\
                        assert(memcmp(buf + 5, ("x"), (rc)) == 0);\
                        assert(buf[5 + (rc)] == '\0');\
                    }\
                    assert(memcmp(buf + 5 + (rc) + 1, cmp, 5) == 0);\
                }
                #define BLOCK2 BLOCK BLOCK
                #define BLOCK4 BLOCK2 BLOCK2
                #define BLOCK8 BLOCK4 BLOCK4
                #define BLOCK16 BLOCK8 BLOCK8
                #define BLOCK32 BLOCK16 BLOCK16
                #define BLOCK64 BLOCK32 BLOCK32
                int main() {
                    char cmp[5];
                    memset(cmp, '\376', sizeof cmp);
                    BLOCK64
                    return EXIT_SUCCESS;
                }""")
    cppcheck([filename]) # should not take more than ~1 second


@pytest.mark.skipif(sys.platform == 'darwin', reason='GitHub macOS runners are too slow')
@pytest.mark.timeout(20)
def test_crash_array_in_namespace(tmpdir):
    # 12847
    filename = os.path.join(tmpdir, 'hang.cpp')
    with open(filename, 'wt') as f:
        f.write(r"""
                #define ROW A, A, A, A, A, A, A, A,
                #define ROW8 ROW ROW ROW ROW ROW ROW ROW ROW
                #define ROW64 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8
                #define ROW512 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64
                #define ROW4096 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512
                namespace N {
                    static const char A = 'a';
                    const char a[] = {
                        ROW4096 ROW4096 ROW4096 ROW4096
                    };
                }""")
    cppcheck([filename]) # should not take more than ~5 seconds


@pytest.mark.skipif(sys.platform == 'darwin', reason='GitHub macOS runners are too slow')
@pytest.mark.timeout(20)
def test_crash_array_in_array(tmpdir):
    # 12861
    filename = os.path.join(tmpdir, 'hang.cpp')
    with open(filename, 'wt') as f:
        f.write(r"""
                #define ROW A, A, A, A, A, A, A, A,
                #define ROW8 ROW ROW ROW ROW ROW ROW ROW ROW
                #define ROW64 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8 ROW8
                #define ROW512 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64 ROW64
                #define ROW4096 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512 ROW512
                void f() {
                    static const char A = 'a';
                    const char a[] = {
                        ROW4096 ROW4096 ROW4096 ROW4096
                    };
                }""")
    cppcheck([filename])


@pytest.mark.timeout(5)
def test_slow_bifurcate(tmpdir):
    # #14134
    filename = os.path.join(tmpdir, 'hang.cpp')
    with open(filename, 'wt') as f:
        f.write(r"""
                class C {
                public:
	                enum class Status {
                		Ok,
                		Waiting,
                		Error,
                	};
                	void setStatus(Status status, const QString& text);
                	QColor m_statusColor;
                };

                template < size_t N, typename T >
                inline QDataStream& deserialize(QDataStream& in, T& value) {
                	if constexpr (N == 0) {}
                	else if constexpr (N == 1) {
                		auto& [f1] = value;
                		in >> f1;
                	}
                	else if constexpr (N == 2) {
                		auto& [f1, f2] = value;
                		in >> f1 >> f2;
                	}
                	else if constexpr (N == 3) {
                		auto& [f1, f2, f3] = value;
                		in >> f1 >> f2 >> f3;
                	}
                	else if constexpr (N == 4) {
                		auto& [f1, f2, f3, f4] = value;
                		in >> f1 >> f2 >> f3 >> f4;
                	}
                	else if constexpr (N == 5) {
                		auto& [f1, f2, f3, f4, f5] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5;
                	}
                	else if constexpr (N == 6) {
                		auto& [f1, f2, f3, f4, f5, f6] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6;
                	}
                	else if constexpr (N == 7) {
                		auto& [f1, f2, f3, f4, f5, f6, f7] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7;
                	}
                	else if constexpr (N == 8) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8;
                	}
                	else if constexpr (N == 9) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9;
                	}
                	else if constexpr (N == 10) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10;
                	}
                	else if constexpr (N == 11) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11;
	                }
                	else if constexpr (N == 12) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12;
                	}
                	else if constexpr (N == 13) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13;
                	}
                	else if constexpr (N == 14) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13 >> f14;
                	}
                	else if constexpr (N == 15) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13 >> f14 >> f15;
                	}
                	else if constexpr (N == 16) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13 >> f14 >> f15 >> f16;
                	}
                	else if constexpr (N == 17) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13 >> f14 >> f15 >> f16 >> f17;
                	}
                	else if constexpr (N == 18) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13 >> f14 >> f15 >> f16 >> f17 >> f18;
                	}
                	else if constexpr (N == 19) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13 >> f14 >> f15 >> f16 >> f17 >> f18 >> f19;
                	}
                	else if constexpr (N == 20) {
                		auto& [f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20] = value;
                		in >> f1 >> f2 >> f3 >> f4 >> f5 >> f6 >> f7 >> f8 >> f9 >> f10 >> f11 >> f12 >> f13 >> f14 >> f15 >> f16 >> f17 >> f18 >> f19 >> f20;
                	}
                	else {
                		static_assert (!sizeof(T), "missing implementation");
                	}
                	return in;
                }

                void C::setStatus(Status status, const QString& text) {
                	const auto& colors = Config::get().ui().colors;
                	QColor color;
                	switch (status) {
                	case Status::Ok:
                		color = colors.green;
                		break;
                	case Status::Waiting:
                		color = Qt::black;
                		break;
                	case Status::Error:
                		color = colors.red;
                		break;
	                }

                	if (m_statusColor != color) {
                		m_statusColor = color;
                	}
                }""")
    cppcheck([filename]) # should not take more than ~1 second
