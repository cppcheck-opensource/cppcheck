/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "path.h"
#include "fixture.h"
#include "helpers.h"
#include "standards.h"

#include <initializer_list>
#include <list>
#include <set>
#include <string>
#include <vector>

#ifndef _WIN32
#include <stdexcept>
#endif

class TestPath : public TestFixture {
public:
    TestPath() : TestFixture("TestPath") {}

private:

    void run() override {
        TEST_CASE(removeQuotationMarks);
        TEST_CASE(acceptFile);
        TEST_CASE(getCurrentPath);
        TEST_CASE(getCurrentExecutablePath);
        TEST_CASE(isAbsolute);
        TEST_CASE(getRelative);
        TEST_CASE(get_path_from_filename);
        TEST_CASE(join);
        TEST_CASE(isDirectory);
        TEST_CASE(isFile);
        TEST_CASE(sameFileName);
        TEST_CASE(getFilenameExtension);
        TEST_CASE(identify);
        TEST_CASE(identifyWithCppProbe);
        TEST_CASE(is_header);
        TEST_CASE(simplifyPath);
        TEST_CASE(getAbsolutePath);
        TEST_CASE(exists);
        TEST_CASE(fromNativeSeparators);
        TEST_CASE(isRelative);
    }

    void removeQuotationMarks() const {
        // Path::removeQuotationMarks()
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("index.cpp"));
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("\"index.cpp"));
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("index.cpp\""));
        ASSERT_EQUALS("index.cpp", Path::removeQuotationMarks("\"index.cpp\""));
        ASSERT_EQUALS("path to/index.cpp", Path::removeQuotationMarks("\"path to\"/index.cpp"));
        ASSERT_EQUALS("path to/index.cpp", Path::removeQuotationMarks("\"path to/index.cpp\""));
        ASSERT_EQUALS("the/path to/index.cpp", Path::removeQuotationMarks("the/\"path to\"/index.cpp"));
        ASSERT_EQUALS("the/path to/index.cpp", Path::removeQuotationMarks("\"the/path to/index.cpp\""));
    }

    void acceptFile() const {
        ASSERT(Path::acceptFile("index.c"));
        ASSERT(Path::acceptFile("index.cpp"));
        ASSERT(Path::acceptFile("index.invalid.cpp"));
        ASSERT(Path::acceptFile("index.invalid.Cpp"));
        ASSERT(Path::acceptFile("index.invalid.C"));
        ASSERT(Path::acceptFile("index.invalid.C++"));
        ASSERT(Path::acceptFile("index.")==false);
        ASSERT(Path::acceptFile("index")==false);
        ASSERT(Path::acceptFile("")==false);
        ASSERT(Path::acceptFile("C")==false);
        {
            Standards::Language lang;
            ASSERT(Path::acceptFile("index.c", &lang));
            ASSERT_EQUALS_ENUM(Standards::Language::C, lang);
        }
        {
            Standards::Language lang;
            ASSERT(Path::acceptFile("index.cpp", &lang));
            ASSERT_EQUALS_ENUM(Standards::Language::CPP, lang);
        }

        // don't accept any headers
        ASSERT_EQUALS(false, Path::acceptFile("index.h"));
        ASSERT_EQUALS(false, Path::acceptFile("index.hpp"));

        const std::set<std::string> extra = { ".extra", ".header" };
        ASSERT(Path::acceptFile("index.c", extra));
        ASSERT(Path::acceptFile("index.cpp", extra));
        ASSERT(Path::acceptFile("index.extra", extra));
        ASSERT(Path::acceptFile("index.header", extra));
        ASSERT(Path::acceptFile("index.h", extra)==false);
        ASSERT(Path::acceptFile("index.hpp", extra)==false);
        {
            Standards::Language lang;
            ASSERT(Path::acceptFile("index.c", extra, &lang));
            ASSERT_EQUALS_ENUM(Standards::Language::C, lang);
        }
        {
            Standards::Language lang;
            ASSERT(Path::acceptFile("index.cpp", extra, &lang));
            ASSERT_EQUALS_ENUM(Standards::Language::CPP, lang);
        }
        {
            Standards::Language lang;
            ASSERT(Path::acceptFile("index.extra", extra, &lang));
            ASSERT_EQUALS_ENUM(Standards::Language::None, lang);
        }
    }

    void getCurrentPath() const {
        ASSERT_EQUALS(true, Path::isAbsolute(Path::getCurrentPath()));
    }

    void getCurrentExecutablePath() const {
        ASSERT_EQUALS(false, Path::getCurrentExecutablePath("").empty());
    }

    void isAbsolute() const {
#ifdef _WIN32
        ASSERT_EQUALS(true, Path::isAbsolute("C:\\foo\\bar"));
        ASSERT_EQUALS(true, Path::isAbsolute("C:/foo/bar"));
        ASSERT_EQUALS(true, Path::isAbsolute("\\\\foo\\bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo\\bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo/bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo.cpp"));
        ASSERT_EQUALS(false, Path::isAbsolute("C:foo.cpp"));
        ASSERT_EQUALS(false, Path::isAbsolute("C:foo\\bar.cpp"));
        ASSERT_EQUALS(false, Path::isAbsolute("bar.cpp"));
        TODO_ASSERT_EQUALS(true, false, Path::isAbsolute("\\"));
#else
        ASSERT_EQUALS(true, Path::isAbsolute("/foo/bar"));
        ASSERT_EQUALS(true, Path::isAbsolute("/"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo/bar"));
        ASSERT_EQUALS(false, Path::isAbsolute("foo.cpp"));
#endif
    }

    void getRelative() const {
        const std::vector<std::string> basePaths = {
            "", // Don't crash with empty paths
            "C:/foo",
            "C:/bar/",
            "C:/test.cpp"
        };

        ASSERT_EQUALS("x.c", Path::getRelativePath("C:/foo/x.c", basePaths));
        ASSERT_EQUALS("y.c", Path::getRelativePath("C:/bar/y.c", basePaths));
        ASSERT_EQUALS("foo/y.c", Path::getRelativePath("C:/bar/foo/y.c", basePaths));
        ASSERT_EQUALS("C:/test.cpp", Path::getRelativePath("C:/test.cpp", basePaths));
        ASSERT_EQUALS("C:/foobar/test.cpp", Path::getRelativePath("C:/foobar/test.cpp", basePaths));
    }

    void get_path_from_filename() const {
        ASSERT_EQUALS("", Path::getPathFromFilename("index.h"));
        ASSERT_EQUALS("/tmp/", Path::getPathFromFilename("/tmp/index.h"));
        ASSERT_EQUALS("a/b/c/", Path::getPathFromFilename("a/b/c/index.h"));
        ASSERT_EQUALS("a/b/c/", Path::getPathFromFilename("a/b/c/"));
    }

    void join() const {
        ASSERT_EQUALS("", Path::join("", ""));
        ASSERT_EQUALS("b", Path::join("", "b"));
        ASSERT_EQUALS("/b", Path::join("", "/b"));
        ASSERT_EQUALS("/b", Path::join("", "\\b"));

        ASSERT_EQUALS("a", Path::join("a", ""));
        ASSERT_EQUALS("a/b", Path::join("a", "b"));
        ASSERT_EQUALS("/b", Path::join("a", "/b"));
        ASSERT_EQUALS("/b", Path::join("a", "\\b"));

        ASSERT_EQUALS("a/", Path::join("a/", ""));
        ASSERT_EQUALS("a/b", Path::join("a/", "b"));
        ASSERT_EQUALS("/b", Path::join("a/", "/b"));
        ASSERT_EQUALS("/b", Path::join("a/", "\\b"));

        ASSERT_EQUALS("a/", Path::join("a\\", ""));
        ASSERT_EQUALS("a/b", Path::join("a\\", "b"));
        ASSERT_EQUALS("/b", Path::join("a\\", "/b"));
        ASSERT_EQUALS("/b", Path::join("a\\", "\\b"));

        // TODO: how to absolute Windows path in path2?
        //ASSERT_EQUALS("", Path::join("a", "s:/b"));

        //ASSERT_EQUALS("", Path::join("S:\\a", "S:/b"));
        //ASSERT_EQUALS("", Path::join("S:\\a", "S:\\b"));
        //ASSERT_EQUALS("", Path::join("S:\\a", "/b"));

        //ASSERT_EQUALS("", Path::join("S:/a", "S:/b"));
        //ASSERT_EQUALS("", Path::join("S:/a", "S:\\b"));
        //ASSERT_EQUALS("", Path::join("S:/a", "/b"));
    }

    void isDirectory() const {
        ScopedFile file("testpath.txt", "", "testpath");
        ScopedFile file2("testpath2.txt", "");
        ASSERT_EQUALS(false, Path::isDirectory("testpath.txt"));
        ASSERT_EQUALS(true, Path::isDirectory("testpath"));
        ASSERT_EQUALS(false, Path::isDirectory("testpath/testpath.txt"));
        ASSERT_EQUALS(false, Path::isDirectory("testpath2.txt"));
    }

    void isFile() const {
        ScopedFile file("testpath.txt", "", "testpath");
        ScopedFile file2("testpath2.txt", "");
        ASSERT_EQUALS(false, Path::isFile("testpath"));
        ASSERT_EQUALS(false, Path::isFile("testpath.txt"));
        ASSERT_EQUALS(true, Path::isFile("testpath/testpath.txt"));
        ASSERT_EQUALS(true, Path::isFile("testpath2.txt"));
    }

    void sameFileName() const {
        ASSERT(Path::sameFileName("test", "test"));

        // case sensitivity cases
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
        ASSERT(Path::sameFileName("test", "Test"));
        ASSERT(Path::sameFileName("test", "TesT"));
        ASSERT(Path::sameFileName("test.h", "test.H"));
        ASSERT(Path::sameFileName("test.hh", "test.Hh"));
        ASSERT(Path::sameFileName("test.hh", "test.hH"));
#else
        ASSERT(!Path::sameFileName("test", "Test"));
        ASSERT(!Path::sameFileName("test", "TesT"));
        ASSERT(!Path::sameFileName("test.h", "test.H"));
        ASSERT(!Path::sameFileName("test.hh", "test.Hh"));
        ASSERT(!Path::sameFileName("test.hh", "test.hH"));
#endif
    }

    void getFilenameExtension() const {
        ASSERT_EQUALS("", Path::getFilenameExtension("test"));
        ASSERT_EQUALS("", Path::getFilenameExtension("Test"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.h"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("Test.h"));
        ASSERT_EQUALS("", Path::getFilenameExtension("test", true));
        ASSERT_EQUALS("", Path::getFilenameExtension("Test", true));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.h", true));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("Test.h", true));

        // case sensitivity cases
#if defined(_WIN32) || (defined(__APPLE__) && defined(__MACH__))
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.H"));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.Hh"));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.hH"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.H", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.Hh", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.hH", true));
#else
        ASSERT_EQUALS(".H", Path::getFilenameExtension("test.H"));
        ASSERT_EQUALS(".Hh", Path::getFilenameExtension("test.Hh"));
        ASSERT_EQUALS(".hH", Path::getFilenameExtension("test.hH"));
        ASSERT_EQUALS(".h", Path::getFilenameExtension("test.H", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.Hh", true));
        ASSERT_EQUALS(".hh", Path::getFilenameExtension("test.hH", true));
#endif
    }


    void identify() const {
        Standards::Language lang;
        bool header;

        ASSERT_EQUALS(Standards::Language::None, Path::identify("", false));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("c", false));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("cpp", false));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("h", false));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("hpp", false));

        // TODO: what about files starting with a "."?
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".c", false));
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".cpp", false));
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".h", false));
        //ASSERT_EQUALS(Standards::Language::None, Path::identify(".hpp", false));

        // C
        ASSERT_EQUALS(Standards::Language::C, Path::identify("index.c", false));
        ASSERT_EQUALS(Standards::Language::C, Path::identify("index.cl", false));
        ASSERT_EQUALS(Standards::Language::C, Path::identify("C:\\foo\\index.c", false));
        ASSERT_EQUALS(Standards::Language::C, Path::identify("/mnt/c/foo/index.c", false));

        // In unix .C is considered C++
