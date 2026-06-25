/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "filelister.h"
#include "filesettings.h"
#include "fixture.h"
#include "path.h"
#include "pathmatch.h"
#include "standards.h"

#include <algorithm>
#include <list>
#include <stdexcept>
#include <string>

class TestFileLister : public TestFixture {
public:
    TestFileLister() : TestFixture("TestFileLister") {}

private:
    void run() override {
        TEST_CASE(recursiveAddFiles);
        TEST_CASE(recursiveAddFilesEmptyPath);
        TEST_CASE(excludeFile1);
        TEST_CASE(excludeFile2);
        TEST_CASE(excludeDir);
        TEST_CASE(addFiles);
    }

    // TODO: generate file list instead
    static std::string findBaseDir() {
        std::string basedir;
        while (!Path::isDirectory(Path::join(basedir, ".github"))) {
            const std::string abspath = Path::getAbsoluteFilePath(basedir);
            basedir += "../";
            // no more going up
            if (Path::getAbsoluteFilePath(basedir) == abspath)
                throw std::runtime_error("could not find repository root directory");
        }
        return basedir;
    }

    void recursiveAddFiles() const {
        const std::string adddir = findBaseDir() + ".";

        // Recursively add add files..
        std::list<FileWithDetails> files;
        std::string err = FileLister::recursiveAddFiles(files, adddir, {}, PathMatch());
        ASSERT_EQUALS("", err);

        ASSERT(!files.empty());

#ifdef _WIN32
        std::string dirprefix;
        if (adddir != ".")
            dirprefix = adddir + "/";
#else
        const std::string dirprefix = adddir + "/";
#endif

        const auto find_file = [&](const std::string& name) {
            return std::find_if(files.cbegin(), files.cend(), [&name](const FileWithDetails& entry) {
                return entry.path() == name;
            });
        };

        // Make sure source files are added..
        auto it = find_file(dirprefix + "cli/main.cpp");
        ASSERT(it != files.end());
        ASSERT_EQUALS_ENUM(Standards::Language::CPP, it->lang());

        it = find_file(dirprefix + "lib/token.cpp");
        ASSERT(it != files.end());
        ASSERT_EQUALS_ENUM(Standards::Language::CPP, it->lang());

        it = find_file(dirprefix + "lib/tokenize.cpp");
        ASSERT(it != files.end());
        ASSERT_EQUALS_ENUM(Standards::Language::CPP, it->lang());

        it = find_file(dirprefix + "gui/main.cpp");
        ASSERT(it != files.end());
        ASSERT_EQUALS_ENUM(Standards::Language::CPP, it->lang());

        it = find_file(dirprefix + "test/testfilelister.cpp");
        ASSERT(it != files.end());
        ASSERT_EQUALS_ENUM(Standards::Language::CPP, it->lang());

        // Make sure headers are not added..
        ASSERT(find_file(dirprefix + "lib/tokenize.h") == files.end());
    }

    void recursiveAddFilesEmptyPath() const {
        std::list<FileWithDetails> files;
        const std::string err = FileLister::recursiveAddFiles(files, "", {}, PathMatch());
        ASSERT_EQUALS("no path specified", err);
    }

    void excludeFile1() const {
        const std::string basedir = findBaseDir();

        std::list<FileWithDetails> files;
        PathMatch matcher({"lib/token.cpp"});
        std::string err = FileLister::recursiveAddFiles(files, basedir + "lib/token.cpp", {}, matcher);
        ASSERT_EQUALS("", err);
        ASSERT(files.empty());
    }

    void excludeFile2() const {
        const std::string basedir = findBaseDir();

        std::list<FileWithDetails> files;
        std::string err = FileLister::recursiveAddFiles(files, basedir + "lib/token.cpp", {}, PathMatch());
        ASSERT_EQUALS("", err);
        ASSERT_EQUALS(1, files.size());
        ASSERT_EQUALS(basedir + "lib/token.cpp", files.begin()->path());
    }

    void excludeDir() const {
        const std::string basedir = findBaseDir() + ".";

        std::list<FileWithDetails> files;
        PathMatch matcher({"lib/"});
        std::string err = FileLister::recursiveAddFiles(files, basedir, {}, matcher);
        ASSERT_EQUALS("", err);
        ASSERT(!files.empty());
        const auto it = std::find_if(files.cbegin(), files.cend(), [](const FileWithDetails& f){
            return f.spath().find("/lib/") != std::string::npos;
        });
        ASSERT(it == files.cend());
    }

    void addFiles() const {
        const std::string adddir = findBaseDir() + ".";

        // TODO: on Windows the prefix is different from when a recursive a folder (see recursiveAddFiles test)
        const std::string dirprefix = adddir + "/";
#ifdef _WIN32
        const std::string dirprefix_nat = Path::toNativeSeparators(dirprefix);
#endif

        std::list<FileWithDetails> files;

        {
            const std::string addfile = Path::join(Path::join(adddir, "cli"), "main.cpp");
            const std::string err = FileLister::addFiles(files, addfile, {}, true,PathMatch());
            ASSERT_EQUALS("", err);
        }
        {
            const std::string addfile = Path::join(Path::join(adddir, "lib"), "token.cpp");
            const std::string err = FileLister::addFiles(files, addfile, {}, true,PathMatch());
            ASSERT_EQUALS("", err);
        }
        {
            const std::string addfile = Path::join(Path::join(adddir, "cli"), "token.cpp"); // does not exist
            const std::string err = FileLister::addFiles(files, addfile, {}, true,PathMatch());
            ASSERT_EQUALS("", err);
        }
        {
            const std::string addfile = Path::join(Path::join(adddir, "lib2"), "token.cpp"); // does not exist
            const std::string err = FileLister::addFiles(files, addfile, {}, true,PathMatch());
            ASSERT_EQUALS("", err);
        }
        {
            const std::string addfile = Path::join(Path::join(adddir, "lib"), "matchcompiler.h");
            const std::string err = FileLister::addFiles(files, addfile, {}, true,PathMatch());
            ASSERT_EQUALS("", err);
        }

        ASSERT_EQUALS(3, files.size());
        auto it = files.cbegin();
        ASSERT_EQUALS(dirprefix + "cli/main.cpp", it->path());
        ASSERT_EQUALS(Path::simplifyPath(dirprefix + "cli/main.cpp"), it->spath());
        ASSERT_EQUALS_ENUM(Standards::Language::None, it->lang());
        it++;
        ASSERT_EQUALS(dirprefix + "lib/token.cpp", it->path());
        ASSERT_EQUALS(Path::simplifyPath(dirprefix + "lib/token.cpp"), it->spath());
        ASSERT_EQUALS_ENUM(Standards::Language::None, it->lang());
        it++;
        ASSERT_EQUALS(dirprefix + "lib/matchcompiler.h", it->path());
        ASSERT_EQUALS(Path::simplifyPath(dirprefix + "lib/matchcompiler.h"), it->spath());
        ASSERT_EQUALS_ENUM(Standards::Language::None, it->lang());
    }

    // TODO: test errors
    // TODO: test wildcards
};

REGISTER_TEST(TestFileLister)
