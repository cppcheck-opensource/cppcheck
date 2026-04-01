/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

// Generate Makefile for cppcheck

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../cli/filelister.h"
#include "../lib/filesettings.h"
#include "../lib/path.h"
#include "../lib/pathmatch.h"
#include "../lib/utils.h"

static std::string builddir(std::string filename)
{
    if (startsWith(filename,"lib/"))
        filename = "$(libcppdir)" + filename.substr(3);
    else if (startsWith(filename, "../lib/")) // oss-fuzz
        filename = "$(libcppdir)" + filename.substr(6);

    return filename;
}

static std::string objfile(std::string cppfile)
{
    cppfile.erase(cppfile.rfind('.'));
    if (startsWith(cppfile, "../externals/simplecpp/")) // oss-fuzz
        cppfile = cppfile.substr(23);
    else if (startsWith(cppfile, "../externals/tinyxml2/")) // oss-fuzz
        cppfile = cppfile.substr(22);
    return builddir(cppfile + ".o");
}

static std::string objfiles(const std::vector<std::string> &files)
{
    std::string allObjfiles;
    for (const std::string &file : files) {
        if (file != files.front())
            allObjfiles += std::string(14, ' ');
        allObjfiles += objfile(file);
        if (file != files.back())
            allObjfiles += " \\\n";
    }
    return allObjfiles;
}

static void getDeps(std::string filename, std::vector<std::string> &depfiles)
{
    static const std::array<std::string, 3> externalfolders{"externals/picojson", "externals/simplecpp", "externals/tinyxml2"};
    static const std::array<std::string, 3> externalfolders_rel{"../externals/picojson", "../externals/simplecpp", "../externals/tinyxml2"};

    // Is the dependency already included?
    if (std::find(depfiles.cbegin(), depfiles.cend(), filename) != depfiles.cend())
        return;

    const bool relative = startsWith(filename, "../"); // oss-fuzz
    if (relative)
        filename = filename.substr(3);

    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        /*
         * Recursively search for includes in other directories.
         * Files are searched according to the following priority:
         * [test, tools] -> cli -> lib -> externals
         */
        if (startsWith(filename, "frontend/"))
            getDeps("lib" + filename.substr(filename.find('/')), depfiles);
        if (startsWith(filename, "cli/"))
            getDeps("lib" + filename.substr(filename.find('/')), depfiles);
        else if (startsWith(filename, "test/"))
            getDeps("cli" + filename.substr(filename.find('/')), depfiles);
        else if (startsWith(filename, "tools"))
            getDeps("cli" + filename.substr(filename.find('/')), depfiles);
        else if (startsWith(filename, "lib/")) {
            const auto& extfolders = relative ? externalfolders_rel : externalfolders;
            for (const std::string & external : extfolders)
                getDeps(external + filename.substr(filename.find('/')), depfiles);
        }
        return;
    }
    if (filename.find(".c") == std::string::npos)
    {
        if (relative)
            depfiles.push_back("../" + filename);
        else
            depfiles.push_back(filename);
    }

    std::string path(filename);
    if (path.find('/') != std::string::npos)
        path.erase(1 + path.rfind('/'));

    std::string line;
    while (std::getline(f, line)) {
        std::string::size_type pos1 = line.find("#include \"");
        char rightBracket = '\"';
        if (pos1 == std::string::npos) {
            pos1 = line.find("#include <");
            rightBracket = '>';
            if (pos1 == std::string::npos)
                continue;
        }

        pos1 += 10;

        const std::string::size_type pos2 = line.find(rightBracket, pos1);
        std::string hfile(path);
        hfile += line.substr(pos1, pos2 - pos1);

        const std::string::size_type traverse_pos = hfile.find("/../");
        if (traverse_pos != std::string::npos)    // TODO: Ugly fix
            hfile.erase(0, 4 + traverse_pos);
        // no need to look up extension-less headers
        if (!endsWith(hfile, ".h"))
            continue;
        if (relative)
            hfile = "../" + hfile;
        getDeps(hfile, depfiles);
    }
}

static void compilefiles(std::ostream &fout, const std::vector<std::string> &files, const std::string &args)
{
    for (const std::string &file : files) {
        const bool external(startsWith(file,"externals/") || startsWith(file,"../externals/"));
        const bool tinyxml2(startsWith(file,"externals/tinyxml2/") || startsWith(file,"../externals/tinyxml2/"));
        fout << objfile(file) << ": " << file;
        std::vector<std::string> depfiles;
        getDeps(file, depfiles);
        std::sort(depfiles.begin(), depfiles.end());
        for (const std::string &depfile : depfiles)
            fout << " " << depfile;
        std::string additional;
        if (external)
            additional += " -w"; // do not show any warnings for external
        if (tinyxml2)
            additional += " -D_LARGEFILE_SOURCE"; // required for fseeko() and ftello() (on Cygwin)
        fout << "\n\t$(CXX) " << args << " $(CPPFLAGS) $(CXXFLAGS)" << additional << " -c -o $@ " << builddir(file) << "\n\n";
    }
}