#ifdef _WIN32
        ASSERT_EQUALS(Standards::Language::C, Path::identify("C:\\foo\\index.C", false));
#endif

        lang = Path::identify("index.c", false, &header);
        ASSERT_EQUALS(Standards::Language::C, lang);
        ASSERT_EQUALS(false, header);

        // C++
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.cpp", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.cxx", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.cc", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.c++", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.tpp", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.txx", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.ipp", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.ixx", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("C:\\foo\\index.cpp", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("C:\\foo\\index.Cpp", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("/mnt/c/foo/index.cpp", false));
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("/mnt/c/foo/index.Cpp", false));

        // TODO: check for case-insensitive filesystem instead
        // In unix .C is considered C++
#if !defined(_WIN32) && !(defined(__APPLE__) && defined(__MACH__))
        ASSERT_EQUALS(Standards::Language::CPP, Path::identify("index.C", false));
#else
        ASSERT_EQUALS(Standards::Language::C, Path::identify("index.C", false));
#endif

        lang = Path::identify("index.cpp", false, &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(false, header);

        // headers
        lang = Path::identify("index.h", false, &header);
        ASSERT_EQUALS(Standards::Language::C, lang);
        ASSERT_EQUALS(true, header);

        lang = Path::identify("index.hpp", false, &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);
        lang = Path::identify("index.hxx", false, &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);
        lang = Path::identify("index.h++", false, &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);
        lang = Path::identify("index.hh", false, &header);
        ASSERT_EQUALS(Standards::Language::CPP, lang);
        ASSERT_EQUALS(true, header);

        ASSERT_EQUALS(Standards::Language::None, Path::identify("index.header", false));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("index.htm", false));
        ASSERT_EQUALS(Standards::Language::None, Path::identify("index.html", false));
    }

