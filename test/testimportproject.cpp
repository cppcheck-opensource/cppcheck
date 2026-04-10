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
#include "importproject.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "xml.h"

#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

class TestImporter final : public ImportProject {
public:
    using ImportProject::importCompileCommands;
    using ImportProject::importCppcheckGuiProject;
    using ImportProject::importVcxproj;
    using ImportProject::SharedItemsProject;
    using ImportProject::collectArgs;
    using ImportProject::fsSetDefines;
    using ImportProject::fsSetIncludePaths;
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
        TEST_CASE(importCompileCommands1);
        TEST_CASE(importCompileCommands2); // #8563, #9567
        TEST_CASE(importCompileCommands3); // check with existing trailing / in directory
        TEST_CASE(importCompileCommands4); // only accept certain file types
        TEST_CASE(importCompileCommands5); // Windows/CMake/Ninja generated compile_commands.json
        TEST_CASE(importCompileCommands6); // Windows/CMake/Ninja generated compile_commands.json with spaces
        TEST_CASE(importCompileCommands7); // linux: "/home/danielm/cppcheck 2"
        TEST_CASE(importCompileCommands8); // Windows: "C:\Users\danielm\cppcheck"
        TEST_CASE(importCompileCommands9);
        TEST_CASE(importCompileCommands10); // #10887: include path with space
        TEST_CASE(importCompileCommands11); // include path order
        TEST_CASE(importCompileCommands12); // #13040: "directory" is parent directory, relative include paths
        TEST_CASE(importCompileCommands13); // #13333: duplicate file entries
        TEST_CASE(importCompileCommands14); // #14156
        TEST_CASE(importCompileCommands15); // #14306
        TEST_CASE(importCompileCommandsArgumentsSection); // Handle arguments section
        TEST_CASE(importCompileCommandsNoCommandSection); // gracefully handles malformed json
        TEST_CASE(importCompileCommandsDirectoryMissing); // 'directory' field missing
        TEST_CASE(importCompileCommandsDirectoryInvalid); // 'directory' field not a string
        TEST_CASE(importCppcheckGuiProject);
        TEST_CASE(importCppcheckGuiProjectPremiumMisra);
        TEST_CASE(ignorePaths);
        TEST_CASE(testVcxprojUnicode);
        TEST_CASE(testCollectArgs1);
        TEST_CASE(testCollectArgs2);
        TEST_CASE(testCollectArgs3);
        TEST_CASE(testCollectArgs4);
        TEST_CASE(testCollectArgs5);
        TEST_CASE(testCollectArgs6);
        TEST_CASE(testCollectArgs7);
        TEST_CASE(testVcxprojConditions);
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
    }

    void setIncludePaths1() const {
        FileSettings fs{"test.cpp", Standards::Language::CPP, 0};
        std::list<std::string> in(1, "../include");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        TestImporter::fsSetIncludePaths(fs, "abc/def/", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("abc/include/", fs.includePaths.front());
    }

    void setIncludePaths2() const {
        FileSettings fs{"test.cpp", Standards::Language::CPP, 0};
        std::list<std::string> in(1, "$(SolutionDir)other");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        variables["SolutionDir"] = "c:/abc/";
        TestImporter::fsSetIncludePaths(fs, "/home/fred", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void setIncludePaths3() const { // macro names are case insensitive
        FileSettings fs{"test.cpp", Standards::Language::CPP, 0};
        std::list<std::string> in(1, "$(SOLUTIONDIR)other");
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        variables["SolutionDir"] = "c:/abc/";
        TestImporter::fsSetIncludePaths(fs, "/home/fred", in, variables);
        ASSERT_EQUALS(1U, fs.includePaths.size());
        ASSERT_EQUALS("c:/abc/other/", fs.includePaths.front());
    }

    void importCompileCommands1() const {
        REDIRECT;
        constexpr char json[] = R"([{
                                   "directory": "/tmp",
                                   "command": "gcc -DTEST1 -DTEST2=2 -o /tmp/src.o -c /tmp/src.c",
                                   "file": "/tmp/src.c"
                                   }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("TEST1=1;TEST2=2", importer.fileSettings.cbegin()->defines);
    }

    void importCompileCommands2() const {
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/bar.c", importer.fileSettings.cbegin()->filename());
#endif
    }

    void importCompileCommands3() const {
        REDIRECT;
        const char json[] = R"([{
                                    "directory": "/tmp/",
                                    "command": "gcc -c src.c",
                                    "file": "src.c"
                               }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.cbegin()->filename());
    }

    void importCompileCommands4() const {
        REDIRECT;
        constexpr char json[] = R"([{
                                    "directory": "/tmp/",
                                    "command": "gcc -c src.mm",
                                    "file": "src.mm"
                                   }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
    }

    void importCompileCommands5() const {
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/src/", importer.fileSettings.cbegin()->includePaths.front());
    }

    void importCompileCommands6() const {
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/src/", importer.fileSettings.cbegin()->includePaths.front());
        ASSERT_EQUALS("C:/Users/dan/git/test-cppcheck/mylib/second src/", importer.fileSettings.cbegin()->includePaths.back());
    }


    void importCompileCommands7() const {
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("FILESDIR=\"/some/path\"", importer.fileSettings.cbegin()->defines);
        ASSERT_EQUALS(1, importer.fileSettings.cbegin()->includePaths.size());
        ASSERT_EQUALS("/home/danielm/cppcheck 2/b/lib/", importer.fileSettings.cbegin()->includePaths.front());
        TODO_ASSERT_EQUALS("/home/danielm/cppcheck 2/externals/",
                           "/home/danielm/cppcheck 2/b/lib/",
                           importer.fileSettings.cbegin()->includePaths.back());
    }

    void importCompileCommands8() const {
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr)); // Do not crash
    }

    void importCompileCommands9() const {
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
    }

    void importCompileCommands10() const { // #10887
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("/home/danielm/cppcheck/test folder/", fs.includePaths.front());
    }

    void importCompileCommands11() const { // include path order
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("/x/def/", fs.includePaths.front());
        ASSERT_EQUALS("/x/abc/", fs.includePaths.back());
    }

    void importCompileCommands12() const { // #13040
        REDIRECT;
        constexpr char json[] =
            R"([{
               "file": "/x/src/1.c" ,
               "directory": "/x",
               "command": "cc -c -I. src/1.c"
            }])";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS(1, fs.includePaths.size());
        ASSERT_EQUALS("/x/", fs.includePaths.front());
    }

    void importCompileCommands13() const { // #13333
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(2, importer.fileSettings.size());
        const FileSettings &fs1 = importer.fileSettings.front();
        const FileSettings &fs2 = importer.fileSettings.back();
        ASSERT_EQUALS(0, fs1.file.fsFileId());
        ASSERT_EQUALS(1, fs2.file.fsFileId());
    }

    void importCompileCommands14() const { // #14156
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS("TFS_LINUX_MODULE_NAME=\"tfs_linux\"", fs.defines);
    }

    void importCompileCommands15() const { // #14306
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
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        const FileSettings &fs = importer.fileSettings.front();
        ASSERT_EQUALS(1, fs.includePaths.size());
        ASSERT_EQUALS("C:/Users/abcd/efg/hijk/path/123/", fs.includePaths.front());
    }

    void importCompileCommandsArgumentsSection() const {
        REDIRECT;
        constexpr char json[] = "[ { \"directory\": \"/tmp/\","
                                "\"arguments\": [\"gcc\", \"-c\", \"src.c\"],"
                                "\"file\": \"src.c\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(true, importer.importCompileCommands(istr));
        ASSERT_EQUALS(1, importer.fileSettings.size());
        ASSERT_EQUALS("/tmp/src.c", importer.fileSettings.cbegin()->filename());
    }

    void importCompileCommandsNoCommandSection() const {
        REDIRECT;
        constexpr char json[] = "[ { \"directory\": \"/tmp/\","
                                "\"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(false, importer.importCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
        ASSERT_EQUALS(1, importer.errors.size());
        ASSERT_EQUALS("no 'arguments' or 'command' field found in compilation database entry", importer.errors[0]);
    }

    void importCompileCommandsDirectoryMissing() const {
        REDIRECT;
        constexpr char json[] = "[ { \"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(false, importer.importCompileCommands(istr));
        ASSERT_EQUALS(0, importer.fileSettings.size());
        ASSERT_EQUALS(1, importer.errors.size());
        ASSERT_EQUALS("'directory' field in compilation database entry missing", importer.errors[0]);
    }

    void importCompileCommandsDirectoryInvalid() const {
        REDIRECT;
        constexpr char json[] = "[ { \"directory\": 123,"
                                "\"file\": \"src.mm\" } ]";
        std::istringstream istr(json);
        TestImporter importer;
        ASSERT_EQUALS(false, importer.importCompileCommands(istr));
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
        ASSERT_EQUALS(true, s.inlineSuppressions);
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

    void testVcxprojUnicode() const
    {
        const char vcxproj[] = R"-(
<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|Win32">
      <Configuration>Release</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Configuration">
    <!-- Only to test that the last configuration entry overwrites this -->
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>true</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|Win32'" Label="Configuration">
    <ConfigurationType>Application</ConfigurationType>
    <UseDebugLibraries>false</UseDebugLibraries>
    <PlatformToolset>v143</PlatformToolset>
    <CharacterSet>NotSet</CharacterSet>
    <UseOfMfc>Static</UseOfMfc>
  </PropertyGroup>
  <ItemGroup>
    <ClCompile Include="main.cpp" />
  </ItemGroup>
</Project>
)-";
        tinyxml2::XMLDocument doc;
        ASSERT_EQUALS(tinyxml2::XML_SUCCESS, doc.Parse(vcxproj, sizeof(vcxproj)));
        TestImporter project;
        std::map<std::string, std::string, cppcheck::stricmp> variables;
        std::vector<TestImporter::SharedItemsProject> cache;
        ASSERT_EQUALS(project.importVcxproj("test.vcxproj", doc, variables, {}, {}, cache), true);
        ASSERT_EQUALS(project.fileSettings.size(), 2);
        ASSERT(project.fileSettings.front().defines.find(";UNICODE=1;") != std::string::npos);
        ASSERT(project.fileSettings.front().defines.find(";_UNICODE=1") != std::string::npos);
        ASSERT(project.fileSettings.front().defines.find(";_UNICODE=1;") == std::string::npos); // No duplicates
        ASSERT_EQUALS(project.fileSettings.front().useMfc, false);
        ASSERT(project.fileSettings.back().defines.find(";UNICODE=1;") == std::string::npos);
        ASSERT(project.fileSettings.back().defines.find(";_UNICODE=1") == std::string::npos);
        ASSERT_EQUALS(project.fileSettings.back().useMfc, true);
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
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.EndsWith('AddressSanitizer'))", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.Contains('Address'))", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.Contains ( 'Address'  ) )", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" $(Configuration.Contains('Address')) And '$(Platform)' == 'Win32'", "Debug-AddressSanitizer", "Win32"));
        ASSERT(cppcheck::testing::evaluateVcxprojCondition(" ($(Configuration.Contains('Address')) ) And ( '$(Platform)' == 'Win32')", "Debug-AddressSanitizer", "Win32"));
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("And", "", ""), std::runtime_error, "Invalid condition: 'And'");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("Or", "", ""), std::runtime_error, "Invalid condition: 'Or'");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("!", "", ""), std::runtime_error, "Invalid condition: '!'");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("'' == '' And ", "", ""), std::runtime_error, "Missing operator");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("('' == ''", "", ""), std::runtime_error, "'(' without closing ')'!");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("'' == '')", "", ""), std::runtime_error, "unmatched ')' in condition '' == '')");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("''", "", ""), std::runtime_error, "Invalid condition: ''''");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("'' == '", "", ""), std::runtime_error, "Can not tokenize condition");
        ASSERT_THROW_EQUALS(cppcheck::testing::evaluateVcxprojCondition("$(Configuration.Lower())", "", ""), std::runtime_error, "Missing operator");
        // invalid expression in => no error. We are ok with that as long as we don't crash
        ASSERT(!cppcheck::testing::evaluateVcxprojCondition("' ' && ' '", "", ""));
    }

    // TODO: test fsParseCommand()

};

REGISTER_TEST(TestImportProject)