static std::string getCppFiles(std::vector<std::string> &files, const std::string &path, bool recursive)
{
    std::list<FileWithDetails> filelist;
    const std::set<std::string> extra;
    const std::vector<std::string> masks;
    const PathMatch matcher(masks, Path::getCurrentPath());
    std::string err = FileLister::addFiles(filelist, path, extra, recursive, matcher);
    if (!err.empty())
        return err;

    // add *.cpp files to the "files" vector..
    for (const auto& file : filelist) {
        if (endsWith(file.path(), ".cpp"))
            files.push_back(file.path());
    }
    return "";
}


static void makeConditionalVariable(std::ostream &os, const std::string &variable, const std::string &defaultValue)
{
    os << "ifndef " << variable << '\n'
       << "    " << variable << '=' << defaultValue << '\n'
       << "endif\n"
       << "\n";
}

static int write_vcxproj(const std::string &proj_name, const std::function<void(std::string&)> &source_f, const std::function<void(std::string&)> &header_f)
{
    std::string outstr;

    {
        // treat as binary to prevent implicit line ending conversions
        std::ifstream in(proj_name, std::ios::binary);
        if (!in.is_open()) {
            std::cerr << "Could not open " << proj_name << std::endl;
            return EXIT_FAILURE;
        }

        std::string line;
        bool in_itemgroup = false;
        while (std::getline(in, line)) {
            if (in_itemgroup) {
                if (line.find("</ItemGroup>") == std::string::npos)
                    continue;
                in_itemgroup = false;
            }

            // strip all remaining line endings
            const std::string::size_type pos = line.find_last_not_of("\r\n");
            if (pos != std::string::npos)
                line.resize(pos+1, '\0');

            outstr += line;
            outstr += "\r\n";

            if (line.find("<ItemGroup Label=\"SourceFiles\">") != std::string::npos) {
                in_itemgroup = true;

                source_f(outstr);
            }

            if (line.find("<ItemGroup Label=\"HeaderFiles\">") != std::string::npos) {
                in_itemgroup = true;

                header_f(outstr);
            }
        }
    }

    // strip trailing \r\n
    {
        const std::string::size_type pos = outstr.find_last_not_of("\r\n");
        if (pos != std::string::npos)
            outstr.resize(pos+1, '\0');
    }

    // treat as binary to prevent implicit line ending conversions
    std::ofstream out(proj_name, std::ios::binary|std::ios::trunc);
    out << outstr;

    return EXIT_SUCCESS;
}

enum ClType : std::uint8_t { Compile, Include, Precompile, NoPch };

static std::string make_vcxproj_cl_entry(const std::string& file, ClType type)
{
    std::string outstr;
    if (type == Precompile) {
        outstr += R"(    <ClCompile Include=")";
        outstr += file;
        outstr += R"(">)";
        outstr += "\r\n";
        outstr += R"(      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release|x64'">Create</PrecompiledHeader>)";
        outstr += "\r\n";
        outstr += R"(      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Create</PrecompiledHeader>)";
        outstr += "\r\n";
        outstr += R"(      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Release-PCRE|x64'">Create</PrecompiledHeader>)";
        outstr += "\r\n";
        outstr += R"(      <PrecompiledHeader Condition="'$(Configuration)|$(Platform)'=='Debug-PCRE|x64'">Create</PrecompiledHeader>)";
        outstr += "\r\n";
        outstr += "    </ClCompile>\r\n";
        return outstr;
    }
    if (type == NoPch) {
        outstr += R"(    <ClCompile Include=")";
        outstr += file;
        outstr += R"(">)";
        outstr += "\r\n";
        outstr += R"(      <PrecompiledHeader>NotUsing</PrecompiledHeader>)";
        outstr += "\r\n";
        outstr += R"(      <ForcedIncludeFiles></ForcedIncludeFiles>)";
        outstr += "\r\n";
        outstr += "    </ClCompile>\r\n";
        return outstr;
    }
    outstr += "    <";
    outstr += (type == Compile) ? "ClCompile" : "ClInclude";
    outstr += R"( Include=")";
    outstr += file;
    outstr += R"(" />)";
    outstr += "\r\n";
    return outstr;
}

static std::vector<std::string> prioritizelib(const std::vector<std::string>& libfiles)
{
    std::map<std::string, std::size_t> priorities;
    std::size_t prio = libfiles.size();
    for (const auto &l : libfiles) {
        priorities.emplace(l, prio--);
    }
    priorities["lib/valueflow.cpp"] = 1000;
    priorities["lib/tokenize.cpp"] = 900;
    priorities["lib/symboldatabase.cpp"] = 800;
    std::vector<std::string> libfiles_prio = libfiles;
    std::sort(libfiles_prio.begin(), libfiles_prio.end(), [&](const std::string &l1, const std::string &l2) {
        const auto p1 = priorities.find(l1);
        const auto p2 = priorities.find(l2);
        return (p1 != priorities.end() ? p1->second : 0) > (p2 != priorities.end() ? p2->second : 0);
    });
    return libfiles_prio;
}