#define identifyWithCppProbeInternal(...) identifyWithCppProbeInternal_(__FILE__, __LINE__, __VA_ARGS__)
    void identifyWithCppProbeInternal_(const char* file, int line, const std::string& filename, const std::string& marker, Standards::Language std) const
    {
        const ScopedFile f(filename, marker);
        ASSERT_EQUALS_LOC_MSG(std, Path::identify(f.path(), true), filename + "\n" + marker, file, line);
    }

    void identifyWithCppProbe() const
    {
        const std::list<std::string> markers_cpp = {
            "// -*- C++ -*-",
            "// -*-C++-*-",
            "// -*- Mode: C++; -*-",
            "// -*-Mode: C++;-*-",
            "// -*- Mode:C++; -*-",
            "// -*-Mode:C++;-*-",
            "// -*- Mode: C++ -*-",
            "// -*-Mode: C++-*-",
            "// -*- Mode:C++ -*-",
            "// -*-Mode:C++-*-",
            "// -*- Mode: C++; c-basic-offset: 8 -*-",
            "// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-",

            "// -*- c++ -*-",
            "// -*- mode: c++; -*-",

            "/* -*- C++ -*- */",
            "/* -*- C++ -*-",

            "//-*- C++ -*-",
            " //-*- C++ -*-",
            "\t//-*- C++ -*-",
            "\t //-*- C++ -*-",
            " \t//-*- C++ -*-",
            "//-*- C++ -*- ",
            "//-*- C++ -*- \n",
            "// -----*- C++ -*-----",
            "// comment-*- C++ -*-comment",
            "// -*- C++ -*-\n",
            "//-*- C++ -*-\r// comment",
            "//-*- C++ -*-\n// comment",
            "//-*- C++ -*-\r\n// comment",

            "/* -*-C++-*- */",
            "/*-*-C++-*-*/",
            " /*-*-C++-*-*/",
        };

        for (const auto& f : { "cppprobe.h", "cppprobe" }) {
            for (const auto& m : markers_cpp) {
                identifyWithCppProbeInternal(f, m, Standards::Language::CPP);
            }
        }

        const std::list<std::string> markers_c = {
            "-*- C++ -*-", // needs to be in comment
            "// -*- C++", // no end marker
            "// -*- C++ --*-", // incorrect end marker
            "// -*- C++/-*-", // unexpected character
            "// comment\n// -*- C++ -*-", // not on the first line
            "// comment\r// -*- C++ -*-", // not on the first line
            "// comment\r\n// -*- C++ -*-", // not on the first line
            "// -*- C -*-",
            "// -*- Mode: C; -*-",
            "// -*- f90 -*-",
            "// -*- fortran -*-",
            "// -*- c-basic-offset: 2 -*-",
            "// -*- c-basic-offset:4; indent-tabs-mode:nil -*-",
            "// ", // no marker
            "// -*-", // incomplete marker
            "/*", // no marker
            "/**/", // no marker
            "/*\n*/", // no marker
            "/* */", // no marker
            "/* \n*/", // no marker
            "/* -*-", // incomplete marker
            "/* \n-*-", // incomplete marker
            "/* \n-*- C++ -*-", // not on the first line
            "/* \n-*- C++ -*- */" // not on the first line
        };

        for (const auto& m : markers_c) {
            identifyWithCppProbeInternal("cppprobe.h", m, Standards::Language::C);
        }
        for (const auto& m : markers_c) {
            identifyWithCppProbeInternal("cppprobe", m, Standards::Language::None);
        }
    }

    void is_header() const {
        ASSERT(Path::isHeader("index.h"));
        ASSERT(Path::isHeader("index.hpp"));
        ASSERT(Path::isHeader("index.hxx"));
        ASSERT(Path::isHeader("index.h++"));
        ASSERT(Path::isHeader("index.hh"));

        ASSERT(Path::isHeader("index.c")==false);
        ASSERT(Path::isHeader("index.cpp")==false);
        ASSERT(Path::isHeader("index.header")==false);
        ASSERT(Path::isHeader("index.htm")==false);
        ASSERT(Path::isHeader("index.html")==false);
    }

    void simplifyPath() const {
        ASSERT_EQUALS("file.cpp", Path::simplifyPath("file.cpp"));
        ASSERT_EQUALS("../file.cpp", Path::simplifyPath("../file.cpp"));
        ASSERT_EQUALS("file.cpp", Path::simplifyPath("test/../file.cpp"));
        ASSERT_EQUALS("../file.cpp", Path::simplifyPath("../test/../file.cpp"));

        ASSERT_EQUALS("file.cpp", Path::simplifyPath("./file.cpp"));
        ASSERT_EQUALS("../file.cpp", Path::simplifyPath("./../file.cpp"));
        ASSERT_EQUALS("file.cpp", Path::simplifyPath("./test/../file.cpp"));
        ASSERT_EQUALS("../file.cpp", Path::simplifyPath("./../test/../file.cpp"));

        ASSERT_EQUALS("test/", Path::simplifyPath("test/"));
        ASSERT_EQUALS("../test/", Path::simplifyPath("../test/"));
        ASSERT_EQUALS("../", Path::simplifyPath("../test/.."));
        ASSERT_EQUALS("../", Path::simplifyPath("../test/../"));

        ASSERT_EQUALS("/home/file.cpp", Path::simplifyPath("/home/test/../file.cpp"));
        ASSERT_EQUALS("/file.cpp", Path::simplifyPath("/home/../test/../file.cpp"));

        ASSERT_EQUALS("C:/home/file.cpp", Path::simplifyPath("C:/home/test/../file.cpp"));
        ASSERT_EQUALS("C:/file.cpp", Path::simplifyPath("C:/home/../test/../file.cpp"));

        ASSERT_EQUALS("../file.cpp", Path::simplifyPath("..\\file.cpp"));
        ASSERT_EQUALS("file.cpp", Path::simplifyPath("test\\..\\file.cpp"));
        ASSERT_EQUALS("../file.cpp", Path::simplifyPath("..\\test\\..\\file.cpp"));

        ASSERT_EQUALS("file.cpp", Path::simplifyPath(".\\file.cpp"));
        ASSERT_EQUALS("../file.cpp", Path::simplifyPath(".\\..\\file.cpp"));
        ASSERT_EQUALS("file.cpp", Path::simplifyPath(".\\test\\..\\file.cpp"));
        ASSERT_EQUALS("../file.cpp", Path::simplifyPath(".\\..\\test\\..\\file.cpp"));

        ASSERT_EQUALS("test/", Path::simplifyPath("test\\"));
        ASSERT_EQUALS("../test/", Path::simplifyPath("..\\test\\"));
        ASSERT_EQUALS("../", Path::simplifyPath("..\\test\\.."));
        ASSERT_EQUALS("../", Path::simplifyPath("..\\test\\..\\"));

        ASSERT_EQUALS("C:/home/file.cpp", Path::simplifyPath("C:\\home\\test\\..\\file.cpp"));
        ASSERT_EQUALS("C:/file.cpp", Path::simplifyPath("C:\\home\\..\\test\\..\\file.cpp"));

        ASSERT_EQUALS("//home/file.cpp", Path::simplifyPath("\\\\home\\test\\..\\file.cpp"));
        ASSERT_EQUALS("//file.cpp", Path::simplifyPath("\\\\home\\..\\test\\..\\file.cpp"));
    }

    void getAbsolutePath() const {
        const std::string cwd = Path::getCurrentPath();

        ScopedFile file("testabspath.txt", "");
        std::string expected = Path::toNativeSeparators(Path::join(cwd, "testabspath.txt"));

        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath("testabspath.txt"));
        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath("./testabspath.txt"));
        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath(".\\testabspath.txt"));
        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath("test/../testabspath.txt"));
        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath("test\\..\\testabspath.txt"));
        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath("./test/../testabspath.txt"));
        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath(".\\test\\../testabspath.txt"));

        ASSERT_EQUALS(expected, Path::getAbsoluteFilePath(Path::join(cwd, "testabspath.txt")));

        std::string cwd_up = Path::getPathFromFilename(cwd);
        cwd_up.pop_back(); // remove trailing slash
        ASSERT_EQUALS(cwd_up, Path::getAbsoluteFilePath(Path::join(cwd, "..")));
        ASSERT_EQUALS(cwd_up, Path::getAbsoluteFilePath(Path::join(cwd, "../")));
        ASSERT_EQUALS(cwd_up, Path::getAbsoluteFilePath(Path::join(cwd, "..\\")));
        ASSERT_EQUALS(cwd_up, Path::getAbsoluteFilePath(Path::join(cwd, "./../")));
        ASSERT_EQUALS(cwd_up, Path::getAbsoluteFilePath(Path::join(cwd, ".\\..\\")));

        ASSERT_EQUALS(cwd, Path::getAbsoluteFilePath("."));
