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

#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "importproject.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class TestImporter final : public ImportProject {
public:
    using ImportProject::processCompileCommands;
    using ImportProject::importCppcheckGuiProject;
    using ImportProject::collectArgs;
    using ImportProject::fsSetDefines;
    using ImportProject::fsSetIncludePaths;
    using ImportProject::setRelativePaths;
    using ImportProject::setProjectPath;
};


class TestImportProject : public TestFixture {
public:
    TestImportProject() : TestFixture("TestImportProject") {}

private:

    void run() override {
        TEST_CASE(setDefines);
        TEST_CASE(setIncludePaths1);
        TEST_CASE(setIncludePaths2);
        TEST_CASE(setIncludePaths3); // macro names are case insensitive
        TEST_CASE(setRelativePathsInclude); // #14746
        TEST_CASE(processCompileCommands1);
        TEST_CASE(processCompileCommands2); // #8563, #9567
        TEST_CASE(processCompileCommands3); // check with existing trailing / in directory
        TEST_CASE(processCompileCommands4); // only accept certain file types
        TEST_CASE(processCompileCommands5); // Windows/CMake/Ninja generated compile_commands.json
        TEST_CASE(processCompileCommands6); // Windows/CMake/Ninja generated compile_commands.json with spaces
        TEST_CASE(processCompileCommands7); // linux: "/home/danielm/cppcheck 2"
        TEST_CASE(processCompileCommands8); // Windows: "C:\Users\danielm\cppcheck"
        TEST_CASE(processCompileCommands9);
        TEST_CASE(processCompileCommands10); // #10887: include path with space
        TEST_CASE(processCompileCommands11); // include path order
        TEST_CASE(processCompileCommands12); // #13040: "directory" is parent directory, relative include paths
        TEST_CASE(processCompileCommands13); // #13333: duplicate file entries
        TEST_CASE(processCompileCommands14); // #14156
        TEST_CASE(processCompileCommands15); // #14306
        TEST_CASE(processCompileCommandsForcedInclude); // -include / /FI force-include
        TEST_CASE(processCompileCommandsArgumentsSection); // Handle arguments section
        TEST_CASE(processCompileCommandsNoCommandSection); // gracefully handles malformed json
        TEST_CASE(processCompileCommandsDirectoryMissing); // 'directory' field missing
        TEST_CASE(processCompileCommandsDirectoryInvalid); // 'directory' field not a string
        TEST_CASE(importCppcheckGuiProject);
        TEST_CASE(importCppcheckGuiProjectDuplicateSuppressions);
        TEST_CASE(importCppcheckGuiProjectPremiumMisra);
        TEST_CASE(importCppcheckGuiProjectAbsPath);    // absolute Unix path must not be prepended with mPath
        TEST_CASE(importCppcheckGuiProjectRelPathWithBase); // relative path must be prepended with mPath
        TEST_CASE(ignorePaths);
        TEST_CASE(testCollectArgs1);
        TEST_CASE(testCollectArgs2);
        TEST_CASE(testCollectArgs3);
        TEST_CASE(testCollectArgs4);
        TEST_CASE(testCollectArgs5);
        TEST_CASE(testCollectArgs6);
        TEST_CASE(testCollectArgs7);
        TEST_CASE(testVcxprojConditions);
        TEST_CASE(testVcxprojConditionEqualityIsNotVersionComparison);
        TEST_CASE(testPropertyNameAllowsHyphen);
        TEST_CASE(testMSBuildStaticFunctions);
        TEST_CASE(testVcxitemsPathResolution);
        TEST_CASE(testMissingChildImportNonFatal); // missing child imports must not abort the project
        TEST_CASE(testCurrentToolsVersionFromProps); // "Current" keyword uses VisualStudioVersion property
        TEST_CASE(testMetadataSelfReferenceCaseInsensitive);
    }

    void setDefines() const {
        FileSettings fs{"test.cpp", Standards::Language::CPP, 0};

        TestImporter::fsSetDefines(fs, "A");
        ASSERT_EQUALS("A=1", fs.defines);

        TestImporter::fsSetDefines(fs, "A;B;");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        TestImporter::fsSetDefines(fs, "A;;B;");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        TestImporter::fsSetDefines(fs, "A;;B");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        // M-2: %(metadata) tokens at the start of the string must be stripped
        TestImporter::fsSetDefines(fs, "%(PreprocessorDefinitions)");
        ASSERT_EQUALS("", fs.defines);

        TestImporter::fsSetDefines(fs, "%(PreprocessorDefinitions);A");
        ASSERT_EQUALS("A=1", fs.defines);

        TestImporter::fsSetDefines(fs, "%(PreprocessorDefinitions);A;B");
        ASSERT_EQUALS("A=1;B=1", fs.defines);

        // Leading %(…) followed by a mid-string %(…) — both must be stripped
        TestImporter::fsSetDefines(fs, "%(PreprocessorDefinitions);A;%(OtherMeta);B");
        ASSERT_EQUALS("A=1;B=1", fs.defines);
    }

    void setIncludePaths1() const {
        FileSettings fs{"test.cpp", Standards::Language::CPP, 0};
        std::list<std::string> in(1, "../include");
        PropertiesMap properties;
        TestImporter importer;
        importer.fsSetIncludePaths(fs, "abc/def/", in, properties);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("abc/include/", fs.includePaths.front());
    }

    void setIncludePaths2() const {
        FileSettings fs{"test.cpp", Standards::Language::CPP, 0};
        std::list<std::string> in(1, "$(SolutionDir)other");
        PropertiesMap properties;
        properties["SolutionDir"] = "c:/abc/";
        TestImporter importer;
        importer.fsSetIncludePaths(fs, "/home/fred", in, properties);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void setIncludePaths3() const { // macro names are case insensitive
        FileSettings fs{"test.cpp", Standards::Language::CPP, 0};
        std::list<std::string> in(1, "$(SOLUTIONDIR)other");
        PropertiesMap properties;
        properties["SolutionDir"] = "c:/abc/";
        TestImporter importer;
        importer.fsSetIncludePaths(fs, "/home/fred", in, properties);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void setRelativePathsInclude() const {
        const std::string cwd = Path::fromNativeSeparators(Path::getCurrentPath());
        TestImporter importer;
        FileSettings fs{cwd + "/sub/a.c", Standards::Language::C, 0};
        fs.includePaths.push_back(cwd + "/");
        importer.fileSettings.push_back(fs);
        importer.setRelativePaths("compile_commands.json");
        fs = importer.fileSettings.front();
        ASSERT_EQUALS(".", fs.includePaths.front());
        ASSERT_EQUALS("sub/a.c", fs.filename());
    }

    void processCompileCommands1() const {
        REDIRECT;
        constexpr char json[] = R"([{
                                   "directory": "/tmp",
                                   "command": "gcc -DTEST1 -DTEST2=2 -o /tmp/src.o -c /tmp/src.c",
                                   "file": "/tmp/src.c"
                                   }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("TEST1=1;TEST2=2", importer.fileSettings.cbegin()->defines);
    }

    void processCompileCommands2() const {
        REDIRECT;
        // Absolute file path
#ifdef _WIN32
        const char json[] = R"([{
                                   "directory": "C:/foo",
                                   "command": "gcc -c /bar.c",
                                   "file": "/bar.c"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("C:/bar.c", importer.fileSettings.cbegin()->filename());
#else
        constexpr char json[] = R"([{
                                   "directory": "/foo",
                                   "command": "gcc -c bar.c",
                                   "file": "/bar.c"
                                   }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/bar.c", importer.fileSettings.cbegin()->filename());
#endif
    }