static void makeMatchcompiler(std::ostream& fout, const std::string& toolsPrefix, std::string args)
{
    if (!args.empty())
        args = " " + args;

    // avoid undefined variable
    fout << "ifndef MATCHCOMPILER\n"
         << "    MATCHCOMPILER=\n"
         << "endif\n";
    // TODO: bail out when matchcompiler.py fails (i.e. invalid PYTHON_INTERPRETER specified)
    // TODO: handle "PYTHON_INTERPRETER="
    // use match compiler..
    fout << "# use match compiler\n";
    fout << "ifeq ($(MATCHCOMPILER),yes)\n"
         << "    # Find available Python interpreter\n"
         << "    ifeq ($(PYTHON_INTERPRETER),)\n"
         << "        PYTHON_INTERPRETER := $(shell which python3)\n"
         << "    endif\n"
         << "    ifeq ($(PYTHON_INTERPRETER),)\n"
         << "        PYTHON_INTERPRETER := $(shell which python)\n"
         << "    endif\n"
         << "    ifeq ($(PYTHON_INTERPRETER),)\n"
         << "        $(error Did not find a Python interpreter)\n"
         << "    endif\n"
         << "    ifdef VERIFY\n"
         << "        matchcompiler_S := $(shell $(PYTHON_INTERPRETER) " << toolsPrefix << "tools/matchcompiler.py" << args << " --verify)\n"
         << "    else\n"
         << "        matchcompiler_S := $(shell $(PYTHON_INTERPRETER) " << toolsPrefix << "tools/matchcompiler.py" << args << ")\n"
         << "    endif\n"
         << "    libcppdir:=build\n"
         << "else ifeq ($(MATCHCOMPILER),)\n"
         << "    libcppdir:=lib\n"
         << "else\n"
         << "    $(error invalid MATCHCOMPILER value '$(MATCHCOMPILER)')\n"
         << "endif\n\n";
}

static void write_ossfuzz_makefile(std::vector<std::string> libfiles_prio, std::vector<std::string> extfiles)
{
    for (auto& l : libfiles_prio)
    {
        l = "../" + l;
    }

    for (auto& e : extfiles)
    {
        e = "../" + e;
    }

    std::ofstream fout("oss-fuzz/Makefile");

    fout << "# This file is generated by dmake, do not edit.\n";
    fout << '\n';
    fout << "# make CXX=clang++ MATCHCOMPILER=yes CXXOPTS=\"-O1 -fno-omit-frame-pointer -gline-tables-only -fsanitize=address -fsanitize-address-use-after-scope\" CPPOPTS=\"-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION -DHAVE_BOOST\" LIB_FUZZING_ENGINE=\"-fsanitize=fuzzer\" oss-fuzz-client\n";
    fout << '\n';
    fout << "MATCHCOMPILER=yes\n"; // always need to enable the matchcompiler so the library files are being copied
    makeMatchcompiler(fout, "../", "--read-dir ../lib");

    fout << "INCS=-I../lib -isystem../externals/simplecpp -isystem../externals/tinyxml2 -isystem../externals/picojson\n";
    fout << '\n';
    fout << "override CXXFLAGS+=-std=c++11 -g -w\n";
    fout << "override CPPFLAGS+=$(INCS)\n";
    fout << '\n';
    fout << "override CXXFLAGS += $(CXXOPTS)\n";
    fout << "override CPPFLAGS += $(CPPOPTS)\n";
    fout << "override LDFLAGS += $(LDOPTS)\n";
    fout << '\n';
    fout << "LIBOBJ =      " << objfiles(libfiles_prio) << "\n";
    fout << '\n';
    fout << "EXTOBJ =      " << objfiles(extfiles) << "\n";
    fout << '\n';
    fout << "oss-fuzz-client: $(EXTOBJ) $(LIBOBJ) main.o type2.o\n";
    fout << "\t${CXX} ${CXXFLAGS} -o $@ $^ ${LIB_FUZZING_ENGINE}\n";
    fout << '\n';
    fout << "no-fuzz: $(EXTOBJ) $(LIBOBJ) main_nofuzz.o type2.o\n";
    fout << "\t${CXX} ${CXXFLAGS} -o $@ $^\n";
    fout << '\n';
    fout << "translate: translate.o type2.o\n";
    fout << "\t${CXX} -std=c++11 -g ${CXXFLAGS} -o $@ type2.cpp translate.cpp\n";
    fout << '\n';
    fout << "clean:\n";
    fout << "\trm -f *.o build/*.o oss-fuzz-client no-fuzz translate\n";
    fout << '\n';
    fout << "preprare-samples:\n";
    fout << "\trm -rf samples\n";
    fout << "\tmkdir -p samples\n";
    fout << "\tcp -R ../samples .\n";
    fout << "\tfind ./samples -type f -name '*.txt' -exec rm -vf {} \\;\n";
    fout << '\n';
    fout << "do-fuzz: oss-fuzz-client preprare-samples\n";
    fout << "\tmkdir -p corpus\n";
    fout << "\t./oss-fuzz-client -only_ascii=1 -timeout=5 -detect_leaks=0 corpus samples ../test/cli/fuzz-crash ../test/cli/fuzz-crash_c ../test/cli/fuzz-timeout\n";
    fout << '\n';
    fout << "dedup-corpus: oss-fuzz-client preprare-samples\n";
    fout << "\tmv corpus corpus_\n";
    fout << "\tmkdir -p corpus\n";
    fout << "\t./oss-fuzz-client -only_ascii=1 -timeout=5 -detect_leaks=0 corpus corpus_ samples ../test/cli/fuzz-crash ../test/cli/fuzz-crash_c ../test/cli/fuzz-timeout -merge=1\n";
    fout << '\n';
    fout << "# jobs:\n";
    fout << "# ./oss-fuzz-client -only_ascii=1 -timeout=5 -detect_leaks=0 corpus samples ../test/cli/fuzz-crash ../test/cli/fuzz-crash_c ../test/cli/fuzz-timeout -workers=12 -jobs=9\n";
    fout << '\n';
    fout << "# minimize:\n";
    fout << "# ./oss-fuzz-client -only_ascii=1 -timeout=5 -detect_leaks=0 -minimize_crash=1 crash-0123456789abcdef\n";
    fout << '\n';

    compilefiles(fout, extfiles, "${LIB_FUZZING_ENGINE}");
    compilefiles(fout, libfiles_prio, "${LIB_FUZZING_ENGINE}");

    fout << '\n';
    fout << "type2.o: type2.cpp type2.h\n";
    fout << "\t$(CXX) ${LIB_FUZZING_ENGINE} $(CPPFLAGS) $(CXXFLAGS) -c -o $@ type2.cpp\n";
    fout << '\n';
    fout << "translate.o: translate.cpp type2.h\n";
    fout << "\t$(CXX) ${LIB_FUZZING_ENGINE} $(CPPFLAGS) $(CXXFLAGS) -c -o $@ translate.cpp\n";
    fout << '\n';
    fout << "main.o: main.cpp type2.h\n";
    fout << "\t$(CXX) ${LIB_FUZZING_ENGINE} $(CPPFLAGS) $(CXXFLAGS) -c -o $@ main.cpp\n";
    fout << '\n';
    fout << "main_nofuzz.o: main.cpp type2.h\n";
    fout << "\t$(CXX) ${LIB_FUZZING_ENGINE} $(CPPFLAGS) $(CXXFLAGS) -DNO_FUZZ -c -o $@ main.cpp\n";
}

