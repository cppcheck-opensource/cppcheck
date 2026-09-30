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

#include "pathmatch.h"
#include "fixture.h"

#include <string>
#include <vector>


class TestPathMatch : public TestFixture {
public:
    TestPathMatch() : TestFixture("TestPathMatch") {}

private:
    class PathMatchTest final : public PathMatch
    {
        friend class TestPathMatch;
    };

    static constexpr auto unix = PathMatch::Syntax::unix;
    static constexpr auto windows = PathMatch::Syntax::windows;
    static constexpr auto ifreg = PathMatch::Filemode::regular;
    static constexpr auto ifdir = PathMatch::Filemode::directory;
#ifdef _WIN32
    const std::string basepath{"C:\\test"};
#else
    const std::string basepath{"/test"};
#endif
    PathMatch emptyMatcher{{}, basepath};
    PathMatch srcMatcher{{"src/"}, basepath};
    PathMatch fooCppMatcher{{"foo.cpp"}, basepath};
    PathMatch srcFooCppMatcher{{"src/foo.cpp"}, basepath};

    void run() override {
        TEST_CASE(emptymaskemptyfile);
        TEST_CASE(emptymaskpath1);
        TEST_CASE(emptymaskpath2);
        TEST_CASE(emptymaskpath3);
        TEST_CASE(onemaskemptypath);
        TEST_CASE(onemasksamepath);
        TEST_CASE(onemasksamepathdifferentslash);
        TEST_CASE(onemasksamepathdifferentcase);
        TEST_CASE(onemasksamepathwithfile);
        TEST_CASE(onemaskshorterpath);
        TEST_CASE(onemaskdifferentdir1);
        TEST_CASE(onemaskdifferentdir2);
        TEST_CASE(onemaskdifferentdir3);
        TEST_CASE(onemaskdifferentdir4);
        TEST_CASE(onemasklongerpath1);
        TEST_CASE(onemasklongerpath2);
        TEST_CASE(onemasklongerpath3);
        TEST_CASE(onemaskcwd);
        TEST_CASE(twomasklongerpath1);
        TEST_CASE(twomasklongerpath2);
        TEST_CASE(twomasklongerpath3);
        TEST_CASE(twomasklongerpath4);
        TEST_CASE(filemask1);
        TEST_CASE(filemaskdifferentcase);
        TEST_CASE(filemask2);
        TEST_CASE(filemask3);
        TEST_CASE(filemaskcwd);
        TEST_CASE(filemaskpath1);
        TEST_CASE(filemaskpath2);
        TEST_CASE(filemaskpath3);
        TEST_CASE(filemaskpath4);
        TEST_CASE(mixedallmatch);
        TEST_CASE(glob1);
        TEST_CASE(glob2);
        TEST_CASE(globstar1);
        TEST_CASE(globstar2);
        TEST_CASE(pathiterator);
        TEST_CASE(unmatched1);
        TEST_CASE(unmatched2);
        TEST_CASE(unmatched3);
        TEST_CASE(unmatched4);
        TEST_CASE(unmatched5);
        TEST_CASE(unmatched6);
    }

    // Test empty PathMatch
    void emptymaskemptyfile() {
        ASSERT(!emptyMatcher.match(""));
    }

    void emptymaskpath1() {
        ASSERT(!emptyMatcher.match("src/", ifdir));
    }

    void emptymaskpath2() {
        ASSERT(!emptyMatcher.match("../src/", ifdir));
    }

    void emptymaskpath3() {
        ASSERT(!emptyMatcher.match("/home/user/code/src/", ifdir));
        ASSERT(!emptyMatcher.match("d:/home/user/code/src/", ifdir));
    }

    // Test PathMatch containing "src/"
    void onemaskemptypath() {
        ASSERT(!srcMatcher.match(""));
    }

    void onemasksamepath() {
        ASSERT(srcMatcher.match("src/", ifdir));
        ASSERT(!srcMatcher.match("src/", ifreg));
    }

    void onemasksamepathdifferentslash() const {
        PathMatch srcMatcher2({"src\\"}, basepath, windows);
        ASSERT(srcMatcher2.match("src/", ifdir));
        ASSERT(!srcMatcher2.match("src/", ifreg));
    }

    void onemasksamepathdifferentcase() const {
        PathMatch match({"sRc/"}, basepath, windows);
        ASSERT(match.match("srC/", ifdir));
        ASSERT(!match.match("srC/", ifreg));
    }

    void onemasksamepathwithfile() {
        ASSERT(srcMatcher.match("src/file.txt"));
    }

    void onemaskshorterpath() const {
        const std::string longerExclude("longersrc/");
        const std::string shorterToMatch("src/");
        ASSERT(shorterToMatch.length() < longerExclude.length());
        PathMatch match({longerExclude});
        ASSERT(match.match(longerExclude, ifdir));
        ASSERT(!match.match(shorterToMatch, ifdir));
    }