    void processCompileCommands3() const {
        REDIRECT;
        const char json[] = R"([{
                                    "directory": "/tmp/",
                                    "command": "gcc -c src.c",
                                    "file": "src.c"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.cbegin()->filename());
    }

    void processCompileCommands4() const {
        REDIRECT;
        constexpr char json[] = R"([{
                                    "directory": "/tmp/",
                                    "command": "gcc -c src.mm",
                                    "file": "src.mm"
                                   }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
    }

    void processCompileCommands5() const {
        REDIRECT;
        constexpr char json[] =
            R"([{
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomylib\\CMakeFiles\\mylib.dir\\src\\foobar\\mylib.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp"
             },
             {
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\myapp\\src -Imyapp -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomyapp\\CMakeFiles\\myapp.dir\\src\\main.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp"
             }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/src/", importer.fileSettings.cbegin()->includePaths.front());
    }

    void processCompileCommands6() const {
        REDIRECT;
        constexpr char json[] =
            R"([{
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src -I\"C:\\Users\\dan\\git\\test-cppcheck\\mylib\\second src\" /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomylib\\CMakeFiles\\mylib.dir\\src\\foobar\\mylib.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\mylib\\src\\foobar\\mylib.cpp"
            },
            {
                "directory": "C:/Users/dan/git/build-test-cppcheck-Desktop_Qt_5_15_0_MSVC2019_64bit-Debug",
                "command": "C:\\PROGRA~2\\MICROS~1\\2019\\COMMUN~1\\VC\\Tools\\MSVC\\1427~1.291\\bin\\HostX64\\x64\\cl.exe /nologo /TP -IC:\\Users\\dan\\git\\test-cppcheck\\myapp\\src -Imyapp -IC:\\Users\\dan\\git\\test-cppcheck\\mylib\\src /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -MDd -std:c++17 /Fomyapp\\CMakeFiles\\myapp.dir\\src\\main.cpp.obj /FdTARGET_COMPILE_PDB /FS -c C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp",
                "file": "C:\\Users\\dan\\git\\test-cppcheck\\myapp\\src\\main.cpp"
             }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/src/", importer.fileSettings.cbegin()->includePaths.front());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/second src/", importer.fileSettings.cbegin()->includePaths.back());
    }


    void processCompileCommands7() const {
        REDIRECT;
        // cmake -DFILESDIR="/some/path" ..
        constexpr char json[] =
            R"([{
                "directory": "/home/danielm/cppcheck 2/b/lib",
                "command": "/usr/bin/c++  -DFILESDIR=\\\"/some/path\\\" -I\"/home/danielm/cppcheck 2/b/lib\" -isystem \"/home/danielm/cppcheck 2/externals\" \"/home/danielm/cppcheck 2/lib/astutils.cpp\"",
                "file": "/home/danielm/cppcheck 2/lib/astutils.cpp"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("FILESDIR=\"/some/path\"", importer.fileSettings.cbegin()->defines);
        ASSERT_EQUALS(1, importer.fileSettings.cbegin()->includePaths.size());
        ASSERT_EQUALS("/home/danielm/cppcheck 2/b/lib/", importer.fileSettings.cbegin()->includePaths.front());
        TODO_ASSERT_EQUALS("/home/danielm/cppcheck 2/externals/",
                           "/home/danielm/cppcheck 2/b/lib/",
                           importer.fileSettings.cbegin()->includePaths.back());
    }

    void processCompileCommands8() const {
        REDIRECT;
        // cmake -DFILESDIR="C:\Program Files\Cppcheck" -G"NMake Makefiles" ..
        constexpr char json[] =
            R"([{
              "directory": "C:/Users/danielm/cppcheck/build/lib",
              "command": "C:\\PROGRA~2\\MICROS~2\\2017\\COMMUN~1\\VC\\Tools\\MSVC\\1412~1.258\\bin\\Hostx64\\x64\\cl.exe  /nologo /TP -DFILESDIR=\"\\\"C:\\Program Files\\Cppcheck\\\"\" -IC:\\Users\\danielm\\cppcheck\\build\\lib -IC:\\Users\\danielm\\cppcheck\\lib -c C:\\Users\\danielm\\cppcheck\\lib\\astutils.cpp",
              "file": "C:/Users/danielm/cppcheck/lib/astutils.cpp"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr)); // Do not crash
    }

    void processCompileCommands9() const {
        REDIRECT;
        // IAR output (https://sourceforge.net/p/cppcheck/discussion/general/thread/608af51e0a/)
        constexpr char json[] =
            R"([{
              "arguments" : [
                 "powershell.exe -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass -File d:\\Projekte\\xyz\\firmware\\app\\xyz-lib\\build.ps1 -IAR -COMPILER_PATH \"c:\\Program Files (x86)\\IAR Systems\\Embedded Workbench 9.0\" -CONTROLLER CC1310F128 -LIB LIB_PERMANENT -COMPILER_DEFINES \"CC1310_HFXO_FREQ=24000000 DEBUG\""
              ],
              "directory" : "d:\\Projekte\\xyz\\firmware\\app",
              "type" : "PRE",
              "file": "1.c"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
    }

    void processCompileCommands10() const { // #10887
        REDIRECT;
        constexpr char json[] =
            R"([{
               "file": "/home/danielm/cppcheck/1/test folder/1.c" ,
               "directory": "",
               "arguments": [
                   "iccavr.exe",
                   "-I",
                   "/home/danielm/cppcheck/test folder"
               ]
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("/home/danielm/cppcheck/test folder/", fs.includePaths.front());
    }

    void processCompileCommands11() const { // include path order
        REDIRECT;
        constexpr char json[] =
            R"([{
               "file": "1.c" ,
               "directory": "/x",
               "arguments": [
                   "cc",
                   "-I",
                   "def",
                   "-I",
                   "abc"
               ]
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("/x/def/", fs.includePaths.front());
        ASSERT_EQUALS("/x/abc/", fs.includePaths.back());
    }

    void processCompileCommands12() const { // #13040
        REDIRECT;
        constexpr char json[] =
            R"([{
               "file": "/x/src/1.c" ,
               "directory": "/x",
               "command": "cc -c -I. src/1.c"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS(1, fs.includePaths.size());
        ASSERT_EQUALS("/x/", fs.includePaths.front());
    }

    void processCompileCommands13() const { // #13333
        REDIRECT;
        constexpr char json[] =
            R"([{
               "file": "/x/src/1.c" ,
               "directory": "/x",
               "command": "cc -c -I. src/1.c"
            },{
               "file": "/x/src/1.c" ,
               "directory": "/x",
               "command": "cc -c -I. src/1.c"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        const FileSettings &fs1 = importer.fileSettings.front();
        const FileSettings &fs2 = importer.fileSettings.back();
        ASSERT_EQUALS(0, fs1.file.fsFileId());
        ASSERT_EQUALS(1, fs2.file.fsFileId());
    }

    void processCompileCommands14() const { // #14156
        REDIRECT;
        constexpr char json[] =
            R"([{
                "arguments": [
                  "/usr/bin/g++",
                  "-DTFS_LINUX_MODULE_NAME=\"tfs_linux\"",
                  "-g",
                  "-c",
                  "cli/main.cpp"
                ],
                "directory": "/home/daniel/cppcheck",
                "file": "/home/daniel/cppcheck/cli/main.cpp",
                "output": "/home/daniel/cppcheck/cli/main.o"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("TFS_LINUX_MODULE_NAME=\"tfs_linux\"", fs.defines);
    }

    void processCompileCommands15() const { // #14306
        REDIRECT;
        constexpr char json[] =
            R"([
                 {
                   "directory": "C:\\Users\\abcd\\efg\\hijk",
                   "command": "gcc \"-Ipath\\123\" \"-c\" test.c",
                   "file": "test.c",
                   "output": "test.obj"
                 }
               ])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS(1, fs.includePaths.size());
        ASSERT_EQUALS("C:/Users/abcd/efg/hijk/path/123/", fs.includePaths.front());
    }

    void processCompileCommandsForcedInclude() const { // -include / /FI force-include
        REDIRECT;
        constexpr char json[] =
            R"([{
               "file": "/x/a.c",
               "directory": "/x",
               "command": "cc -include prefix.h /FIplatform.h -c a.c"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS(2, fs.forcedIncludes.size());
        ASSERT_EQUALS("prefix.h", fs.forcedIncludes.front()); // gcc/clang -include
        ASSERT_EQUALS("platform.h", fs.forcedIncludes.back()); // MSVC/clang-cl /FI
    }

    void processCompileCommandsArgumentsSection() const {
        REDIRECT;
        constexpr char json[] = "[ { \"directory\": \"/tmp/\","
                                "\"arguments\": [\"gcc\", \"-c\", \"src.c\"],"
                                "\"file\": \"src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.processCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.cbegin()->filename());
    }

    void processCompileCommandsNoCommandSection() const {
        REDIRECT;
        constexpr char json[] = "[ { \"directory\": \"/tmp/\","
                                "\"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(false, importer.processCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
        ASSERT_EQUALS(1, importer.errors.size());
        ASSERT_EQUALS("no 'arguments' or 'command' field found in compilation database entry", importer.errors[0]);
    }

    void processCompileCommandsDirectoryMissing() const {
        REDIRECT;
        constexpr char json[] = "[ { \"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(false, importer.processCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
        ASSERT_EQUALS(1, importer.errors.size());
        ASSERT_EQUALS("'directory' field in compilation database entry missing", importer.errors[0]);
    }

    void processCompileCommandsDirectoryInvalid() const {
        REDIRECT;
        constexpr char json[] = "[ { \"directory\": 123,"
                                "\"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(false, importer.processCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
        ASSERT_EQUALS(1, importer.errors.size());
        ASSERT_EQUALS("'directory' field in compilation database entry is not a string", importer.errors[0]);
    }

    void importCppcheckGuiProject() const {
        REDIRECT;
        constexpr char xml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<project version=\"1\">\n"
                               "    <root name=\".\"/>\n"
                               "    <builddir>out1</builddir>\n"
                               "    <analyze-all-vs-configs>true</analyze-all-vs-configs>\n"
                               "    <includedir>\n"
                               "        <dir name=\"lib/\"/>\n"
                               "    </includedir>\n"
                               "    <paths>\n"
                               "        <dir name=\"cli/\"/>\n"
                               "    </paths>\n"
                               "    <user-include>gcc-macros.h</user-include>\n"
                               "    <exclude>\n"
                               "        <path name=\"gui/temp/\"/>\n"
                               "    </exclude>\n"
                               "    <inline-suppression>true</inline-suppression>\n"
                               "    <project-name>test test</project-name>\n"
                               "</project>\n";
        std::istringstream istr(xml);
        Settings s;
        Suppressions supprs;
        TestImporter project;
        ASSERT_EQUALS(true, project.importCppcheckGuiProject(istr, s, supprs));
        ASSERT_EQUALS(1, project.guiProject.pathNames.size());
        ASSERT_EQUALS("cli/", project.guiProject.pathNames[0]);
        ASSERT_EQUALS(1, s.includePaths.size());
        ASSERT_EQUALS("lib/", s.includePaths.front());
        ASSERT_EQUALS(1, s.userIncludes.size());
        ASSERT_EQUALS("gcc-macros.h", s.userIncludes.front());
        ASSERT_EQUALS(true, s.inlineSuppressions);
    }

    void importCppcheckGuiProjectDuplicateSuppressions() const {
        REDIRECT;
        constexpr char xml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<project version=\"1\">\n"
                               "    <root name=\".\"/>\n"
                               "    <project-name>test test</project-name>\n"
                               "    <suppressions>\n"
                               "        <suppression>uninitvar</suppression>\n"
                               "        <suppression>uninitvar</suppression>\n"
                               "    </suppressions>\n"
                               "</project>\n";
        std::istringstream istr(xml);
        Settings s;
        Suppressions supprs;
        TestImporter project;
        ASSERT_EQUALS(false, project.importCppcheckGuiProject(istr, s, supprs));
        ASSERT_EQUALS(1, project.errors.size());
        ASSERT_EQUALS("suppression 'uninitvar' already exists", project.errors[0]);
    }

    void importCppcheckGuiProjectPremiumMisra() const {
        REDIRECT;
        constexpr char xml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<project>\n"
                               "    <paths>\n"
                               "        <dir name=\"m1.c\"/>\n"
                               "    </paths>\n"
                               "    <addons>\n"
                               "        <addon>misra</addon>\n"  // <- Premium: add premium argument misra-c-2012
                               "    </addons>\n"
                               "</project>";
        std::istringstream istr(xml);
        Settings s;
        s.premium = true;
        Suppressions supprs;
        TestImporter project;
        ASSERT_EQUALS(true, project.importCppcheckGuiProject(istr, s, supprs));
        ASSERT_EQUALS("--misra-c-2012", s.premiumArgs);
        ASSERT(s.addons.empty());
    }

    void importCppcheckGuiProjectAbsPath() const {
        // Regression test: absolute Unix paths in <paths> must not be prepended with the
        // project directory (mPath).  Before the fix, joinRelativePath() classified "/foo"
        // as RootRelative (no drive letter) and prepended mPath, yielding "/proj//foo".
        REDIRECT;
        constexpr char xml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<project version=\"1\">\n"
                               "    <paths>\n"
                               "        <dir name=\"/abs/path/file.cpp\"/>\n"
                               "    </paths>\n"
                               "</project>\n";
        std::istringstream istr(xml);
        Settings s;
        Suppressions supprs;
        TestImporter project;
        project.setProjectPath("/project/dir/");
        ASSERT_EQUALS(true, project.importCppcheckGuiProject(istr, s, supprs));
        ASSERT_EQUALS(1, project.guiProject.pathNames.size());
        ASSERT_EQUALS("/abs/path/file.cpp", project.guiProject.pathNames[0]);
    }

    void importCppcheckGuiProjectRelPathWithBase() const {
        // Relative paths in <paths> must be prepended with the project directory (mPath),
        // matching MSBuild/GUI behaviour where relative entries are project-dir-relative.
        // Also covers the mixed-CWD scenario: when --project=../foo.cppcheck is used,
        // mPath is "../", so a relative entry like "src/a.cpp" becomes "../src/a.cpp".
        REDIRECT;
        constexpr char xml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<project version=\"1\">\n"
                               "    <paths>\n"
                               "        <dir name=\"/abs/file.cpp\"/>\n"
                               "        <dir name=\"src/rel.cpp\"/>\n"
                               "    </paths>\n"
                               "</project>\n";
        std::istringstream istr(xml);
        Settings s;
        Suppressions supprs;
        TestImporter project;
        project.setProjectPath("../");
        ASSERT_EQUALS(true, project.importCppcheckGuiProject(istr, s, supprs));
        ASSERT_EQUALS(2, project.guiProject.pathNames.size());
        ASSERT_EQUALS("/abs/file.cpp", project.guiProject.pathNames[0]);
        ASSERT_EQUALS("../src/rel.cpp", project.guiProject.pathNames[1]);
    }

    void ignorePaths() const {
        FileSettings fs1{"foo/bar", Standards::Language::CPP, 0};
        FileSettings fs2{"qwe/rty", Standards::Language::CPP, 0};
        TestImporter project;
        project.fileSettings = {std::move(fs1), std::move(fs2)};

        project.ignorePaths({"*foo", "bar*"});
        ASSERT_EQUALS(1, project.fileSettings.size());

        project.ignorePaths({"foo/*"});
        ASSERT_EQUALS(1, project.fileSettings.size());
        ASSERT_EQUALS("qwe/rty", project.fileSettings.front().filename());

        project.ignorePaths({ "*e/r*" });
        ASSERT_EQUALS(0, project.fileSettings.size());
    }

    void testCollectArgs1() const
    {
        std::vector<std::string> args;
        const std::string cmd = "  gcc -o main main.c  ";
        const std::string error = TestImporter::collectArgs(cmd, args);

        ASSERT_EQUALS("", error);
        ASSERT_EQUALS(4, args.size());
        ASSERT_EQUALS("gcc", args[0]);
        ASSERT_EQUALS("-o", args[1]);
        ASSERT_EQUALS("main", args[2]);
        ASSERT_EQUALS("main.c", args[3]);
    }

    void testCollectArgs2() const
    {
        std::vector<std::string> args;
        const std::string cmd = "gcc -o main \"directory with space\"/main.c";
        const std::string error = TestImporter::collectArgs(cmd, args);

        ASSERT_EQUALS("", error);
        ASSERT_EQUALS(4, args.size());
        ASSERT_EQUALS("gcc", args[0]);
        ASSERT_EQUALS("-o", args[1]);
        ASSERT_EQUALS("main", args[2]);
        ASSERT_EQUALS("directory with space/main.c", args[3]);
    }

    void testCollectArgs3() const
    {
        std::vector<std::string> args;
        const std::string cmd = "gcc -o main directory\\ with\\ space/main.c";
        const std::string error = TestImporter::collectArgs(cmd, args);

        ASSERT_EQUALS("", error);
        ASSERT_EQUALS(4, args.size());
        ASSERT_EQUALS("gcc", args[0]);
        ASSERT_EQUALS("-o", args[1]);
        ASSERT_EQUALS("main", args[2]);
        ASSERT_EQUALS("directory with space/main.c", args[3]);
    }

    void testCollectArgs4() const
    {
        std::vector<std::string> args;
        const std::string cmd = "gcc -o main \'directory with space\'/main.c";
        const std::string error = TestImporter::collectArgs(cmd, args);

        ASSERT_EQUALS("", error);
        ASSERT_EQUALS(4, args.size());
        ASSERT_EQUALS("gcc", args[0]);
        ASSERT_EQUALS("-o", args[1]);
        ASSERT_EQUALS("main", args[2]);
        ASSERT_EQUALS("directory with space/main.c", args[3]);
    }

    void testCollectArgs5() const
    {
        std::vector<std::string> args;
        const std::string cmd = "gcc -o main directory_with_quote\\\"/main.c";
        const std::string error = TestImporter::collectArgs(cmd, args);

        ASSERT_EQUALS("", error);
        ASSERT_EQUALS(4, args.size());
        ASSERT_EQUALS("gcc", args[0]);
        ASSERT_EQUALS("-o", args[1]);
        ASSERT_EQUALS("main", args[2]);
        ASSERT_EQUALS("directory_with_quote\"/main.c", args[3]);
    }

    void testCollectArgs6() const
    {
        std::vector<std::string> args;
        const std::string cmd = "gcc -o main windows\\\\path\\\\main.c";
        const std::string error = TestImporter::collectArgs(cmd, args);

        ASSERT_EQUALS("", error);
        ASSERT_EQUALS(4, args.size());
        ASSERT_EQUALS("gcc", args[0]);
        ASSERT_EQUALS("-o", args[1]);
        ASSERT_EQUALS("main", args[2]);
        ASSERT_EQUALS("windows\\path\\main.c", args[3]);
    }

    void testCollectArgs7() const
    {
        std::vector<std::string> args;
        const std::string cmd = "gcc -o main \"non-terminated-quote/main.c";
        const std::string error = TestImporter::collectArgs(cmd, args);

        ASSERT_EQUALS("Missing closing quote in command string", error);
    }

    void testVcxprojConditions() const
    {
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'$(Configuration)'=='Debug'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'$(Platform)'=='Win32'", "Debug", "Win32"));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'$(Configuration)'=='Release'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" '$(Configuration)' == 'Debug' ", "Debug", "Win32"));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition(" '$(Configuration)' != 'Debug' ", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'$(Configuration)|$(Platform)' == 'Debug|Win32' ", "Debug", "Win32"));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("!('$(Configuration)|$(Platform)' == 'Debug|Win32' )", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" '$(Configuration)' == 'Debug' And '$(Platform)' == 'Win32'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" '$(Configuration)' == 'Debug' Or '$(Platform)' == 'Win32'", "Release", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.StartsWith('Debug'))", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.ToUpper().StartsWith('DEBUG'))", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.EndsWith('AddressSanitizer'))", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.Contains('Address'))", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.Contains ( 'Address'  ) )", "Debug-AddressSanitizer", "Win32"));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.StartsWith('Release'))", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Platform.Contains('32'))", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.Contains('Address')) And '$(Platform)' == 'Win32'", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" ($(Configuration.Contains('Address')) ) And ( '$(Platform)' == 'Win32')", "Debug-AddressSanitizer", "Win32"));
        // Relational operators - integer
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'14' >= '14'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'15' > '14'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'13' > '14'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'13' < '14'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'15' < '14'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'13' <= '14'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'14' <= '14'", "", ""));
        // Relational operators - version
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'14.0' >= '14.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'14.1' >= '14.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'13.0' >= '14.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.10.0.0' > '1.9.0.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'v14.0' >= '14.0'", "", ""));
        // Version comparison: full 4-part #.#.#.#
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4' == '1.2.3.4'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4' == '1.2.3.5'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3.5' > '1.2.3.4'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4' < '1.2.3.5'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'2.0.0.0' > '1.9.9.9'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.9.9.9' > '2.0.0.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4' >= '1.2.3.4'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4' <= '1.2.3.4'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4' > '1.2.3.4'", "", ""));
        // Version comparison: more than 4 parts (no truncation)
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4.5' > '1.2.3.4.4'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4.4' > '1.2.3.4.5'", "", ""));
        // == and != are NOT relational operators: real MSBuild documents them
        // as ordinary equality/inequality (numeric if both sides are plain
        // numbers, case-insensitive string otherwise), never the zero-padded,
        // component-wise equality $([MSBuild]::VersionEquals(...)) implements
        // (see testMSBuildStaticFunctions() for that function's own,
        // deliberately different, semantics). '1.2.3.4.0' and '1.2.3.4' are
        // different strings and neither parses as a plain number, so == and
        // != see them as unequal -- even though relationally (see
        // '17'/'17.0.0.0' below) MSBuildVersion's own comparator would treat
        // a shorter version as simply less than a longer one, not equal to it
        // either.
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4.0' == '1.2.3.4'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3.4.0' != '1.2.3.4'", "", ""));
        // Relational operators (<, >, <=, >=) DO use MSBuildVersion's
        // comparator, which treats a version's omitted trailing components as
        // -1 (see MSBuildVersion::cmp()'s doc comment) -- so a shorter
        // version string sorts strictly before a longer one, '17' < '17.0.0.0',
        // rather than comparing equal to it under either == or a relational op.
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'17' == '17.0.0.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'17' != '17.0.0.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'17' >= '17.0.0.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'17' <= '17.0.0.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'17' > '17.0.0.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'17' < '17.0.0.0'", "", ""));
        // Same reasoning as '17'/'17.0.0.0' above: == is plain (numeric- or
        // string-, never version-) comparison, so none of these are equal --
        // '17.0', '17.0.0.0' and '17.0.0' are all different strings from each
        // other and from '17', and none of them parses as a plain integer
        // (they contain '.').
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'17.0' == '17'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'17.0' == '17.0.0.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'17.0.0' == '17'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'17.1' > '17.0.0.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'16.9' > '17'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1' < '1.0.0.1'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3' < '1.2.3.1'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.2.3' > '1.2.2.9'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'2' > '1.9.9.9'", "", ""));
        // 'Current' on LHS
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Current' > '14'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Current' >= '18'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'Current' < '14'", "", ""));
        // 'Current' on RHS
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'14' < 'Current'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'18' <= 'Current'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'19' < 'Current'", "", ""));
        // Static property functions in conditions
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([MSBuild]::Add(1, 2)) == '3'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([MSBuild]::EnsureTrailingSlash('foo')) == 'foo/'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([System.String]::IsNullOrEmpty('')) == 'True'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([System.Math]::Max(1, 2)) == '2'", "", ""));
        // Unknown variable
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'$(DoesNotExist)' == ''", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'$(PATH)' != ''", "", ""));
        // Relational operators - error case
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("'14.0' >= ''", "", ""), std::runtime_error, "Cannot compare '14.0' and ''");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("And", "", ""), std::runtime_error, "Invalid condition: 'And'");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("Or", "", ""), std::runtime_error, "Invalid condition: 'Or'");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("!", "", ""), std::runtime_error, "Invalid condition: '!'");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("'' == '' And ", "", ""), std::runtime_error, "Missing operator");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("('' == ''", "", ""), std::runtime_error, "'(' without closing ')'!");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("'' == '')", "", ""), std::runtime_error, "unmatched ')' in condition '' == '')");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("''", "", ""), std::runtime_error, "Invalid condition: ''''");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("'' == '", "", ""), std::runtime_error, "Can not tokenize condition");
        // ToUpper / ToLower
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.ToUpper()) == 'DEBUG'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.ToLower()) == 'debug'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.ToUpper()) == 'debug'", "Debug", "Win32"));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("$(Configuration.ToUpper()) == 'RELEASE'", "Debug", "Win32"));
        // C-style && is not a valid MSBuild operator — throws rather than silently returning false
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("' ' && ' '", "", ""), std::runtime_error, "Invalid condition: '' ' && ' ''");
        // case insensitive
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Debug' == 'DEBUG'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'Debug' != 'DEBUG'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(configuration) == 'Debug'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(configuration) == 'Debug'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(CONFIGURATION) == 'Debug'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration) == 'Debug'", "Debug", "Win32"));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("true", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("TRUE", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("false", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("FALSE", "", ""));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("true And true", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("true Or false", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("true And false", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("false Or false", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("!false", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("!true", "", ""));

        // HasTrailingSlash
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("HasTrailingSlash('foo/')", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("HasTrailingSlash('foo\\')", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("HasTrailingSlash('foo')", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("HasTrailingSlash('')", "", ""));

        // string manipulation
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Trim()) == 'Debug'", "  Debug  ", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.TrimStart()) == 'Debug  '", "  Debug  ", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.TrimEnd()) == '  Debug'", "  Debug  ", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Substring(0, 5)) == 'Debug'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Substring(6)) == 'Test'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('-', '_')) == 'Debug_Test'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition( "$(Configuration.Trim().ToUpper()) == 'DEBUG'", "  Debug  ", "Win32"));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("true", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("false", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("true And false", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("!false", "", ""));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition( "$(Configuration.Substring(5)) == ''", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition( "$(Configuration.Substring(5, 0)) == ''", "Debug", "Win32"));
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Substring(-1)) == ''", "Debug", "Win32"), std::runtime_error, "Substring start index out of range");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition( "$(Configuration.Substring(4, 2)) == ''", "Debug", "Win32"), std::runtime_error, "Substring length out of range");

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Trim()) == 'Debug'", " \tDebug\r\n", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.TrimStart()) == 'Debug  '", "  Debug  ", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.TrimEnd()) == '  Debug'", "  Debug  ", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Trim('-')) == 'Debug'", "--Debug--", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.TrimStart('-')) == 'Debug--'", "--Debug--", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.TrimEnd('-')) == '--Debug'", "--Debug--", "Win32"));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('-', '_')) == 'Debug_Test'","Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('Debug', 'Release')) == 'Release-Test'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('x', 'y')) == 'Debug-Test'", "Debug-Test", "Win32"));
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('', 'x')) == 'Debug'", "Debug", "Win32"), std::runtime_error, "Replace search string cannot be empty");
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('Debug', 'Release')) == 'Release-Test'", "Debug-Test", "Win32"));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('debug', 'Release')) == 'Release-Test'", "Debug-Test", "Win32"));

        // Length property access (no parentheses)
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Length) == '5'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Length) == '0'", "", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Platform.Length) == '5'", "Debug", "Win32"));
        // Length after a method call in a chain
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.ToUpper().Length) == '5'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Replace('-', '').Length) == '9'", "Debug-Test", "Win32"));
        // Length in PropertyValueExpander (non-condition) path
        ASSERT_EQUALS("5", cppcheck::testing::expandMSBuildProperties("$(Configuration.Length)", "Debug", "Win32"));
        ASSERT_EQUALS("0", cppcheck::testing::expandMSBuildProperties("$(Configuration.Length)", "", "Win32"));
        ASSERT_EQUALS("5", cppcheck::testing::expandMSBuildProperties("$(Platform.Length)", "Debug", "Win32"));
        ASSERT_EQUALS("5", cppcheck::testing::expandMSBuildProperties("$(Configuration.ToUpper().Length)", "Debug", "Win32"));

        // IndexOf / LastIndexOf
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.IndexOf('-')) == '5'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.IndexOf('x')) == '-1'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.IndexOf('e')) == '1'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.IndexOf('e', '3')) == '7'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.LastIndexOf('e')) == '7'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.LastIndexOf('x')) == '-1'", "Debug-Test", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.LastIndexOf('e', '3')) == '1'", "Debug-Test", "Win32"));
        ASSERT_EQUALS("5",  cppcheck::testing::expandMSBuildProperties("$(Configuration.IndexOf('-'))", "Debug-Test", "Win32"));
        ASSERT_EQUALS("-1", cppcheck::testing::expandMSBuildProperties("$(Configuration.IndexOf('x'))", "Debug-Test", "Win32"));
        ASSERT_EQUALS("7",  cppcheck::testing::expandMSBuildProperties("$(Configuration.LastIndexOf('e'))", "Debug-Test", "Win32"));

        // PadLeft / PadRight
        ASSERT_EQUALS("  hi", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadLeft('4'))", "hi", "Win32"));
        ASSERT_EQUALS("hi  ", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadRight('4'))", "hi", "Win32"));
        ASSERT_EQUALS("00hi", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadLeft('4', '0'))", "hi", "Win32"));
        ASSERT_EQUALS("hi00", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadRight('4', '0'))", "hi", "Win32"));
        // no padding needed when value already long enough
        ASSERT_EQUALS("hello", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadLeft('3'))", "hello", "Win32"));
        ASSERT_EQUALS("hello", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadRight('3'))", "hello", "Win32"));
        // width exactly equal to value length — no padding
        ASSERT_EQUALS("hi", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadLeft('2'))", "hi", "Win32"));
        ASSERT_EQUALS("hi", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadRight('2'))", "hi", "Win32"));
        // width 0 — no padding
        ASSERT_EQUALS("hi", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadLeft('0'))", "hi", "Win32"));
        // chaining: pad then measure length
        ASSERT_EQUALS("6", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadLeft('6', '0').Length)", "hi", "Win32"));
        ASSERT_EQUALS("6", cppcheck::testing::expandMSBuildProperties("$(Configuration.PadRight('6', '0').Length)", "hi", "Win32"));
        // condition evaluation path
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.PadLeft('4', '0')) == '00hi'", "hi", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.PadRight('4', '0')) == 'hi00'", "hi", "Win32"));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(Configuration) == DEBUG", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$(configuration) == 'Debug'", "Debug", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'0x10' > '0x0F'", "", ""));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'0x0F' < '0x10'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'0x10' < '0x0F'", "", ""));

        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'0x10' > '0x0F'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'0x0F' < '0x10'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'0x10' < '0x0F'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'010' > '9'", "", ""));
        // Equality comparison: numeric, Boolean, then string fallback -- NOT
        // MSBuildVersion's zero-padded comparison (see the '17'/'17.0.0.0'
        // block above in testVcxprojConditions() for why): '0x10' and '16'
        // are both plain (hex/decimal) integers, so == compares them
        // numerically and they're equal; '1.0' is not a plain integer (it
        // contains '.') and is a different string from '1', so == falls back
        // to string comparison and they're NOT equal.
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'0x10' == '16'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.0' == '1'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'true' == 'TRUE'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Alpha' == 'alpha'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'Alpha' == 'Beta'", "", ""));
        // Boolean literals and quoted boolean strings are case-insensitive (H-2)
        // Unquoted keywords (matchWord is already case-insensitive)
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("true", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("false", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("True", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("False", "", ""));
        // Quoted boolean strings from property expansion (e.g. <WholeProgramOptimization>true</...>)
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'true'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'false'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'TRUE'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'FALSE'", "", ""));
        // Composition with mixed-case booleans
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'true' And 'True'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'true' And 'false'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'false' Or 'TRUE'", "", ""));
        // Equality comparison: numeric, Boolean, then string fallback -- NOT
        // MSBuildVersion's zero-padded comparison (see the '17'/'17.0.0.0'
        // block above in testVcxprojConditions() for why): '0x10' and '16'
        // are both plain (hex/decimal) integers, so == compares them
        // numerically and they're equal; '1.0' is not a plain integer (it
        // contains '.') and is a different string from '1', so == falls back
        // to string comparison and they're NOT equal.
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'0x10' == '16'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.0' == '1'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'true' == 'TRUE'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Alpha' == 'alpha'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'Alpha' == 'Beta'", "", ""));
    }

    // Regression coverage for compare()'s == / != previously sharing the
    // same numeric -> Boolean -> Version -> string cascade as the relational
    // operators <, >, <=, >=. Real MSBuild documents <, >, <=, >= as usable
    // only with numeric values -- a dotted, multi-component string isn't a
    // valid number there, which is exactly why the dedicated
    // $([MSBuild]::VersionGreaterThan(...)) etc. functions exist -- while ==
    // and != are documented as ordinary equality/inequality: numeric if both
    // sides are plain numbers, case-insensitive string otherwise. Before the
    // fix, == and != additionally fell through to MSBuildVersion's own
    // comparator when both sides parsed as a dotted version string, and that
    // comparator's own "==" implementation deliberately treats an omitted
    // trailing component as zero (1 == 1.0 == 1.0.0) -- correct for the
    // dedicated VersionEquals() function this codebase also exposes (see
    // testMSBuildStaticFunctions()'s $([MSBuild]::VersionEquals(...))
    // coverage), but not something a plain == in an ordinary
    // Condition="..." attribute should ever do. This meant e.g.
    // Condition="'$(SomeVersion)' == '1.1.0'" could spuriously match
    // '$(SomeVersion)'=='1.1', a real vcxproj-authoring hazard given how
    // often toolset/SDK version properties look exactly like this.
    void testVcxprojConditionEqualityIsNotVersionComparison() const {
        // The exact shape of bug report: a shorter and a longer dotted
        // version string must NOT compare equal under == or != , even though
        // MSBuildVersion's own zero-padded equality (used by
        // $([MSBuild]::VersionEquals(...)) -- and, before this fix, leaked
        // into plain ==) would treat them as the same value.
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.1' == '1.1.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.1' != '1.1.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1' == '1.0'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1' != '1.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1' == '1.0.0'", "", ""));

        // Relationally (<, >, <=, >=), MSBuildVersion's comparator treats an
        // omitted trailing component as -1, not 0 (matching .NET's
        // System.Version semantics), so a shorter version sorts strictly
        // BEFORE a longer one -- neither "equal" under == nor under the
        // relational operators either, it is simply less.
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.1' < '1.1.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.1' >= '1.1.0'", "", ""));

        // Two identical dotted version strings are still equal under == --
        // this is ordinary string equality doing its job, not version
        // comparison; nothing about excluding == from the version-comparison
        // path should affect the case where both sides are literally the
        // same text.
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'1.1.0' == '1.1.0'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'1.1.0' != '1.1.0'", "", ""));

        // Genuinely plain numbers (no dots) are still compared numerically
        // under == -- this really is documented MSBuild behavior for ==,
        // unlike the dotted-version case above, so leading zeros and the
        // like still numerically match.
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'01' == '1'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'01' != '1'", "", ""));

        // $([MSBuild]::VersionEquals(...)) is a distinct, dedicated function
        // with its own, deliberately zero-padding, semantics -- unaffected
        // by any of the above, since it never goes through compare() at all.
        // See testMSBuildStaticFunctions() for its general coverage; this is
        // exactly the '1.1'/'1.1.0' pair that a plain == above now rejects.
        ASSERT_EQUALS("True", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionEquals('1.1', '1.1.0'))"));
    }

    // Regression coverage for property-name identifier parsing rejecting '-'.
    // Microsoft documents a valid MSBuild property name as
    // [A-Za-z_][A-Za-z0-9_-]* -- '-' IS allowed after the first character,
    // e.g. <My-Property>foo</My-Property> referenced as $(My-Property) -- but
    // PropertyValueExpander::parseIdentifier() (used by expandMSBuildVariables(),
    // the general $(...) expansion path), ConditionParser's property-name
    // parsing (used by $(...) inside a Condition="..."), and
    // hasOnlyInvariantItemPathVariables()'s inline scanner (used for ClCompile
    // item Include/Update/Remove paths, see vcxproj_item_macro_path_hyphen_test.py
    // for CLI-level coverage of that one) all previously stopped at the first
    // '-', silently truncating the name -- so $(My-Property) was parsed as a
    // reference to a property literally named "My" followed by the literal
    // text "-Property)", which (barring an actual property named exactly
    // "My") just left the whole expression unexpanded, rather than resolving
    // "My-Property" as one property name the way real MSBuild does. Method
    // and member names are NOT affected -- MSBuild's own grammar restricts
    // those to [A-Za-z_][A-Za-z0-9_]* with no '-' (see
    // ConditionParser::parseMethodName()'s doc comment and the inline method
    // scans in PropertyValueExpander::tryParseExpr()) -- so this test also
    // confirms a hyphen right after a '.' in a method position is correctly
    // NOT consumed as part of the method name.
    //
    // expandMSBuildExpression()/evaluateVcxprojCondition() (the testing hooks
    // used here) only let a caller set Configuration/Platform in the
    // properties map directly, so an OS environment variable -- the other
    // source PropertyValueExpander::lookup() and ConditionParser::getPropertyValue()
    // already fall back to for any name not in the properties map -- is the
    // only way to inject an arbitrarily-named property through them.
    void testPropertyNameAllowsHyphen() const {
        const char *const envName = "CppcheckTest-Hyphen-Prop";
#ifdef _WIN32
        _putenv_s(envName, "hyphen-value");
#else
        setenv(envName, "hyphen-value", 1);
#endif

        // General property expansion (expandMSBuildVariables(), via
        // PropertyValueExpander::parseIdentifier()).
        ASSERT_EQUALS("hyphen-value", cppcheck::testing::expandMSBuildExpression("$(CppcheckTest-Hyphen-Prop)"));
        ASSERT_EQUALS("prefix-hyphen-value-suffix",
                      cppcheck::testing::expandMSBuildExpression("prefix-$(CppcheckTest-Hyphen-Prop)-suffix"));

        // Condition evaluation (ConditionParser::parsePropertyName(), via
        // parsePropertyExpression()).
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'$(CppcheckTest-Hyphen-Prop)' == 'hyphen-value'", "", ""));
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'$(CppcheckTest-Hyphen-Prop)' == 'wrong-value'", "", ""));

        // A hyphen is still correctly rejected in a METHOD name -- the
        // property name parses in full ("CppcheckTest-Hyphen-Prop"), but the
        // method-name scan then stops at the '-' in ".To-Upper", leaving
        // "To" as an unrecognized no-paren property access (passing `value`
        // through unchanged, exactly like an unrelated unknown property
        // accessor would) and "-Upper)" behind as literal, unconsumed text
        // that the surrounding expansion then copies through verbatim --
        // rather than treating "-" as part of the method name and matching
        // some (nonexistent) "To-Upper" method.
        ASSERT_EQUALS("hyphen-value-Upper)", cppcheck::testing::expandMSBuildExpression("$(CppcheckTest-Hyphen-Prop.To-Upper)"));

#ifdef _WIN32
        _putenv_s(envName, "");
#else
        unsetenv(envName);
#endif
    }

    void testMSBuildStaticFunctions() const {
        // --- $([MSBuild]::...) arithmetic ---
        ASSERT_EQUALS("3", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Add(1, 2))"));
        ASSERT_EQUALS("5", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Subtract(8, 3))"));
        ASSERT_EQUALS("12", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Multiply(3, 4))"));
        ASSERT_EQUALS("3", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Divide(9, 3))"));
        ASSERT_EQUALS("2", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Modulo(5, 3))"));

        // --- $([MSBuild]::...) path helpers ---
        ASSERT_EQUALS("foo/", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::EnsureTrailingSlash('foo'))"));
        ASSERT_EQUALS("foo/", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::EnsureTrailingSlash('foo/'))"));
        // NormalizePath: join segments, normalise separators, resolve . and ..
        // Use absolute first segments so results are deterministic (CWD-independent).
        ASSERT_EQUALS("/a/b/c",    cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizePath('/a', 'b', 'c'))"));
        ASSERT_EQUALS("/a/b/c",    cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizePath('/a\\b\\c'))"));
        ASSERT_EQUALS("/a/c",      cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizePath('/a/b/../c'))"));
        ASSERT_EQUALS("/a/b/c",    cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizePath('/a/b/./c'))"));
        ASSERT_EQUALS("C:/a/c",    cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizePath('C:\\a\\b\\..\\c'))"));
        ASSERT_EQUALS("C:/a/b/c",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizePath('C:\\a', 'b', 'c'))"));
        // NormalizeDirectory: same as NormalizePath but always has a trailing slash
        ASSERT_EQUALS("/a/b/c/",   cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizeDirectory('/a', 'b', 'c'))"));
        ASSERT_EQUALS("/a/b/c/",   cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizeDirectory('/a\\b\\c'))"));
        ASSERT_EQUALS("C:/a/b/c/", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizeDirectory('C:\\a', 'b', 'c'))"));
        // Regression: space between method name and '(' must not drop the arg list.
        // Before the fix this produced "" and a "unknown class MSBuild or member
        // NormalizeDirectory" debug entry because the arg parser never saw '('.
        ASSERT_EQUALS("/a/b/c/",   cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizeDirectory ('/a', 'b', 'c'))"));
        ASSERT_EQUALS("/a/b/c",    cppcheck::testing::expandMSBuildExpression("$([MSBuild]::NormalizePath ('/a', 'b', 'c'))"));
        // $(…) references inside quoted args must be expanded before the function
        // runs, so that '..' resolution operates on real path components.
        // expandMSBuildProperties pre-populates Configuration and Platform; use
        // them here as stand-ins for real path properties.
        ASSERT_EQUALS("Debug/", cppcheck::testing::expandMSBuildProperties("$([MSBuild]::NormalizeDirectory('$(Configuration)'))", "Debug", "Win32"));
        ASSERT_EQUALS("Win32/sub/", cppcheck::testing::expandMSBuildProperties("$([MSBuild]::NormalizeDirectory('$(Platform)', 'sub'))", "Debug", "Win32"));
        // The critical real-world pattern: NormalizeDirectory with an inner
        // property and '..' segments — must NOT collapse to empty.
        ASSERT_EQUALS("Debug/", cppcheck::testing::expandMSBuildProperties("$([MSBuild]::NormalizeDirectory('$(Configuration)', 'sub', '..'))", "Debug", "Win32"));
        // Unquoted $(Var)trailing/text arg: trailing bare-word text must be
        // concatenated onto the expanded value so that e.g.
        //   $(MSBuildThisFileDirectory)\..\tools  →  one concatenated arg, not two.
        // Use $(Configuration) as a stand-in for a real directory property.
        ASSERT_EQUALS("sub/", cppcheck::testing::expandMSBuildProperties("$([MSBuild]::NormalizeDirectory($(Configuration)\\..\\sub))", "Debug", "Win32"));
        ASSERT_EQUALS("sub", cppcheck::testing::expandMSBuildProperties("$([MSBuild]::NormalizePath($(Configuration)\\..\\sub))", "Debug", "Win32"));
        // Multi-arg quoted form with three inner $(…) args
        ASSERT_EQUALS("Debug/Win32/sub/", cppcheck::testing::expandMSBuildProperties("$([MSBuild]::NormalizeDirectory('$(Configuration)', '$(Platform)', 'sub'))", "Debug", "Win32"));
        // Version comparison functions (missing trailing components treated as 0)
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionGreaterThan('2.0', '1.9'))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionGreaterThan('1.9', '2.0'))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionGreaterThan('1.0', '1.0'))"));
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionGreaterThanOrEquals('1.0', '1.0'))"));
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionGreaterThanOrEquals('2.0', '1.0'))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionGreaterThanOrEquals('1.0', '2.0'))"));
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionLessThan('1.9', '2.0'))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionLessThan('2.0', '1.9'))"));
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionLessThanOrEquals('1.0', '1.0'))"));
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionEquals('1.0', '1.0.0'))"));   // missing component = 0
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionEquals('1.2.3', '1.2.3'))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionEquals('1.0', '1.1'))"));
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionNotEquals('1.0', '1.1'))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionNotEquals('1.0', '1.0.0'))"));
        // multi-component: 1.2.3.4 vs 1.2.3.5
        ASSERT_EQUALS("True",  cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionLessThan('1.2.3.4', '1.2.3.5'))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::VersionGreaterThan('1.2.3.4', '1.2.3.5'))"));
        // usable in a condition
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([MSBuild]::VersionGreaterThan('17.0', '16.9')) == 'True'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([MSBuild]::VersionEquals('14.0', '14.0.0')) == 'True'", "", ""));

        // ValueOrDefault: return first arg when non-empty, else second
        ASSERT_EQUALS("x", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::ValueOrDefault('x', 'y'))"));
        ASSERT_EQUALS("y", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::ValueOrDefault('', 'y'))"));
        // GetCurrentToolsVersion
        ASSERT_EQUALS("Current", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::GetCurrentToolsVersion())"));

        // --- $([MSBuild]::...) bitwise ---
        ASSERT_EQUALS("2", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::BitwiseAnd(6, 3))"));
        ASSERT_EQUALS("7", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::BitwiseOr(5, 3))"));
        ASSERT_EQUALS("6", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::BitwiseXor(5, 3))"));

        // --- $([MSBuild]::...) Escape / Unescape ---
        ASSERT_EQUALS("%3B", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Escape(';'))"));
        ASSERT_EQUALS(";", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Unescape('%3B'))"));
        ASSERT_EQUALS("%24", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Escape('$'))"));
        ASSERT_EQUALS("$", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Unescape('%24'))"));

        // --- $([System.String]::...) ---
        ASSERT_EQUALS("True", cppcheck::testing::expandMSBuildExpression("$([System.String]::IsNullOrEmpty(''))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([System.String]::IsNullOrEmpty('x'))"));
        ASSERT_EQUALS("True", cppcheck::testing::expandMSBuildExpression("$([System.String]::IsNullOrWhiteSpace('  '))"));
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([System.String]::IsNullOrWhiteSpace('x'))"));
        ASSERT_EQUALS("ab", cppcheck::testing::expandMSBuildExpression("$([System.String]::Concat('a', 'b'))"));
        ASSERT_EQUALS("a,b", cppcheck::testing::expandMSBuildExpression("$([System.String]::Join(',', 'a', 'b'))"));
        // String.Format: plain substitution
        ASSERT_EQUALS("foo and bar", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0} and {1}', 'foo', 'bar'))"));
        // String.Format: escaped braces
        ASSERT_EQUALS("{literal}", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{{literal}}'))"));
        // String.Format: D (decimal, optional zero-padding)
        ASSERT_EQUALS("42",     cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:D}', '42'))"));
        ASSERT_EQUALS("000042", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:D6}', '42'))"));
        ASSERT_EQUALS("-000005", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:D6}', '-5'))"));
        // String.Format: X / x (hexadecimal)
        ASSERT_EQUALS("FF",   cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:X}', '255'))"));
        ASSERT_EQUALS("00FF", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:X4}', '255'))"));
        ASSERT_EQUALS("00ff", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:x4}', '255'))"));
        // String.Format: F (fixed-point)
        ASSERT_EQUALS("3.14",   cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:F2}', '3.14159'))"));
        ASSERT_EQUALS("3.1416", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:F4}', '3.14159'))"));
        // String.Format: non-numeric arg with a numeric specifier — pass through unchanged
        ASSERT_EQUALS("abc", cppcheck::testing::expandMSBuildExpression("$([System.String]::Format('{0:D6}', 'abc'))"));

        // --- $([System.Math]::...) ---
        ASSERT_EQUALS("10", cppcheck::testing::expandMSBuildExpression("$([System.Math]::Max(5, 10))"));
        ASSERT_EQUALS("5", cppcheck::testing::expandMSBuildExpression("$([System.Math]::Min(5, 10))"));
        ASSERT_EQUALS("5", cppcheck::testing::expandMSBuildExpression("$([System.Math]::Abs(-5))"));
        ASSERT_EQUALS("5", cppcheck::testing::expandMSBuildExpression("$([System.Math]::Abs(5))"));
        ASSERT_EQUALS("2", cppcheck::testing::expandMSBuildExpression("$([System.Math]::Floor(2.9))"));
        ASSERT_EQUALS("3", cppcheck::testing::expandMSBuildExpression("$([System.Math]::Ceiling(2.1))"));

        // --- $([System.IO.Path]::...) ---
        ASSERT_EQUALS("bar.cpp", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::GetFileName('C:/foo/bar.cpp'))"));
        ASSERT_EQUALS("bar", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::GetFileNameWithoutExtension('C:/foo/bar.cpp'))"));
        ASSERT_EQUALS("C:/foo", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::GetDirectoryName('C:/foo/bar.cpp'))"));
        ASSERT_EQUALS(".cpp", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::GetExtension('bar.cpp'))"));
        ASSERT_EQUALS("True", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::IsPathRooted('C:/foo'))"));
        ASSERT_EQUALS("True", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::IsPathRooted('C:\\\\foo'))"));
        ASSERT_EQUALS("True", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::IsPathRooted('\\\\server\\\\share'))"));
        ASSERT_EQUALS("True", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::IsPathRooted('C:file.cpp'))"));  // drive-relative: .NET returns True (has drive letter)
        ASSERT_EQUALS("False", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::IsPathRooted('foo'))"));
        // Path.Combine: 1, 2, 3, N args; absolute segment resets path
        ASSERT_EQUALS("a", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::Combine('a'))"));
        ASSERT_EQUALS("a/b", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::Combine('a', 'b'))"));
        ASSERT_EQUALS("a/b/c", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::Combine('a', 'b', 'c'))"));
        ASSERT_EQUALS("a/b/c/d", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::Combine('a', 'b', 'c', 'd'))"));
        ASSERT_EQUALS("/abs/b", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::Combine('a', '/abs', 'b'))"));
        ASSERT_EQUALS("/abs", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::Combine('a', 'b', '/abs'))"));
        ASSERT_EQUALS(Path::fromNativeSeparators(Path::getCurrentPath()) + "/a/b/c/d", cppcheck::testing::expandMSBuildExpression("$([System.IO.Path]::GetFullPath($([System.IO.Path]::Combine('a', 'b', 'c', 'd')))"));

        // --- $([System.IO.FileInfo]::new(...)) / $([System.IO.DirectoryInfo]::new(...)) ---
        // ::new returns the normalized path; chains resolve on that string.
        ASSERT_EQUALS("C:/foo/bar.cpp", cppcheck::testing::expandMSBuildExpression("$([System.IO.FileInfo]::new('C:/foo/bar.cpp').FullName)"));
        ASSERT_EQUALS("C:/foo/", cppcheck::testing::expandMSBuildExpression("$([System.IO.FileInfo]::new('C:/foo/bar.cpp').DirectoryName)"));
        ASSERT_EQUALS("bar.cpp", cppcheck::testing::expandMSBuildExpression("$([System.IO.FileInfo]::new('C:/foo/bar.cpp').Name)"));
        ASSERT_EQUALS("C:/foo", cppcheck::testing::expandMSBuildExpression("$([System.IO.DirectoryInfo]::new('C:/foo').FullName)"));
        // .Method(args) chain after ::new — e.g. replace in the resolved path
        ASSERT_EQUALS("C:/foo/baz.cpp", cppcheck::testing::expandMSBuildExpression("$([System.IO.FileInfo]::new('C:/foo/bar.cpp').FullName.Replace('bar', 'baz'))"));

        // --- Composite / nesting ---
        ASSERT_EQUALS("6", cppcheck::testing::expandMSBuildExpression("$([MSBuild]::Add($([MSBuild]::Multiply(2, 2)), 2))"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([MSBuild]::Add(1, 2)) == '3'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([System.String]::IsNullOrEmpty('')) == 'True'", "", ""));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("$([System.Math]::Max(10, 5)) == '10'", "", ""));
    }

    void testVcxitemsPathResolution() const {
        // importVcxitems must absolutise relative paths relative to ProjectDir,
        // MSBuild always sets ProjectDir with a trailing slash — use that canonical form.
        // Path styles differ by platform: Path::isAbsolute requires C:/ on Windows, / on Linux.
#ifdef _WIN32
        const std::string projDir    = "C:/proj/";
        const std::string projSub    = "C:/proj/sub/Shared.vcxitems";
        const std::string projRoot   = "C:/proj/Shared.vcxitems";
        const std::string absPath    = "C:/absolute/Shared.vcxitems";
        const std::string absInput   = "C:/absolute/Shared.vcxitems";
#else
        const std::string projDir    = "/proj/";
        const std::string projSub    = "/proj/sub/Shared.vcxitems";
        const std::string projRoot   = "/proj/Shared.vcxitems";
        const std::string absPath    = "/absolute/Shared.vcxitems";
        const std::string absInput   = "/absolute/Shared.vcxitems";
#endif
        // Relative path + ProjectDir → absolutised result
        ASSERT_EQUALS(projSub,  cppcheck::testing::resolveVcxitemsFilename("sub/Shared.vcxitems", projDir));
        ASSERT_EQUALS(projRoot, cppcheck::testing::resolveVcxitemsFilename("Shared.vcxitems", projDir));

        // Already-absolute path must pass through unchanged regardless of ProjectDir
        ASSERT_EQUALS(absPath, cppcheck::testing::resolveVcxitemsFilename(absInput, projDir));

        // No ProjectDir → relative path is returned as-is
        ASSERT_EQUALS("Shared.vcxitems", cppcheck::testing::resolveVcxitemsFilename("Shared.vcxitems", ""));
    }

    void testMissingChildImportNonFatal() const {
        // A .vcxproj that imports a non-existent .props file must still import
        // successfully.  Before the fix, result > NotResolvable caused errors.emplace_back()
        // + return false, aborting the whole project.  After the fix it is a debug warning
        // and the remaining ClCompile items are still collected.

        // Use the current directory (no subdirectory) to avoid ScopedFile throwing
        // "directory already exists" when the test is re-run without a clean build.
        const ScopedFile mainCpp("testm3_main.cpp", "");
        const ScopedFile vcxproj(
            "testm3.vcxproj",
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
            "  <ItemGroup Label=\"ProjectConfigurations\">\n"
            "    <ProjectConfiguration Include=\"Debug|Win32\">\n"
            "      <Configuration>Debug</Configuration>\n"
            "      <Platform>Win32</Platform>\n"
            "    </ProjectConfiguration>\n"
            "  </ItemGroup>\n"
            "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\" Label=\"Configuration\">\n"
            "    <ConfigurationType>Application</ConfigurationType>\n"
            "    <PlatformToolset>v143</PlatformToolset>\n"
            "  </PropertyGroup>\n"
            "  <!-- Import a file that does not exist: fix must not abort import -->\n"
            "  <Import Project=\"nonexistent_m3_missing.props\" />\n"
            "  <ItemGroup>\n"
            "    <ClCompile Include=\"testm3_main.cpp\" />\n"
            "  </ItemGroup>\n"
            "</Project>\n");

        ImportProject project;
        const ImportProject::Type result = project.import(vcxproj.path());

        // Must succeed — missing child imports must not abort the project.
        ASSERT_EQUALS(static_cast<int>(ImportProject::Type::VS_VCXPROJ), static_cast<int>(result));

        // No hard errors — the missing import is demoted to a debug warning.
        ASSERT(project.errors.empty());

        // The source file must still have been collected despite the missing import.
        const bool foundMain = std::any_of(project.fileSettings.begin(), project.fileSettings.end(), [](const FileSettings &fs) {
            return fs.filename().find("main.cpp") != std::string::npos;
        });
        ASSERT(foundMain);
    }

    void testCurrentToolsVersionFromProps() const {
        // L-3: "Current" keyword in relational conditions must use VisualStudioVersion
        // from the properties map rather than a hardcoded major version number.
        //
        // evaluateVcxprojCondition uses an empty properties map, so VisualStudioVersion
        // is absent and the fallback value { 18 } applies.  These assertions verify
        // the comparison logic is correct for the fallback case; an end-to-end
        // .vcxproj test with <VisualStudioVersion> set would be needed to exercise
        // the property-lookup path directly.

        // fallback Current = MSBuildVersion::parse("18.0") = {18,0} (VS 2026)
        // Now that the fallback is a two-component version "18.0", the equal-major
        // comparisons work correctly without .NET single-component ambiguity.
        // "Current" >= "18.0" -> {18,0} >= {18,0} -> true
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Current' >= '18.0'", "Debug", "Win32"));
        // "Current" <= "18.0" -> {18,0} <= {18,0} -> true
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Current' <= '18.0'", "Debug", "Win32"));
        // "Current" > "17.0" -> {18,0} > {17,0} -> true
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Current' > '17.0'", "Debug", "Win32"));
        // "17.0" < "Current" -> {17,0} < {18,0} -> true
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'17.0' < 'Current'", "Debug", "Win32"));
        // "Current" < "19.0" -> {18,0} < {19,0} -> true
        ASSERT(cppcheck::testing::evaluateVcxprojCondition("'Current' < '19.0'", "Debug", "Win32"));
        // "Current" > "19.0" -> {18,0} > {19,0} -> false
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("'Current' > '19.0'", "Debug", "Win32"));
    }

    void testMetadataSelfReferenceCaseInsensitive() const {
        // MSBuild item metadata names are case-insensitive (MetadataMap already
        // uses cppcheck::stricmp for exactly this reason). Some .vcxproj/.props
        // files use property-style accumulation in an ItemDefinitionGroup (e.g.
        // <PreprocessorDefinitions>$(PreprocessorDefinitions);EXTRA</PreprocessorDefinitions>)
        // instead of the %(...) item-metadata-reference syntax -- addMetadata()
        // handles that "$(eName) self-reference" idiom by substituting in the
        // metadata value accumulated so far. That substitution used to go through
        // the plain, case-sensitive findAndReplace() instead of
        // findAndReplaceCaseInsensitive() (used everywhere else in this file for
        // exactly this kind of property/metadata name comparison), so a
        // self-reference spelled with different casing than the element's own
        // tag name -- e.g. $(preprocessordefinitions) inside a
        // <PreprocessorDefinitions> element -- was not recognized and was left
        // in the output as literal, unresolved macro text instead of being
        // replaced by the value accumulated from the preceding
        // ItemDefinitionGroup.
        //
        // Two sequential ItemDefinitionGroup blocks exercise this: the first
        // establishes the base value; the second references it back via
        // $(preprocessordefinitions) in a different case than the
        // <PreprocessorDefinitions> tag itself.

        const ScopedFile mainCpp("testcasemeta_main.cpp", "");
        const ScopedFile vcxproj(
            "testcasemeta.vcxproj",
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n"
            "  <ItemGroup Label=\"ProjectConfigurations\">\n"
            "    <ProjectConfiguration Include=\"Debug|Win32\">\n"
            "      <Configuration>Debug</Configuration>\n"
            "      <Platform>Win32</Platform>\n"
            "    </ProjectConfiguration>\n"
            "  </ItemGroup>\n"
            "  <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\" Label=\"Configuration\">\n"
            "    <ConfigurationType>Application</ConfigurationType>\n"
            "    <PlatformToolset>v143</PlatformToolset>\n"
            "  </PropertyGroup>\n"
            "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n"
            "    <ClCompile>\n"
            "      <PreprocessorDefinitions>BASE_DEFINE</PreprocessorDefinitions>\n"
            "    </ClCompile>\n"
            "  </ItemDefinitionGroup>\n"
            "  <!-- Self-reference spelled in a different case than the element's own\n"
            "       tag name -- real MSBuild (case-insensitive metadata names) still\n"
            "       resolves this to the value accumulated by the ItemDefinitionGroup\n"
            "       above (\"BASE_DEFINE\"). -->\n"
            "  <ItemDefinitionGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|Win32'\">\n"
            "    <ClCompile>\n"
            "      <PreprocessorDefinitions>$(preprocessordefinitions);EXTRA_DEFINE</PreprocessorDefinitions>\n"
            "    </ClCompile>\n"
            "  </ItemDefinitionGroup>\n"
            "  <ItemGroup>\n"
            "    <ClCompile Include=\"testcasemeta_main.cpp\" />\n"
            "  </ItemGroup>\n"
            "</Project>\n");

        ImportProject project;
        const ImportProject::Type result = project.import(vcxproj.path());
        ASSERT_EQUALS(static_cast<int>(ImportProject::Type::VS_VCXPROJ), static_cast<int>(result));

        const auto it = std::find_if(project.fileSettings.begin(), project.fileSettings.end(), [](const FileSettings &fs) {
            return fs.filename().find("testcasemeta_main.cpp") != std::string::npos;
        });
        ASSERT(it != project.fileSettings.end());

        // Must end with the fully-resolved accumulation
        // "BASE_DEFINE=1;EXTRA_DEFINE=1" -- not a leftover literal
        // "$(preprocessordefinitions)" macro reference, which would mean the
        // case-mismatched self-reference was never recognized and substituted.
        // (fs.defines is prefixed with the toolset's own predefined macros --
        // e.g. _MSC_VER -- which this test does not otherwise care about.)
        ASSERT(it->defines.find("$(preprocessordefinitions)") == std::string::npos);
        const std::string expectedSuffix = ";BASE_DEFINE=1;EXTRA_DEFINE=1";
        ASSERT(it->defines.size() >= expectedSuffix.size());
        ASSERT_EQUALS(expectedSuffix, it->defines.substr(it->defines.size() - expectedSuffix.size()));
    }

    // TODO: test fsParseCommand()

};

REGISTER_TEST(TestImportProject)