int main(int argc, char **argv)
{
    const bool release(argc >= 2 && std::string(argv[1]) == "--release");

    // Get files..
    std::vector<std::string> libfiles;
    std::string err = getCppFiles(libfiles, "lib/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }
    const std::vector<std::string> libfiles_prio = prioritizelib(libfiles);

    std::vector<std::string> extfiles;
    err = getCppFiles(extfiles, "externals/", true);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> frontendfiles;
    err = getCppFiles(frontendfiles, "frontend/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> clifiles;
    err = getCppFiles(clifiles, "cli/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> testfiles;
    err = getCppFiles(testfiles, "test/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::string> toolsfiles;
    err = getCppFiles(toolsfiles, "tools/dmake/", false);
    if (!err.empty()) {
        std::cerr << err << std::endl;
        return EXIT_FAILURE;
    }

    if (libfiles.empty() && frontendfiles.empty() && clifiles.empty() && testfiles.empty()) {
        std::cerr << "No files found. Are you in the correct directory?" << std::endl;
        return EXIT_FAILURE;
    }

    // TODO: add files without source via parsing
    std::set<std::string> libfiles_h;
    for (const std::string &libfile : libfiles) {
        std::string fname(libfile.substr(4));
        fname.erase(fname.find(".cpp"));
        libfiles_h.emplace(fname + ".h");
    }
    libfiles_h.emplace("analyzer.h");
    libfiles_h.emplace("calculate.h");
    libfiles_h.emplace("config.h");
    libfiles_h.emplace("filesettings.h");
    libfiles_h.emplace("findtoken.h");
    libfiles_h.emplace("json.h");
    libfiles_h.emplace("matchcompiler.h");
    libfiles_h.emplace("precompiled.h");
    libfiles_h.emplace("smallvector.h");
    libfiles_h.emplace("sourcelocation.h");
    libfiles_h.emplace("tokenrange.h");
    libfiles_h.emplace("valueptr.h");
    libfiles_h.emplace("version.h");
    libfiles_h.emplace("vf_analyze.h");
    libfiles_h.emplace("xml.h");

    std::vector<std::string> frontendfiles_h;
    for (const std::string &frontendfile : frontendfiles) {
        std::string fname(frontendfile.substr(9));
        fname.erase(fname.find(".cpp"));
        frontendfiles_h.emplace_back(fname + ".h");
    }

    std::vector<std::string> clifiles_h;
    for (const std::string &clifile : clifiles) {
        std::string fname(clifile.substr(4));
        if (fname == "main.cpp")
            continue;
        fname.erase(fname.find(".cpp"));
        clifiles_h.emplace_back(fname + ".h");
    }

    std::set<std::string> testfiles_h;
    testfiles_h.emplace("fixture.h");
    testfiles_h.emplace("helpers.h");
    testfiles_h.emplace("options.h");
    testfiles_h.emplace("precompiled.h");
    testfiles_h.emplace("redirect.h");

    // TODO: write filter files
    // Visual Studio projects
    write_vcxproj("cli/cli.vcxproj", [&](std::string &outstr){
        for (const std::string &clifile: clifiles) {
            const std::string c = clifile.substr(4);
            outstr += make_vcxproj_cl_entry(c, c == "executor.cpp" ? Precompile : Compile);
        }
        for (const std::string &frontendfile: frontendfiles) {
            const std::string c = "..\\frontend\\" + frontendfile.substr(9);
            outstr += make_vcxproj_cl_entry(c, Compile);
        }
    }, [&](std::string &outstr){
        for (const std::string &clifile_h: clifiles_h) {
            outstr += make_vcxproj_cl_entry(clifile_h, Include);
        }
        for (const std::string &frontendfile_h: frontendfiles_h) {
            const std::string h = "..\\frontend\\" + frontendfile_h;
            outstr += make_vcxproj_cl_entry(h, Include);
        }
    });

    write_vcxproj("lib/cppcheck.vcxproj", [&](std::string &outstr){
        outstr += make_vcxproj_cl_entry(R"(..\externals\simplecpp\simplecpp.cpp)", NoPch);
        outstr += make_vcxproj_cl_entry(R"(..\externals\tinyxml2\tinyxml2.cpp)", NoPch);

        for (const std::string &libfile: libfiles_prio) {
            const std::string l = libfile.substr(4);
            outstr += make_vcxproj_cl_entry(l, l == "check.cpp" ? Precompile : Compile);
        }
    }, [&](std::string &outstr){
        outstr += make_vcxproj_cl_entry(R"(..\externals\simplecpp\simplecpp.h)", Include);
        outstr += make_vcxproj_cl_entry(R"(..\externals\tinyxml2\tinyxml2.h)", Include);

        for (const std::string &libfile_h: libfiles_h) {
            outstr += make_vcxproj_cl_entry(libfile_h, Include);
        }
    });

    write_vcxproj("test/testrunner.vcxproj", [&](std::string &outstr){
        for (const std::string &clifile: clifiles) {
            if (clifile == "cli/main.cpp")
                continue;
            const std::string c = R"(..\cli\)" + clifile.substr(4);
            outstr += make_vcxproj_cl_entry(c, Compile);
        }

        for (const std::string &frontendfile: frontendfiles) {
            const std::string c = "..\\frontend\\" + frontendfile.substr(9);
            outstr += make_vcxproj_cl_entry(c, Compile);
        }

        for (const std::string &testfile: testfiles) {
            const std::string t = testfile.substr(5);
            outstr += make_vcxproj_cl_entry(t, t == "fixture.cpp" ? Precompile : Compile);
        }
    }, [&](std::string &outstr){
        for (const std::string &clifile_h: clifiles_h) {
            const std::string c = R"(..\cli\)" + clifile_h;
            outstr += make_vcxproj_cl_entry(c, Include);
        }

        for (const std::string &frontendfile_h: frontendfiles_h) {
            const std::string h = "..\\frontend\\" + frontendfile_h;
            outstr += make_vcxproj_cl_entry(h, Include);
        }

        for (const std::string &testfile_h: testfiles_h) {
            outstr += make_vcxproj_cl_entry(testfile_h, Include);
        }
    });

    static constexpr char makefile[] = "Makefile";
    std::ofstream fout(makefile, std::ios_base::trunc);
    if (!fout.is_open()) {
        std::cerr << "An error occurred while trying to open "
                  << makefile
                  << ".\n";
        return EXIT_FAILURE;
    }

    fout << "# This file is generated by dmake, do not edit.\n\n";

    fout << "ifndef VERBOSE\n"
         << "    VERBOSE=\n"
         << "endif\n";

    fout << "# To compile with rules, use 'make HAVE_RULES=yes'\n";
    makeConditionalVariable(fout, "HAVE_RULES", "");

    makeMatchcompiler(fout, "", "");

    // avoid undefined variable
    fout << "ifndef CPPFLAGS\n"
         << "    CPPFLAGS=\n"
         << "endif\n\n";

    // explicit files dir..
    fout << "ifdef FILESDIR\n"
         << "    override CPPFLAGS+=-DFILESDIR=\\\"$(FILESDIR)\\\"\n"
         << "endif\n\n";

    // enable backtrac
    fout << "RDYNAMIC?=-rdynamic\n";

    // The _GLIBCXX_DEBUG doesn't work in cygwin or other Win32 systems.
    fout << "# Set the CPPCHK_GLIBCXX_DEBUG flag. This flag is not used in release Makefiles.\n"
         << "# The _GLIBCXX_DEBUG define doesn't work in Cygwin or other Win32 systems.\n"
         << "ifndef COMSPEC\n"
         << "    ifeq ($(VERBOSE),1)\n"
         << "        $(info COMSPEC not found)\n"
         << "    endif\n"
         << "    ifdef ComSpec\n"
         << "        ifeq ($(VERBOSE),1)\n"
         << "            $(info ComSpec found)\n"
         << "        endif\n"
         << "        #### ComSpec is defined on some WIN32's.\n"
         << "        WINNT=1\n"
         << "\n"
         << "        ifeq ($(VERBOSE),1)\n"
         << "            $(info PATH=$(PATH))\n"
         << "        endif\n"
         << "\n"
         << "        ifneq (,$(findstring /cygdrive/,$(PATH)))\n"
         << "            ifeq ($(VERBOSE),1)\n"
         << "                $(info /cygdrive/ found in PATH)\n"
         << "            endif\n"
         << "            CYGWIN=1\n"
         << "        endif # CYGWIN\n"
         << "    endif # ComSpec\n"
         << "endif # COMSPEC\n"
         << "\n"
         << "ifdef WINNT\n"
         << "    ifeq ($(VERBOSE),1)\n"
         << "        $(info WINNT found)\n"
         << "    endif\n"
         << "    #### Maybe Windows\n"
         << "    ifndef CPPCHK_GLIBCXX_DEBUG\n"
         << "        CPPCHK_GLIBCXX_DEBUG=\n"
         << "    endif # !CPPCHK_GLIBCXX_DEBUG\n"
         << "\n"
         << "    ifeq ($(VERBOSE),1)\n"
         << "        $(info MSYSTEM=$(MSYSTEM))\n"
         << "    endif\n"
         << "\n"
         << "    ifneq ($(MSYSTEM),MINGW32 MINGW64)\n"
         << "        RDYNAMIC=\n"
         << "    endif\n"
         << "\n"
         << "    LDFLAGS+=-lshlwapi\n"
         << "else # !WINNT\n"
         << "    ifeq ($(VERBOSE),1)\n"
         << "        $(info WINNT not found)\n"
         << "    endif\n"
         << "\n"
         << "    uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')\n"
         << "\n"
         << "    ifeq ($(VERBOSE),1)\n"
         << "        $(info uname_S=$(uname_S))\n"
         << "    endif\n"
         << "\n"
         << "    ifeq ($(uname_S),Linux)\n"
         << "        ifndef CPPCHK_GLIBCXX_DEBUG\n"
         << "            CPPCHK_GLIBCXX_DEBUG=-D_GLIBCXX_DEBUG\n"
         << "        endif # !CPPCHK_GLIBCXX_DEBUG\n"
         << "    endif # Linux\n"
         << "\n"
         << "    ifeq ($(uname_S),GNU/kFreeBSD)\n"
         << "        ifndef CPPCHK_GLIBCXX_DEBUG\n"
         << "            CPPCHK_GLIBCXX_DEBUG=-D_GLIBCXX_DEBUG\n"
         << "        endif # !CPPCHK_GLIBCXX_DEBUG\n"
         << "    endif # GNU/kFreeBSD\n"
         << "\n"
         << "    LDFLAGS+=-pthread\n"
         << "\n"
         << "endif # WINNT\n"
         << "\n";

    fout << "ifdef CYGWIN\n"
         << "    ifeq ($(VERBOSE),1)\n"
         << "        $(info CYGWIN found)\n"
         << "    endif\n"
         << "\n"
         << "    # Increase stack size for Cygwin builds to avoid segmentation fault in limited recursive tests.\n"
         << "    CXXFLAGS+=-Wl,--stack,8388608\n"
         << "endif # CYGWIN\n"
         << "\n";

    // skip "-D_GLIBCXX_DEBUG" if clang, since it breaks the build
    makeConditionalVariable(fout, "CXX", "g++");
    fout << "ifeq (clang++, $(findstring clang++,$(CXX)))\n"
         << "    CPPCHK_GLIBCXX_DEBUG=\n"
         << "endif\n";

    // Makefile settings..
    if (release) {
        makeConditionalVariable(fout, "CXXFLAGS", "-O2 -Wall -Wno-sign-compare -Wno-multichar");
        fout << "override CPPFLAGS += -DNDEBUG\n\n";
    } else {
        makeConditionalVariable(fout, "CXXFLAGS",
                                "-pedantic "
                                "-Wall "
                                "-Wextra "
                                "-Wcast-qual "
                                "-Wfloat-equal "
                                "-Wmissing-declarations "
                                "-Wmissing-format-attribute "
                                "-Wno-long-long "
                                "-Wpacked "
                                "-Wredundant-decls "
                                "-Wundef "
                                "-Wno-sign-compare "
                                "-Wno-multichar "
                                "-Woverloaded-virtual "
                                "-g");
        //fout << "override CPPFLAGS += $(CPPCHK_GLIBCXX_DEBUG)\n\n"; // TODO: when using CXXOPTS this would always be set - need to handle this differently
    }

    fout << "ifeq (g++, $(findstring g++,$(CXX)))\n"
         << "    override CXXFLAGS += -pipe\n"
         << "endif\n"
         << "override CXXFLAGS += -std=c++11"
         << "\n";

    fout << "ifeq ($(HAVE_RULES),yes)\n"
         << "    PCRE_CONFIG = $(shell which pcre-config)\n"
         << "    ifeq ($(PCRE_CONFIG),)\n"
         << "        $(error Did not find pcre-config)\n"
         << "    endif\n"
         << "    override CXXFLAGS += $(shell $(PCRE_CONFIG) --cflags)\n"
         << "    override CPPFLAGS += -DHAVE_RULES\n"
         << "    ifdef LIBS\n"
         << "        LIBS += $(shell $(PCRE_CONFIG) --libs)\n"
         << "    else\n"
         << "        LIBS=$(shell $(PCRE_CONFIG) --libs)\n"
         << "    endif\n"
         << "else ifneq ($(HAVE_RULES),)\n"
         << "    $(error invalid HAVE_RULES value '$(HAVE_RULES)')\n"
         << "endif\n\n";

    fout << "# older make versions do not support # in $(shell) and newer ones handle the escape sequence literally\n"
         << "REQUIRE_ESCAPE=$(shell echo \"\\#define DEF\" | $(CXX) -c -xc - 2> /dev/null && echo \"1\" || echo \"0\")\n"
         << "ifeq ($(REQUIRE_ESCAPE),1)\n"
         << "    HAVE_EXECINFO_H=$(shell echo \"\\#include <execinfo.h>\" | $(CXX) -c -xc - 2> /dev/null && echo \"1\" || echo \"0\")\n"
         << "else\n"
         << "    HAVE_EXECINFO_H=$(shell echo \"#include <execinfo.h>\" | $(CXX) -c -xc - 2> /dev/null && echo \"1\" || echo \"0\")\n"
         << "endif\n"
         << "override CPPFLAGS += -DHAVE_EXECINFO_H=$(HAVE_EXECINFO_H)\n\n";

    fout << "override CXXFLAGS += $(CXXOPTS)\n";
    fout << "override CPPFLAGS += $(CPPOPTS)\n";
    fout << "override LDFLAGS += $(LDOPTS)\n\n";

    makeConditionalVariable(fout, "PREFIX", "/usr");
    makeConditionalVariable(fout, "INCLUDE_FOR_LIB", "-Ilib -isystem externals -isystem externals/picojson -isystem externals/simplecpp -isystem externals/tinyxml2");
    makeConditionalVariable(fout, "INCLUDE_FOR_FE", "-Ilib");
    makeConditionalVariable(fout, "INCLUDE_FOR_CLI", "-Ilib -Ifrontend -isystem externals/picojson -isystem externals/simplecpp -isystem externals/tinyxml2");
    makeConditionalVariable(fout, "INCLUDE_FOR_TEST", "-Ilib -Ifrontend -Icli -isystem externals/picojson -isystem externals/simplecpp -isystem externals/tinyxml2");

    makeConditionalVariable(fout, "CFLAGS_FOR_TEST", "-Wno-dollar-in-identifier-extension");

    fout << "BIN=$(DESTDIR)$(PREFIX)/bin\n\n";
    fout << "# For 'make man': sudo apt-get install xsltproc docbook-xsl docbook-xml on Linux\n";
    fout << "DB2MAN?=/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/manpages/docbook.xsl\n";
    fout << "XP=xsltproc -''-nonet -''-param man.charmap.use.subset \"0\"\n";
    fout << "MAN_SOURCE=man/cppcheck.1.xml\n\n";

    fout << "\n###### Object Files\n\n";
    fout << "LIBOBJ =      " << objfiles(libfiles_prio) << "\n\n";
    fout << "EXTOBJ =      " << objfiles(extfiles) << "\n\n";
    fout << "FEOBJ =       " << objfiles(frontendfiles) << "\n\n";
    fout << "CLIOBJ =      " << objfiles(clifiles) << "\n\n";
    fout << "TESTOBJ =     " << objfiles(testfiles) << "\n\n";

    fout << ".PHONY: run-dmake tags\n\n";
    fout << "\n###### Targets\n\n";
    fout << "cppcheck: $(EXTOBJ) $(LIBOBJ) $(FEOBJ) $(CLIOBJ)\n";
    fout << "\t$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS) $(LDFLAGS) $(RDYNAMIC)\n\n";
    fout << "all:\tcppcheck testrunner\n\n";
    std::string testrunner_clifiles_o;
    for (const std::string &clifile: clifiles) {
        if (clifile == "cli/main.cpp")
            continue;
        testrunner_clifiles_o += ' ';
        const std::string o = clifile.substr(0, clifile.length()-3) + 'o';
        testrunner_clifiles_o += o;
    }
    fout << "testrunner: $(EXTOBJ) $(TESTOBJ) $(LIBOBJ) $(FEOBJ)" << testrunner_clifiles_o << "\n";
    fout << "\t$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS) $(LDFLAGS) $(RDYNAMIC)\n\n";
    fout << "test:\tall\n";
    fout << "\t./testrunner\n\n";
    fout << "check:\tall\n";
    fout << "\t./testrunner -q\n\n";
    fout << "checkcfg:\tcppcheck validateCFG\n";
    fout << "\t./test/cfg/runtests.sh\n\n";
    fout << "dmake:\ttools/dmake/dmake.o cli/filelister.o $(libcppdir)/pathmatch.o $(libcppdir)/path.o $(libcppdir)/utils.o externals/simplecpp/simplecpp.o\n";
    fout << "\t$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)\n\n";
    fout << "run-dmake: dmake\n";
    fout << "\t./dmake" << (release ? " --release" : "") << "\n\n"; // Make CI in release builds happy
    fout << "clean:\n";
    fout << "\trm -f build/*.cpp build/*.o lib/*.o cli/*.o frontend/*.o test/*.o tools/dmake/*.o externals/*/*.o testrunner dmake cppcheck cppcheck.exe cppcheck.1\n\n";
    fout << "man:\tman/cppcheck.1\n\n";
    fout << "man/cppcheck.1:\t$(MAN_SOURCE)\n\n";
    fout << "\t$(XP) $(DB2MAN) $(MAN_SOURCE)\n\n";
    fout << "tags:\n";
    fout << "\tctags -R --exclude=doxyoutput --exclude=test/cfg cli externals gui lib test\n\n";
    fout << "install: cppcheck\n";
    fout << "\tinstall -d ${BIN}\n";
    fout << "\tinstall cppcheck ${BIN}\n";
    fout << "\tinstall htmlreport/cppcheck-htmlreport ${BIN}\n";
    fout << "ifdef FILESDIR\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}/addons\n";
    fout << "\tinstall -m 644 addons/*.json ${DESTDIR}${FILESDIR}/addons\n";
    fout << "\tinstall -m 644 addons/*.py ${DESTDIR}${FILESDIR}/addons\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}/cfg\n";
    fout << "\tinstall -m 644 cfg/*.cfg ${DESTDIR}${FILESDIR}/cfg\n";
    fout << "\tinstall -d ${DESTDIR}${FILESDIR}/platforms\n";
    fout << "\tinstall -m 644 platforms/*.xml ${DESTDIR}${FILESDIR}/platforms\n";
    fout << "else\n";
    fout << "\t$(error FILESDIR must be set!)\n";
    fout << "endif\n";
    fout << "\n";
    fout << "uninstall:\n";
    fout << "\t@if test -d ${BIN}; then \\\n";
    fout << "\t  files=\"cppcheck cppcheck-htmlreport\"; \\\n";
    fout << "\t  echo '(' cd ${BIN} '&&' rm -f $$files ')'; \\\n";
    fout << "\t  ( cd ${BIN} && rm -f $$files ); \\\n";
    fout << "\tfi\n";
    fout << "ifdef FILESDIR \n";
    fout << "\t@if test -d ${DESTDIR}${FILESDIR}; then \\\n";
    fout << "\t  echo rm -rf ${DESTDIR}${FILESDIR}; \\\n";
    fout << "\t  rm -rf ${DESTDIR}${FILESDIR}; \\\n";
    fout << "\tfi\n";
    fout << "endif\n";
    fout << "# Validation of library files:\n";
    fout << "ConfigFiles := $(wildcard cfg/*.cfg)\n";
    fout << "ConfigFilesCHECKED := $(patsubst %.cfg,%.checked,$(ConfigFiles))\n";
    fout << ".PHONY: validateCFG\n";
    fout << "%.checked:%.cfg\n";
    fout << "\txmllint --noout --relaxng cfg/cppcheck-cfg.rng $<\n";
    fout << "validateCFG: ${ConfigFilesCHECKED}\n";
    fout << "\txmllint --noout cfg/cppcheck-cfg.rng\n\n";
    fout << "# Validation of platforms files:\n";
    fout << "PlatformFiles := $(wildcard platforms/*.xml)\n";
    fout << "PlatformFilesCHECKED := $(patsubst %.xml,%.checked,$(PlatformFiles))\n";
    fout << ".PHONY: validatePlatforms\n";
    fout << "%.checked:%.xml\n";
    fout << "\txmllint --noout --relaxng platforms/cppcheck-platforms.rng $<\n";
    fout << "validatePlatforms: ${PlatformFilesCHECKED}\n";
    fout << "\txmllint --noout platforms/cppcheck-platforms.rng\n";
    fout << "\n";
    fout << "# Validate XML output (to detect regressions)\n";
    fout << "/tmp/errorlist.xml: cppcheck\n";
    fout << "\t./cppcheck --errorlist >$@\n";
    fout << "/tmp/example.xml: cppcheck\n";
    fout << "\t./cppcheck --xml --enable=all --inconclusive --max-configs=1 samples 2>/tmp/example.xml\n";
    fout << "createXMLExamples:/tmp/errorlist.xml /tmp/example.xml\n";
    fout << ".PHONY: validateXML\n";
    fout << "validateXML: createXMLExamples\n";
    fout << "\txmllint --noout cppcheck-errors.rng\n";
    fout << "\txmllint --noout --relaxng cppcheck-errors.rng /tmp/errorlist.xml\n";
    fout << "\txmllint --noout --relaxng cppcheck-errors.rng /tmp/example.xml\n";
    fout << "\ncheckCWEEntries: /tmp/errorlist.xml\n";
    // TODO: handle "PYTHON_INTERPRETER="
    fout << "\t$(eval PYTHON_INTERPRETER := $(if $(PYTHON_INTERPRETER),$(PYTHON_INTERPRETER),$(shell which python3)))\n";
    fout << "\t$(eval PYTHON_INTERPRETER := $(if $(PYTHON_INTERPRETER),$(PYTHON_INTERPRETER),$(shell which python)))\n";
    fout << "\t$(eval PYTHON_INTERPRETER := $(if $(PYTHON_INTERPRETER),$(PYTHON_INTERPRETER),$(error Did not find a Python interpreter)))\n";
    fout << "\t$(PYTHON_INTERPRETER) tools/listErrorsWithoutCWE.py -F /tmp/errorlist.xml\n";
    fout << ".PHONY: validateRules\n";
    fout << "validateRules:\n";
    fout << "\txmllint --noout rules/*.xml\n";
    fout << ".PHONY: check-nonneg\n";
    fout << "check-nonneg:\n";
    // TODO: how to use provided number of jobs?
    fout << "\tls lib/*.cpp | xargs -n 1 -P $$(nproc) g++ -fsyntax-only -DNONNEG $(CXXFLAGS) $(INCLUDE_FOR_LIB)\n";

    fout << "\n###### Build\n\n";

    compilefiles(fout, libfiles_prio, "${INCLUDE_FOR_LIB}");
    compilefiles(fout, frontendfiles, "${INCLUDE_FOR_FE}");
    compilefiles(fout, clifiles, "${INCLUDE_FOR_CLI}");
    compilefiles(fout, testfiles, "${INCLUDE_FOR_TEST} ${CFLAGS_FOR_TEST}");
    compilefiles(fout, extfiles, "");
    compilefiles(fout, toolsfiles, "${INCLUDE_FOR_LIB}");

    write_ossfuzz_makefile(libfiles_prio, extfiles);

    return 0;
}