#ifndef _WIN32
        TODO_ASSERT_EQUALS(cwd, "", Path::getAbsoluteFilePath("./"));
        TODO_ASSERT_EQUALS(cwd, "", Path::getAbsoluteFilePath(".\\"));
#else
        ASSERT_EQUALS(cwd, Path::getAbsoluteFilePath("./"));
        ASSERT_EQUALS(cwd, Path::getAbsoluteFilePath(".\\"));
#endif

        ASSERT_EQUALS("", Path::getAbsoluteFilePath(""));

#ifndef _WIN32
        // the underlying realpath() call only returns something if the path actually exists
        ASSERT_THROW_EQUALS(Path::getAbsoluteFilePath("testabspath2.txt"), std::runtime_error, "path 'testabspath2.txt' does not exist");
#else
        ASSERT_EQUALS(Path::toNativeSeparators(Path::join(cwd, "testabspath2.txt")), Path::getAbsoluteFilePath("testabspath2.txt"));
#endif

#ifdef _WIN32
        // determine an existing drive letter
        std::string drive = Path::getCurrentPath().substr(0, 2);
        ASSERT_EQUALS(drive + "", Path::getAbsoluteFilePath(drive + "\\"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "\\path"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "\\path\\"));
        ASSERT_EQUALS(drive + "\\path\\files.txt", Path::getAbsoluteFilePath(drive + "\\path\\files.txt"));
        ASSERT_EQUALS(drive + "", Path::getAbsoluteFilePath(drive + "//"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "//path"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "//path/"));
        ASSERT_EQUALS(drive + "\\path\\files.txt", Path::getAbsoluteFilePath(drive + "//path/files.txt"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "\\\\path"));
        ASSERT_EQUALS(drive + "", Path::getAbsoluteFilePath(drive + "/"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "/path"));

        drive[0] = static_cast<char>(toupper(drive[0]));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "\\path"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "/path"));

        drive[0] = static_cast<char>(tolower(drive[0]));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "\\path"));
        ASSERT_EQUALS(drive + "\\path", Path::getAbsoluteFilePath(drive + "/path"));

        ASSERT_EQUALS("1:\\path\\files.txt", Path::getAbsoluteFilePath("1:\\path\\files.txt")); // treated as valid drive
        ASSERT_EQUALS(
            Path::toNativeSeparators(Path::join(Path::getCurrentPath(), "CC:\\path\\files.txt")),
            Path::getAbsoluteFilePath("CC:\\path\\files.txt")); // treated as filename
        ASSERT_EQUALS("1:\\path\\files.txt", Path::getAbsoluteFilePath("1:/path/files.txt")); // treated as valid drive
        ASSERT_EQUALS(
            Path::toNativeSeparators(Path::join(Path::getCurrentPath(), "CC:\\path\\files.txt")),
            Path::getAbsoluteFilePath("CC:/path/files.txt")); // treated as filename