    void onemaskdifferentdir1() {
        ASSERT(!srcMatcher.match("srcfiles/file.txt"));
    }

    void onemaskdifferentdir2() {
        ASSERT(!srcMatcher.match("proj/srcfiles/file.txt"));
    }

    void onemaskdifferentdir3() {
        ASSERT(!srcMatcher.match("proj/mysrc/file.txt"));
    }

    void onemaskdifferentdir4() {
        ASSERT(!srcMatcher.match("proj/mysrcfiles/file.txt"));
    }

    void onemasklongerpath1() {
        ASSERT(srcMatcher.match("/tmp/src/", ifdir));
        ASSERT(srcMatcher.match("d:/tmp/src/", ifdir));
    }

    void onemasklongerpath2() {
        ASSERT(srcMatcher.match("src/module/", ifdir));
    }

    void onemasklongerpath3() {
        ASSERT(srcMatcher.match("project/src/module/", ifdir));
    }

    void onemaskcwd() {
        ASSERT(srcMatcher.match("./src", ifdir));
    }

    void twomasklongerpath1() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(!match.match("project/", ifdir));
    }

    void twomasklongerpath2() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(match.match("project/src/", ifdir));
        ASSERT(!match.match("project/src/", ifreg));
    }

    void twomasklongerpath3() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(match.match("project/module/", ifdir));
        ASSERT(!match.match("project/module/", ifreg));
    }

    void twomasklongerpath4() const {
        PathMatch match({ "src/", "module/" });
        ASSERT(match.match("project/src/module/", ifdir));
        ASSERT(match.match("project/src/module/", ifreg));
    }

    // Test PathMatch containing "foo.cpp"
    void filemask1() {
        ASSERT(fooCppMatcher.match("foo.cpp"));
    }

    void filemaskdifferentcase() const {
        PathMatch match({"foo.cPp"}, basepath, windows);
        ASSERT(match.match("fOo.cpp"));
    }

    void filemask2() {
        ASSERT(fooCppMatcher.match("../foo.cpp"));
    }

    void filemask3() {
        ASSERT(fooCppMatcher.match("src/foo.cpp"));
    }

    void filemaskcwd() {
        ASSERT(fooCppMatcher.match("./lib/foo.cpp"));
    }

    // Test PathMatch containing "src/foo.cpp"
    void filemaskpath1() {
        ASSERT(srcFooCppMatcher.match("src/foo.cpp"));
    }

    void filemaskpath2() {
        ASSERT(srcFooCppMatcher.match("proj/src/foo.cpp"));
    }

    void filemaskpath3() {
        ASSERT(!srcFooCppMatcher.match("foo.cpp"));
    }

    void filemaskpath4() {
        ASSERT(!srcFooCppMatcher.match("bar/foo.cpp"));
    }

    void mixedallmatch() const { // #13570
        // when trying to match a directory against a directory entry it erroneously modified a local variable also used for file matching
        PathMatch match({ "tests/", "file.c" });
        ASSERT(match.match("tests/", ifdir));
        ASSERT(!match.match("tests/", ifreg));
        ASSERT(match.match("lib/file.c"));
    }

    void glob1() const {
        PathMatch match({"test?.cpp"});
        ASSERT(match.match("test1.cpp"));
        ASSERT(match.match("src/test1.cpp"));
        ASSERT(match.match("test1.cpp/src"));
        ASSERT(!match.match("test1.c"));
        ASSERT(!match.match("test.cpp"));
    }

    void glob2() const {
        PathMatch match({"test*.cpp"});
        ASSERT(match.match("test1.cpp"));
        ASSERT(match.match("src/test1.cpp"));
        ASSERT(match.match("test1.cpp/src"));
        ASSERT(!match.match("test1.c"));
        ASSERT(match.match("test.cpp"));
    }

    void globstar1() const {
        PathMatch match({"src/**/foo.c"});
        ASSERT(match.match("src/lib/foo/foo.c"));
        ASSERT(match.match("src/lib/foo/bar/foo.c"));
        ASSERT(!match.match("src/lib/foo/foo.cpp"));
        ASSERT(!match.match("src/lib/foo/bar/foo.cpp"));
        ASSERT(!match.match("src/foo.c"));
    }

    void globstar2() const {
        PathMatch match({"./src/**/foo.c"});
        ASSERT(match.match("src/lib/foo/foo.c"));
        ASSERT(match.match("src/lib/foo/bar/foo.c"));
        ASSERT(!match.match("src/lib/foo/foo.cpp"));
        ASSERT(!match.match("src/lib/foo/bar/foo.cpp"));
        ASSERT(!match.match("src/foo.c"));
    }

    void pathiterator() const {
        /* See https://learn.microsoft.com/en-us/dotnet/standard/io/file-path-formats
         * for information on Windows path syntax. */
        using PathIterator = PathMatchTest::PathIterator;
        ASSERT_EQUALS("/", PathIterator("/", nullptr, unix).read());
        ASSERT_EQUALS("/", PathIterator("//", nullptr, unix).read());
        ASSERT_EQUALS("/", PathIterator("/", "/", unix).read());
        ASSERT_EQUALS("/hello/world", PathIterator("/hello/universe/.", "../world//", unix).read());
        ASSERT_EQUALS("/", PathIterator("//./..//.///.", "../../..///", unix).read());
        ASSERT_EQUALS("/foo/bar", PathIterator(nullptr, "/foo/bar/.", unix).read());
        ASSERT_EQUALS("/foo/bar", PathIterator("/foo/bar/.", nullptr, unix).read());
        ASSERT_EQUALS("/foo/bar", PathIterator("/foo", "bar", unix).read());
        ASSERT_EQUALS("", PathIterator("", "", unix).read());
        ASSERT_EQUALS("", PathIterator("", nullptr, unix).read());
        ASSERT_EQUALS("", PathIterator(nullptr, "", unix).read());
        ASSERT_EQUALS("", PathIterator(nullptr, nullptr, unix).read());
        ASSERT_EQUALS("c:", PathIterator("C:", nullptr, windows).read());
        /* C: without slash is a bit ambiguous. It should probably not be considered a root because it's
         * not fully qualified (it designates the current directory on the C drive),
         * so this test could be considered to be unspecified behavior. */
        ASSERT_EQUALS("c:", PathIterator("C:", "../..", windows).read());
        ASSERT_EQUALS("c:/windows/system32", PathIterator("C:", "Windows\\System32\\Drivers\\..\\.", windows).read());
        ASSERT_EQUALS("c:/", PathIterator("C:\\Program Files\\", "..", windows).read());
        ASSERT_EQUALS("//./", PathIterator("\\\\.\\C:\\", "../..", windows).read());
        ASSERT_EQUALS("//./", PathIterator("\\\\.\\", "..\\..", windows).read());
        ASSERT_EQUALS("//?/", PathIterator("\\\\?\\", "..\\..", windows).read());
        /* The server and share should actually be considered part of the root and not be removed */
        ASSERT_EQUALS("//", PathIterator("\\\\Server\\Share\\Directory", "../..\\../..", windows).read());
    }

    void unmatched1() const {
        // nothing has been matched yet => all patterns are unmatched
        PathMatch match({"src/", "foo.cpp"});
        ASSERT_EQUALS(2, match.unmatched().size());
        ASSERT_EQUALS("src/", match.unmatched()[0]);
        ASSERT_EQUALS("foo.cpp", match.unmatched()[1]);
    }

    void unmatched2() const {
        PathMatch match({"src/", "foo.cpp"});
        ASSERT(match.match("src/bar.cpp"));
        ASSERT_EQUALS(1, match.unmatched().size());
        ASSERT_EQUALS("foo.cpp", match.unmatched()[0]);
        ASSERT(match.match("lib/foo.cpp"));
        ASSERT_EQUALS(0, match.unmatched().size());
    }

    void unmatched3() const {
        // failed matches does not affect the unmatched patterns
        PathMatch match({"src/", "foo.cpp"});
        ASSERT(!match.match("lib/bar.cpp"));
        ASSERT(!match.match("src/", ifreg));
        ASSERT_EQUALS(2, match.unmatched().size());
        ASSERT(match.match("src/", ifdir));
        ASSERT_EQUALS(1, match.unmatched().size());
        ASSERT_EQUALS("foo.cpp", match.unmatched()[0]);
    }

    void unmatched4() const {
        // glob patterns
        PathMatch match({"test*.cpp", "src/**/foo.c"});
        ASSERT(match.match("src/lib/foo/foo.c"));
        ASSERT_EQUALS(1, match.unmatched().size());
        ASSERT_EQUALS("test*.cpp", match.unmatched()[0]);
        ASSERT(!match.match("src/foo.c"));
        ASSERT(match.match("test1.cpp"));
        ASSERT_EQUALS(0, match.unmatched().size());
    }

    void unmatched5() const {
        // all matching patterns are recorded
        PathMatch match({"src/", "src/foo.cpp"});
        ASSERT(match.match("src/foo.cpp"));
        ASSERT_EQUALS(0, match.unmatched().size());
    }

    void unmatched6() const {
        // a pattern inside a matched directory is considered used
        PathMatch match({"src/", "src/foo.cpp", "src/lib/", "lib/foo.cpp"});
        ASSERT(match.match("src/", ifdir));
        ASSERT_EQUALS(1, match.unmatched().size());
        ASSERT_EQUALS("lib/foo.cpp", match.unmatched()[0]);
    }
};

REGISTER_TEST(TestPathMatch)