#endif

#ifndef _WIN32
        ASSERT_THROW_EQUALS(Path::getAbsoluteFilePath("C:\\path\\files.txt"), std::runtime_error, "path 'C:\\path\\files.txt' does not exist");
#endif

        // TODO: test UNC paths
        // TODO: test with symlinks
    }

    void exists() const {
        ScopedFile file("testpath.txt", "", "testpath");
        ScopedFile file2("testpath2.txt", "");

        ASSERT_EQUALS(true, Path::exists("testpath"));
        ASSERT_EQUALS(true, Path::exists("testpath2.txt"));

        ASSERT_EQUALS(false, Path::exists("testpath2"));

        bool b = false;

        ASSERT_EQUALS(true, Path::exists("testpath", &b));
        ASSERT_EQUALS(true, b);
        ASSERT_EQUALS(true, Path::exists("testpath/testpath.txt", &b));
        ASSERT_EQUALS(false, b);
        ASSERT_EQUALS(true, Path::exists("testpath2.txt", &b));
        ASSERT_EQUALS(false, b);

        ASSERT_EQUALS(false, Path::exists("testpath2", &b));
        ASSERT_EQUALS(false, b);
        ASSERT_EQUALS(false, Path::exists("testpath/testpath2.txt", &b));
        ASSERT_EQUALS(false, b);
        ASSERT_EQUALS(false, Path::exists("testpath.txt", &b));
        ASSERT_EQUALS(false, b);
    }

    void fromNativeSeparators() const {
        ASSERT_EQUALS("lib/file.c", Path::fromNativeSeparators("lib/file.c"));
        ASSERT_EQUALS("lib//file.c", Path::fromNativeSeparators("lib//file.c"));
        ASSERT_EQUALS("/lib/file.c", Path::fromNativeSeparators("/lib/file.c"));
        ASSERT_EQUALS("//lib/file.c", Path::fromNativeSeparators("//lib/file.c"));
        ASSERT_EQUALS("./lib/file.c", Path::fromNativeSeparators("./lib/file.c"));

        ASSERT_EQUALS("lib/file.c", Path::fromNativeSeparators("lib\\file.c"));
        ASSERT_EQUALS("lib//file.c", Path::fromNativeSeparators("lib\\\\file.c"));
        ASSERT_EQUALS("/lib/file.c", Path::fromNativeSeparators("\\lib\\file.c"));
        ASSERT_EQUALS("//lib/file.c", Path::fromNativeSeparators("\\\\lib\\file.c"));
        ASSERT_EQUALS("./lib/file.c", Path::fromNativeSeparators(".\\lib\\file.c"));
    }

    void isRelative() const {
        ASSERT_EQUALS(true, Path::isRelative("dir/file"));
        ASSERT_EQUALS(true, Path::isRelative("dir\\file"));

        // TODO: is this expected?
        ASSERT_EQUALS(true, Path::isRelative("file/"));
        ASSERT_EQUALS(true, Path::isRelative("file\\"));

        ASSERT_EQUALS(false, Path::isRelative("file"));

#ifdef _WIN32
        // this is a relative path on Windows
        ASSERT_EQUALS(true, Path::isRelative("/dir/file"));
#else
        ASSERT_EQUALS(false, Path::isRelative("/dir/file"));
#endif

#ifdef _WIN32
        // TODO: this is not detected as absolute path in _WIN32 builds
        ASSERT_EQUALS(false, Path::isRelative("c:\\dir\\file"));
        ASSERT_EQUALS(false, Path::isRelative("c:/dir/file"));
#else
        ASSERT_EQUALS(true, Path::isRelative("c:\\dir\\file"));
        ASSERT_EQUALS(true, Path::isRelative("c:/dir/file"));
#endif
    }
};

REGISTER_TEST(TestPath)
