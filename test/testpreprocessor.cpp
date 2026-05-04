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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.

#include "errortypes.h"
#include "library.h"
#include "path.h"
#include "platform.h"
#include "preprocessor.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "tokenlist.h"
#include "fixture.h"
#include "helpers.h"

#include <cstring>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <simplecpp.h>

class ErrorLogger;

class TestPreprocessor : public TestFixture {
public:
    TestPreprocessor() : TestFixture("TestPreprocessor") {}

private:
    template<size_t size>
    std::string expandMacros(const char (&code)[size], ErrorLogger &errorLogger) const {
        simplecpp::OutputList outputList;
        std::vector<std::string> files;
        simplecpp::TokenList tokens1 = simplecpp::TokenList(code, files, "file.cpp", &outputList);
        Preprocessor p(tokens1, settingsDefault, errorLogger, Path::identify(tokens1.getFiles()[0], false));
        ASSERT(p.loadFiles(files));
        simplecpp::TokenList tokens2 = p.preprocess("", files, outputList);
        (void)p.reportOutput(outputList, true);
        return tokens2.stringify();
    }

    template<size_t size>
    void preprocess(const char (&code)[size], std::vector<std::string> &files, const std::string& file0, TokenList& tokenlist, const simplecpp::DUI& dui)
    {
        if (!files.empty())
            throw std::runtime_error("file list not empty");

        if (tokenlist.front())
            throw std::runtime_error("token list not empty");

        simplecpp::OutputList outputList;
        const simplecpp::TokenList tokens1(code, files, file0, &outputList);

        // Preprocess..
        simplecpp::TokenList tokens2(files);
        simplecpp::FileDataCache cache;
        simplecpp::preprocess(tokens2, tokens1, files, cache, dui, &outputList);
        Preprocessor preprocessor(tokens2, settingsDefault, *this, Standards::Language::C);
        (void)preprocessor.reportOutput(outputList, true);

        // Tokenizer..
        tokenlist.createTokens(std::move(tokens2));
    }

    template<size_t size>
    std::vector<RemarkComment> getRemarkComments(const char (&code)[size], ErrorLogger& errorLogger) const
    {
        std::vector<std::string> files;
        simplecpp::TokenList tokens1(code, files, "test.cpp");

        const Preprocessor preprocessor(tokens1, settingsDefault, errorLogger, Path::identify(tokens1.getFiles()[0], false));
        return preprocessor.getRemarkComments();
    }

    static std::string getcodeforcfg(const Settings& settings, ErrorLogger& errorlogger, const char* code, std::size_t size, const std::string &cfg, const std::string &filename, SuppressionList *inlineSuppression = nullptr)
    {
        std::map<std::string, std::string> cfgcode = getcode(settings, errorlogger, code, size, std::set<std::string>{cfg}, filename, inlineSuppression);
        const auto it = cfgcode.find(cfg);
        if (it == cfgcode.end())
            return "";
        return it->second;
    }

    template<size_t size>
    static std::string getcodeforcfg(const Settings& settings, ErrorLogger& errorlogger, const char (&code)[size], const std::string &cfg, const std::string &filename, SuppressionList *inlineSuppression = nullptr)
    {
        return getcodeforcfg(settings, errorlogger, code, size-1, cfg, filename, inlineSuppression);
    }

    template<size_t size>
    static std::map<std::string, std::string> getcode(const Settings& settings, ErrorLogger& errorlogger, const char (&code)[size], const std::string &filename = "file.c")
    {
        return getcode(settings, errorlogger, code, size-1, {}, filename, nullptr);
    }

    static std::map<std::string, std::string> getcode(const Settings& settings, ErrorLogger& errorlogger, const char* code, std::size_t size, std::set<std::string> cfgs, const std::string &filename, SuppressionList *inlineSuppression)
    {
        simplecpp::OutputList outputList;
        std::vector<std::string> files;

        simplecpp::TokenList tokens({code, size}, files, Path::simplifyPath(filename), &outputList);

        // TODO: we should be using the actual Preprocessor implementation
        Preprocessor preprocessor(tokens, settings, errorlogger, Path::identify(tokens.getFiles()[0], false));

        // TODO: should be possible without a Preprocessor instance
        if (preprocessor.reportOutput(outputList, true))
            return {};

        if (inlineSuppression)
            preprocessor.inlineSuppressions(*inlineSuppression);
        preprocessor.removeComments();
        preprocessor.simplifyPragmaAsm();

        std::map<std::string, std::string> cfgcode;
        if (cfgs.empty())
            cfgs = preprocessor.getConfigs();
        for (const std::string & config : cfgs) {
            try {
                const bool writeLocations = (strstr(code, "#include") != nullptr);
                cfgcode[config] = preprocessor.getcode(config, files, writeLocations);
            } catch (const simplecpp::Output &) {
                cfgcode[config] = "";
            }
        }

        return cfgcode;
    }

    const Settings settings0 = settingsBuilder().severity(Severity::information).build();

    void run() override {
        mNewTemplate = true;

        // The bug that started the whole work with the new preprocessor
        TEST_CASE(Bug2190219);

        TEST_CASE(error1); // #error => don't extract any code
        TEST_CASE(error2); // #error if symbol is not defined
        TEST_CASE(error3);
        TEST_CASE(error4);  // #2919 - wrong filename is reported
        TEST_CASE(error5);
        TEST_CASE(error6);
        TEST_CASE(error7);
        TEST_CASE(error8); // #10170 -> previous #if configurations

        TEST_CASE(setPlatformInfo);

        // Handling include guards (don't create extra configuration for it)
        TEST_CASE(includeguard1);
        TEST_CASE(includeguard2);

        TEST_CASE(if0);
        TEST_CASE(if1);

        TEST_CASE(elif);

        TEST_CASE(if_cond1);
        TEST_CASE(if_cond2);
        TEST_CASE(if_cond3);
        TEST_CASE(if_cond4);
        TEST_CASE(if_cond5);
        TEST_CASE(if_cond6);
        TEST_CASE(if_cond8);
        TEST_CASE(if_cond9);
        TEST_CASE(if_cond10);
        TEST_CASE(if_cond11);
        TEST_CASE(if_cond12);
        TEST_CASE(if_cond13);
        TEST_CASE(if_cond14);

        TEST_CASE(if_or_1);
        TEST_CASE(if_or_2);

        TEST_CASE(if_macro_eq_macro); // #3536
        TEST_CASE(ticket_3675);
        TEST_CASE(ticket_3699);
        TEST_CASE(ticket_4922); // #4922

        // Macros..
        TEST_CASE(macro_simple1);
        TEST_CASE(macro_simple2);
        TEST_CASE(macro_simple3);
        TEST_CASE(macro_simple4);
        TEST_CASE(macro_simple5);
        TEST_CASE(macro_simple6);
        TEST_CASE(macro_simple7);
        TEST_CASE(macro_simple8);
        TEST_CASE(macro_simple9);
        TEST_CASE(macro_simple10);
        TEST_CASE(macro_simple11);
        TEST_CASE(macro_simple12);
        TEST_CASE(macro_simple13);
        TEST_CASE(macro_simple14);
        TEST_CASE(macro_simple15);
        TEST_CASE(macro_simple16);  // #4703: Macro parameters not trimmed
        TEST_CASE(macro_simple17);  // #5074: isExpandedMacro not set
        TEST_CASE(macro_simple18);  // (1e-7)
        TEST_CASE(macroInMacro1);
        TEST_CASE(macroInMacro2);
        TEST_CASE(macro_linenumbers);
        TEST_CASE(macro_nopar);
        TEST_CASE(macro_incdec);  // separate ++ and -- with space when expanding such macro: '#define M(X)  A-X'
        TEST_CASE(macro_switchCase);
        TEST_CASE(macro_NULL); // skip #define NULL .. it is replaced in the tokenizer
        TEST_CASE(string1);
        TEST_CASE(string2);
        TEST_CASE(string3);
        TEST_CASE(preprocessor_undef);
        TEST_CASE(defdef);  // Defined multiple times
        TEST_CASE(preprocessor_doublesharp);
        TEST_CASE(preprocessor_include_in_str);
        TEST_CASE(va_args_1);
        //TEST_CASE(va_args_2); invalid code
        TEST_CASE(va_args_3);
        TEST_CASE(va_args_4);
        TEST_CASE(va_args_5);
        TEST_CASE(multi_character_character);

        TEST_CASE(stringify);
        TEST_CASE(stringify2);
        TEST_CASE(stringify3);
        TEST_CASE(stringify4);
        TEST_CASE(stringify5);
        TEST_CASE(ifdefwithfile);
        TEST_CASE(pragma);
        TEST_CASE(pragma_asm_1);
        TEST_CASE(pragma_asm_2);
        TEST_CASE(endifsemicolon);
        TEST_CASE(missing_doublequote);
        TEST_CASE(handle_error);
        TEST_CASE(dup_defines);

        TEST_CASE(define_part_of_func);
        TEST_CASE(conditionalDefine);
        TEST_CASE(macro_parameters);
        TEST_CASE(newline_in_macro);
        TEST_CASE(ifdef_ifdefined);

        // define and then ifdef
        TEST_CASE(define_if1);
        TEST_CASE(define_if2);
        TEST_CASE(define_if3);
        TEST_CASE(define_if4); // #4079 - #define X +123
        TEST_CASE(define_if5); // #4516 - #define B (A & 0x00f0)
        TEST_CASE(define_if6); // #4863 - #define B (A?-1:1)
        TEST_CASE(define_ifdef);
        TEST_CASE(define_ifndef1);
        TEST_CASE(define_ifndef2);
        TEST_CASE(ifndef_define);
        TEST_CASE(undef_ifdef);
        TEST_CASE(endfile);

        TEST_CASE(redundant_config);

        TEST_CASE(invalid_define_1); // #2605 - hang for: '#define ='
        TEST_CASE(invalid_define_2); // #4036 - hang for: '#define () {(int f(x) }'

        // inline suppression, missingInclude/missingIncludeSystem
        TEST_CASE(inline_suppressions);
        TEST_CASE(inline_suppressions_not_next_line);

        // remark comment
        TEST_CASE(remarkComment1);
        TEST_CASE(remarkComment2);
        TEST_CASE(remarkComment3);
        TEST_CASE(remarkComment4);

        // Using -D to predefine symbols
        TEST_CASE(predefine1);
        TEST_CASE(predefine2);
        TEST_CASE(predefine3);
        TEST_CASE(predefine4);
        TEST_CASE(predefine5);  // automatically define __cplusplus
        TEST_CASE(predefine6);  // automatically define __STDC_VERSION__


        TEST_CASE(strictAnsi);

        TEST_CASE(invalidElIf); // #2942 segfault

        // Preprocessor::getConfigs
        TEST_CASE(getConfigs1);
        TEST_CASE(getConfigs2);
        TEST_CASE(getConfigs3);
        TEST_CASE(getConfigs4);
        TEST_CASE(getConfigs5);
        TEST_CASE(getConfigs7);
        TEST_CASE(getConfigs7a);
        TEST_CASE(getConfigs7b);
        TEST_CASE(getConfigs7c);
        TEST_CASE(getConfigs7d);
        TEST_CASE(getConfigs7e);
        TEST_CASE(getConfigs8);  // #if A==1  => cfg: A=1
        TEST_CASE(getConfigs10); // #5139
        TEST_CASE(getConfigs11); // #9832 - include guards
        TEST_CASE(getConfigs12); // #14222
        TEST_CASE(getConfigs13); // #14222
        TEST_CASE(getConfigs_gte); // #1059
        TEST_CASE(getConfigs_lte); // #1059
        TEST_CASE(getConfigs_gt); // #1059
        TEST_CASE(getConfigs_lt); // #1059
        TEST_CASE(getConfigs_eq_compound); // #1059
        TEST_CASE(getConfigs_gte_compound); // #1059
        TEST_CASE(getConfigs_lte_compound); // #1059
        TEST_CASE(getConfigs_gt_compound); // #1059
        TEST_CASE(getConfigs_lt_compound); // #1059
        TEST_CASE(getConfigsError);

        TEST_CASE(getConfigsD1);

        TEST_CASE(getConfigsU1);
        TEST_CASE(getConfigsU2);
        TEST_CASE(getConfigsU3);
        TEST_CASE(getConfigsU4);
        TEST_CASE(getConfigsU5);
        TEST_CASE(getConfigsU6);
        TEST_CASE(getConfigsU7);

        TEST_CASE(getConfigsAndCodeIssue14317);
        TEST_CASE(getConfigsMostGeneralConfigIssue14317);

        TEST_CASE(if_sizeof);

        TEST_CASE(invalid_ifs); // #5909

        TEST_CASE(garbage);

        TEST_CASE(wrongPathOnErrorDirective);

        TEST_CASE(testMissingInclude);
        TEST_CASE(testMissingInclude2);
        TEST_CASE(testMissingInclude3);
        TEST_CASE(testMissingInclude4);
        TEST_CASE(testMissingInclude5);
        TEST_CASE(testMissingInclude6);
        TEST_CASE(testMissingSystemInclude);
        TEST_CASE(testMissingSystemInclude2);
        TEST_CASE(testMissingSystemInclude3);
        TEST_CASE(testMissingSystemInclude4);
        TEST_CASE(testMissingSystemInclude5);
        TEST_CASE(testMissingIncludeMixed);
        TEST_CASE(testMissingIncludeCheckConfig);

        TEST_CASE(hasInclude);

        TEST_CASE(limitsDefines);

        TEST_CASE(hashCalculation);

        TEST_CASE(standard);

        TEST_CASE(writeLocations);

        TEST_CASE(pragmaAsm);
    }

    template<size_t size>
    std::string getConfigsStr(const char (&code)[size], const char *arg = nullptr, const char *library = nullptr) {
        Settings settings;
        if (arg && std::strncmp(arg,"-D",2)==0)
            settings.userDefines = arg + 2;
        if (arg && std::strncmp(arg,"-U",2)==0)
            settings.userUndefs.insert(arg+2);
        if (library)
            ASSERT(settings.library.load("", library, false).errorcode == Library::ErrorCode::OK);
        std::vector<std::string> files;
        simplecpp::OutputList outputList;
        simplecpp::TokenList tokens(code,files,"test.c",&outputList);
        Preprocessor preprocessor(tokens, settings, *this, Standards::Language::C);
        ASSERT(preprocessor.loadFiles(files));
        ASSERT(!preprocessor.reportOutput(outputList, true));
        preprocessor.removeComments();
        const std::set<std::string> configs = preprocessor.getConfigs();
        std::string ret;
        for (const std::string & config : configs)
            ret += config + '\n';
        return ret;
    }

    template<size_t size>
    std::size_t getHash(const char (&code)[size]) {
        std::vector<std::string> files;
        simplecpp::TokenList tokens(code,files,"test.c");
        Preprocessor preprocessor(tokens, settingsDefault, *this, Standards::Language::C);
        ASSERT(preprocessor.loadFiles(files));
        preprocessor.removeComments();
        return preprocessor.calculateHash("");
    }

    void Bug2190219() {
        const char filedata[] = "#ifdef __cplusplus\n"
                                "cpp\n"
                                "#else\n"
                                "c\n"
                                "#endif";

        {
            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata, "file.cpp");

            // Compare results..
            ASSERT_EQUALS(1U, actual.size());
            ASSERT_EQUALS("\ncpp", actual.at(""));
        }

        {
            // Ticket #7102 - skip __cplusplus in C code
            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata, "file.c");

            // Compare results..
            ASSERT_EQUALS(1U, actual.size());
            ASSERT_EQUALS("\n\n\nc", actual.at(""));
        }
    }

    void error1() {
        const char filedata[] = "#ifdef A\n"
                                ";\n"
                                "#else\n"
                                "#error abcd\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA=A\n", getConfigsStr(filedata));
    }

    void error2() {
        const char filedata1[] = "#ifndef A\n"
                                 "#error\n"
                                 "#endif\n";
        ASSERT_EQUALS("A\n", getConfigsStr(filedata1));

        const char filedata2[] = "#if !A\n"
                                 "#error\n"
                                 "#endif\n";
        ASSERT_EQUALS("A\n", getConfigsStr(filedata2));
    }

    void error3() {
        const auto settings = dinit(Settings, $.userDefines = "__cplusplus");
        const char code[] = "#error hello world!\n";
        (void)getcodeforcfg(settings, *this, code, "X", "test.c");
        ASSERT_EQUALS("[test.c:1:2]: (error) #error hello world! [preprocessorErrorDirective]\n", errout_str());
    }

    // Ticket #2919 - wrong filename reported for #error
    void error4() {
        // In included file
        {
            ScopedFile header("ab.h", "#error hello world!\n");
            const auto settings = dinit(Settings, $.userDefines = "TEST");
            const char code[] = "#include \"ab.h\"";
            (void)getcodeforcfg(settings, *this, code, "TEST", "test.c");
            ASSERT_EQUALS("[ab.h:1:2]: (error) #error hello world! [preprocessorErrorDirective]\n", errout_str());
        }

        // After including a file
        {
            ScopedFile header("ab.h", "");
            const auto settings = dinit(Settings, $.userDefines = "TEST");
            const char code[] = "#include \"ab.h\"\n"
                                "#error aaa";
            (void)getcodeforcfg(settings, *this, code, "TEST", "test.c");
            ASSERT_EQUALS("[test.c:2:2]: (error) #error aaa [preprocessorErrorDirective]\n", errout_str());
        }
    }

    void error5() {
        // No message if --force is given
        const auto settings = dinit(Settings,
                                    $.userDefines = "TEST",
                                        $.force = true);
        const char code[] = "#error hello world!\n";
        (void)getcodeforcfg(settings, *this, code, "X", "test.c");
        ASSERT_EQUALS("", errout_str());
    }

    void error6() {
        const char filedata1[] = "#ifdef A\n"
                                 "#else\n"
                                 "#error 1\n"
                                 "#endif\n"
                                 "#ifdef B\n"
                                 "#else\n"
                                 "#error 2\n"
                                 "#endif\n";
        ASSERT_EQUALS("\nA=A\nA=A;B=B\nB=B\n", getConfigsStr(filedata1));

        const char filedata2[] = "#ifndef A\n"
                                 "#error 1\n"
                                 "#endif\n"
                                 "#ifndef B\n"
                                 "#error 2\n"
                                 "#endif\n";
        ASSERT_EQUALS("A;B\n", getConfigsStr(filedata2));

        const char filedata3[] = "#if !A\n"
                                 "#error 1\n"
                                 "#endif\n"
                                 "#if !B\n"
                                 "#error 2\n"
                                 "#endif\n";
        ASSERT_EQUALS("A;B\n", getConfigsStr(filedata3));

    }

    void error7() { // #8074
        const char filedata[] = "#define A\n"
                                "\n"
                                "#if defined(B)\n"
                                "#else\n"
                                "#error \"1\"\n"
                                "#endif\n"
                                "\n"
                                "#if defined(A)\n"
                                "#else\n"
                                "#error \"2\"\n"
                                "#endif\n";
        ASSERT_EQUALS("\nB=B\n", getConfigsStr(filedata));
    }

    void error8() {
        const char filedata[] = "#ifdef A\n"
                                "#ifdef B\n"
                                "#endif\n"
                                "#else\n"
                                "#endif\n"
                                "\n"
                                "#ifndef C\n"
                                "#error aa\n"
                                "#endif";
        ASSERT_EQUALS("A=A;B=B;C\nA=A;C\nC\n", getConfigsStr(filedata));
    }

    void setPlatformInfo() {
        // read code with simplecpp..
        const char filedata[] = "#if sizeof(long) == 4\n"
                                "1\n"
                                "#else\n"
                                "2\n"
                                "#endif\n";

        // preprocess code with unix32 platform..
        {
            std::vector<std::string> files;
            simplecpp::TokenList tokens(filedata, files, "test.c");
            const Settings settings = settingsBuilder().platform(Platform::Type::Unix32).build();
            Preprocessor preprocessor(tokens, settings, *this, Path::identify(tokens.getFiles()[0], false));
            preprocessor.setPlatformInfo();
            ASSERT_EQUALS("\n1", preprocessor.getcode("", files, false));
        }

        // preprocess code with unix64 platform..
        {
            std::vector<std::string> files;
            simplecpp::TokenList tokens(filedata, files, "test.c");
            const Settings settings = settingsBuilder().platform(Platform::Type::Unix64).build();
            Preprocessor preprocessor(tokens, settings, *this, Path::identify(tokens.getFiles()[0], false));
            preprocessor.setPlatformInfo();
            ASSERT_EQUALS("\n\n\n2", preprocessor.getcode("", files, false));
        }
    }

    void includeguard1() {
        ScopedFile header("abc.h",
                          "#ifndef abcH\n"
                          "#define abcH\n"
                          "#endif\n");
        // Handling include guards..
        const char filedata[] = "#include \"abc.h\"\n"
                                "#ifdef ABC\n"
                                "#endif";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }

    void includeguard2() {
        ScopedFile header("abc.h",
                          "foo\n"
                          "#ifdef ABC\n"
                          "\n"
                          "#endif\n");
        // Handling include guards..
        const char filedata[] = "#include \"abc.h\"\n";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }


    void ifdefwithfile() {
        ScopedFile header("abc.h", "class A{};/*\n\n\n\n\n\n\n*/\n");

        // Handling include guards..
        const char filedata[] = "#ifdef ABC\n"
                                "#include \"abc.h\"\n"
                                "#endif\n"
                                "int main() {}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, actual.size());
        ASSERT_EQUALS("\n\n\nint main ( ) { }", actual.at(""));
        ASSERT_EQUALS("\n#line 1 \"abc.h\"\nclass A { } ;\n#line 4 \"file.c\"\n int main ( ) { }", actual.at("ABC=ABC"));
    }

    void if0() {
        const char filedata[] = " # if /* comment */  0 // comment\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void if1() {
        const char filedata[] = " # if /* comment */  1 // comment\n"
                                "ABC\n"
                                " # endif \n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }


    void elif() {
        {
            const char filedata[] = "#if DEF1\n"
                                    "ABC\n"
                                    "#elif DEF2\n"
                                    "DEF\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nDEF1\nDEF2\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if(defined DEF1)\n"
                                    "ABC\n"
                                    "#elif(defined DEF2)\n"
                                    "DEF\n"
                                    "#else\n"
                                    "GHI\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nDEF1=DEF1\nDEF2=DEF2\n", getConfigsStr(filedata));
        }
    }

    void if_cond1() {
        const char filedata[] = "#if LIBVER>100\n"
                                "    A\n"
                                "#else\n"
                                "    B\n"
                                "#endif\n";
        ASSERT_EQUALS("\nLIBVER=101\n", getConfigsStr(filedata));
    }

    void if_cond2() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#endif\n"
                                "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA=A\nA=A;B=B\n", getConfigsStr(filedata));

        if_cond2b();
        if_cond2c();
        if_cond2d();
        if_cond2e();
    }

    void if_cond2b() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nA;B=B\n", "\nA\nB=B\n", getConfigsStr(filedata));
    }

    void if_cond2c() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nA\nA;B=B\n", "\nA\nB=B\n", getConfigsStr(filedata));
    }

    void if_cond2d() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA\nA;B=B\nB=B\n", getConfigsStr(filedata));
    }

    void if_cond2e() {
        const char filedata[] = "#if !defined(A)\n"
                                "!a\n"
                                "#elif !defined(B)\n"
                                "!b\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA\nB\n", getConfigsStr(filedata));
    }

    void if_cond3() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#if defined(B) && defined(C)\n"
                                "abc\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA=A\nA=A;B=B;C=C\n", getConfigsStr(filedata));
    }

    void if_cond4() {
        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if defined A || defined B\n"
                                    "ab\n"
                                    "#endif\n";
            ASSERT_EQUALS("\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if A\n"
                                    "{\n"
                                    "#if (defined(B))\n"
                                    "foo();\n"
                                    "#endif\n"
                                    "}\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA\nA;B=B\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if (defined A) || defined (B)\n"
                                    "ab\n"
                                    "#endif\n";
            ASSERT_EQUALS("\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if (A)\n"
                                    "foo();\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if! A\n"
                                    "foo();\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
    }

    void if_cond5() {
        const char filedata[] = "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n"
                                "cd\n"
                                "#if defined(B) && defined(A)\n"
                                "ef\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA=A;B=B\n", getConfigsStr(filedata));
    }

    void if_cond6() {
        const char filedata[] = "\n"
                                "#if defined(A) && defined(B))\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA=A;B=B\n", getConfigsStr(filedata));
    }

    void if_cond8() {
        const char filedata[] = "#if defined(A) + defined(B) + defined(C) != 1\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nA=A\n", "\nA=A;B=B;C=C\n", getConfigsStr(filedata));
    }


    void if_cond9() {
        const char filedata[] = "#if !defined _A\n"
                                "abc\n"
                                "#endif\n";
        ASSERT_EQUALS("\n_A\n", getConfigsStr(filedata));
    }

    void if_cond10() {
        const char filedata[] = "#if !defined(a) && !defined(b)\n"
                                "#if defined(and)\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => don't crash..
        (void)getcode(settings0, *this, filedata);
    }

    void if_cond11() {
        const char filedata[] = "#if defined(L_fixunssfdi) && LIBGCC2_HAS_SF_MODE\n"
                                "#if LIBGCC2_HAS_DF_MODE\n"
                                "#elif FLT_MANT_DIG < W_TYPE_SIZE\n"
                                "#endif\n"
                                "#endif\n";
        (void)getcode(settings0, *this, filedata);
        ASSERT_EQUALS("", errout_str());
    }

    void if_cond12() {
        const char filedata[] = "#define A (1)\n"
                                "#if A == 1\n"
                                ";\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void if_cond13() {
        const char filedata[] = "#if ('A' == 0x41)\n"
                                "123\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void if_cond14() {
        const char filedata[] = "#if !(A)\n"
                                "123\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }



    void if_or_1() {
        const char filedata[] = "#if defined(DEF_10) || defined(DEF_11)\n"
                                "a1;\n"
                                "#endif\n";
        ASSERT_EQUALS("\nDEF_10=DEF_10;DEF_11=DEF_11\n", getConfigsStr(filedata));
    }

    void if_or_2() {
        const char filedata[] = "#if X || Y\n"
                                "a1;\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nX;Y\n", "\n", getConfigsStr(filedata));
    }

    void if_macro_eq_macro() {
        const char code[] = "#define A B\n"
                            "#define B 1\n"
                            "#define C 1\n"
                            "#if A == C\n"
                            "Wilma\n"
                            "#else\n"
                            "Betty\n"
                            "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(code));
    }

    void ticket_3675() {
        const char code[] = "#ifdef YYSTACKSIZE\n"
                            "#define YYMAXDEPTH YYSTACKSIZE\n"
                            "#else\n"
                            "#define YYSTACKSIZE YYMAXDEPTH\n"
                            "#endif\n"
                            "#if YYDEBUG\n"
                            "#endif\n";
        (void)getcode(settings0, *this, code);

        // There's nothing to assert. It just needs to not hang.
    }

    void ticket_3699() {
        const char code[] = "#define INLINE __forceinline\n"
                            "#define inline __forceinline\n"
                            "#define __forceinline inline\n"
                            "#if !defined(_WIN32)\n"
                            "#endif\n"
                            "INLINE inline __forceinline\n";
        const std::map<std::string, std::string> actual = getcode(settings0, *this, code);

        // First, it must not hang. Second, inline must becomes inline, and __forceinline must become __forceinline.
        ASSERT_EQUALS("\n\n\n\n\n$__forceinline $inline $__forceinline", actual.at(""));
    }

    void ticket_4922() { // #4922
        const char code[] = "__asm__ \n"
                            "{ int extern __value) 0; (double return (\"\" } extern\n"
                            "__typeof __finite (__finite) __finite __inline \"__GI___finite\");";
        (void)getcode(settings0, *this, code);
    }

    void macro_simple1() {
        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA(5);\n";
            ASSERT_EQUALS("\nf ( 5 ) ;", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA (5);\n";
            ASSERT_EQUALS("\nf ( 5 ) ;", expandMacros(filedata, *this));
        }
    }

    void macro_simple2() {
        const char filedata[] = "#define min(x,y) x<y?x:y\n"
                                "min(a(),b());\n";
        ASSERT_EQUALS("\na ( ) < b ( ) ? a ( ) : b ( ) ;", expandMacros(filedata, *this));
    }

    void macro_simple3() {
        const char filedata[] = "#define A 4\n"
                                "A AA\n";
        ASSERT_EQUALS("\n4 AA", expandMacros(filedata, *this));
    }

    void macro_simple4() {
        const char filedata[] = "#define TEMP_1 if( temp > 0 ) return 1;\n"
                                "TEMP_1\n";
        ASSERT_EQUALS("\nif ( temp > 0 ) return 1 ;", expandMacros(filedata, *this));
    }

    void macro_simple5() {
        const char filedata[] = "#define ABC if( temp > 0 ) return 1;\n"
                                "\n"
                                "void foo()\n"
                                "{\n"
                                "    int temp = 0;\n"
                                "    ABC\n"
                                "}\n";
        ASSERT_EQUALS("\n\nvoid foo ( )\n{\nint temp = 0 ;\nif ( temp > 0 ) return 1 ;\n}", expandMacros(filedata, *this));
    }

    void macro_simple6() {
        const char filedata[] = "#define ABC (a+b+c)\n"
                                "ABC\n";
        ASSERT_EQUALS("\n( a + b + c )", expandMacros(filedata, *this));
    }

    void macro_simple7() {
        const char filedata[] = "#define ABC(str) str\n"
                                "ABC(\"(\")\n";
        ASSERT_EQUALS("\n\"(\"", expandMacros(filedata, *this));
    }

    void macro_simple8() {
        const char filedata[] = "#define ABC 123\n"
                                "#define ABCD 1234\n"
                                "ABC ABCD\n";
        ASSERT_EQUALS("\n\n123 1234", expandMacros(filedata, *this));
    }

    void macro_simple9() {
        const char filedata[] = "#define ABC(a) f(a)\n"
                                "ABC( \"\\\"\" );\n"
                                "ABC( \"g\" );\n";
        ASSERT_EQUALS("\nf ( \"\\\"\" ) ;\nf ( \"g\" ) ;", expandMacros(filedata, *this));
    }

    void macro_simple10() {
        const char filedata[] = "#define ABC(t) t x\n"
                                "ABC(unsigned long);\n";
        ASSERT_EQUALS("\nunsigned long x ;", expandMacros(filedata, *this));
    }

    void macro_simple11() {
        const char filedata[] = "#define ABC(x) delete x\n"
                                "ABC(a);\n";
        ASSERT_EQUALS("\ndelete a ;", expandMacros(filedata, *this));
    }

    void macro_simple12() {
        const char filedata[] = "#define AB ab.AB\n"
                                "AB.CD\n";
        ASSERT_EQUALS("\nab . AB . CD", expandMacros(filedata, *this));
    }

    void macro_simple13() {
        const char filedata[] = "#define TRACE(x)\n"
                                "TRACE(;if(a))\n";
        ASSERT_EQUALS("", expandMacros(filedata, *this));
    }

    void macro_simple14() {
        const char filedata[] = "#define A \"  a  \"\n"
                                "printf(A);\n";
        ASSERT_EQUALS("\nprintf ( \"  a  \" ) ;", expandMacros(filedata, *this));
    }

    void macro_simple15() {
        const char filedata[] = "#define FOO\"foo\"\n"
                                "FOO\n";
        ASSERT_EQUALS("\n\"foo\"", expandMacros(filedata, *this));
    }

    void macro_simple16() {  // # 4703
        const char filedata[] = "#define MACRO( A, B, C ) class A##B##C##Creator {};\n"
                                "MACRO( B\t, U , G )";
        ASSERT_EQUALS("\nclass BUGCreator { } ;", expandMacros(filedata, *this));
    }

    void macro_simple17() {  // # 5074 - the Token::isExpandedMacro() doesn't always indicate properly if token comes from macro
        // It would probably be OK if the generated code was
        // "\n123+$123" since the first 123 comes from the source code
        const char filedata[] = "#define MACRO(A) A+123\n"
                                "MACRO(123)";
        ASSERT_EQUALS("\n123 + 123", expandMacros(filedata, *this));
    }

    void macro_simple18() {  // (1e-7)
        const char filedata1[] = "#define A (1e-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1e-7 ) ;", expandMacros(filedata1, *this));

        const char filedata2[] = "#define A (1E-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1E-7 ) ;", expandMacros(filedata2, *this));

        const char filedata3[] = "#define A (1e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1e+7 ) ;", expandMacros(filedata3, *this));

        const char filedata4[] = "#define A (1.e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1.e+7 ) ;", expandMacros(filedata4, *this));

        const char filedata5[] = "#define A (1.7f)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1.7f ) ;", expandMacros(filedata5, *this));

        const char filedata6[] = "#define A (.1)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( .1 ) ;", expandMacros(filedata6, *this));

        const char filedata7[] = "#define A (1.)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1. ) ;", expandMacros(filedata7, *this));

        const char filedata8[] = "#define A (8.0E+007)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 8.0E+007 ) ;", expandMacros(filedata8, *this));
    }

    void macroInMacro1() {
        {
            const char filedata[] = "#define A(m) long n = m; n++;\n"
                                    "#define B(n) A(n)\n"
                                    "B(0)\n";
            ASSERT_EQUALS("\n\nlong n = 0 ; n ++ ;", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define A B\n"
                                    "#define B 3\n"
                                    "A\n";
            ASSERT_EQUALS("\n\n3", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define BC(b, c...) 0##b * 0##c\n"
                                    "#define ABC(a, b...) a + BC(b)\n"
                                    "\n"
                                    "ABC(1);\n" // <- too few parameters
                                    "ABC(2,3);\n"
                                    "ABC(4,5,6);\n";

            ASSERT_EQUALS("\n\n\n1 + 0 * 0 ;\n2 + 03 * 0 ;\n4 + 05 * 06 ;", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define A 4\n"
                                    "#define B(a) a,A\n"
                                    "B(2);\n";
            ASSERT_EQUALS("\n\n2 , 4 ;", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define A(x) (x)\n"
                                    "#define B )A(\n"
                                    "#define C )A(\n";
            ASSERT_EQUALS("", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define A(x) (x*2)\n"
                                    "#define B A(\n"
                                    "foo B(i));\n";
            ASSERT_EQUALS("\n\nfoo ( ( i ) * 2 ) ;", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define foo foo\n"
                                    "foo\n";
            ASSERT_EQUALS("\nfoo", expandMacros(filedata, *this));
        }

        {
            const char filedata[] =
                "#define B(A1, A2) } while (0)\n"
                "#define A(name) void foo##name() { do { B(1, 2); }\n"
                "A(0)\n"
                "A(1)\n";
            ASSERT_EQUALS("\n\nvoid foo0 ( ) { do { } while ( 0 ) ; }\nvoid foo1 ( ) { do { } while ( 0 ) ; }", expandMacros(filedata, *this));
        }

        {
            const char filedata[] =
                "#define B(x) (\n"
                "#define A() B(xx)\n"
                "B(1) A() ) )\n";
            ASSERT_EQUALS("\n\n( ( ) )", expandMacros(filedata, *this));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "#define PTR2 PTR1 PTR1\n"
                "int PTR2 PTR2 foo )))) = 0;\n";
            ASSERT_EQUALS("\n\nint ( ( ( ( foo ) ) ) ) = 0 ;", expandMacros(filedata, *this));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "PTR1 PTR1\n";
            ASSERT_EQUALS("\n( (", expandMacros(filedata, *this));
        }
    }

    void macroInMacro2() {
        const char filedata[] = "#define A(x) a##x\n"
                                "#define B 0\n"
                                "A(B)\n";
        ASSERT_EQUALS("\n\naB", expandMacros(filedata, *this));
    }

    void macro_linenumbers() {
        const char filedata[] = "#define AAA(a)\n"
                                "AAA(5\n"
                                "\n"
                                ")\n"
                                "int a;\n";
        ASSERT_EQUALS("\n"
                      "\n"
                      "\n"
                      "\n"
                      "int a ;",
                      expandMacros(filedata, *this));
    }

    void macro_nopar() {
        const char filedata[] = "#define AAA( ) { NULL }\n"
                                "AAA()\n";
        ASSERT_EQUALS("\n{ NULL }", expandMacros(filedata, *this));
    }

    void macro_incdec() {
        const char filedata[] = "#define M1(X) 1+X\n"
                                "#define M2(X) 2-X\n"
                                "M1(+1) M2(-1)\n";
        ASSERT_EQUALS("\n\n1 + + 1 2 - - 1", expandMacros(filedata, *this));
    }

    void macro_switchCase() {
        {
            // Make sure "case 2" doesn't become "case2"
            const char filedata[] = "#define A( b ) "
                                    "switch( a ){ "
                                    "case 2: "
                                    " break; "
                                    "}\n"
                                    "A( 5 );\n";
            ASSERT_EQUALS("\nswitch ( a ) { case 2 : break ; } ;", expandMacros(filedata, *this));
        }

        {
            // Make sure "2 BB" doesn't become "2BB"
            const char filedata[] = "#define A() AA : 2 BB\n"
                                    "A();\n";
            ASSERT_EQUALS("\nAA : 2 BB ;", expandMacros(filedata, *this));
        }

        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{ } break ; ;", expandMacros(filedata, *this));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() _break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{ } _break ; ;", expandMacros(filedata, *this));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() 5;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{ } 5 ; ;", expandMacros(filedata, *this));
        }
    }

    void macro_NULL() {
        // See ticket #4482 - UB when passing NULL to variadic function
        ASSERT_EQUALS("\n0", expandMacros("#define null 0\nnull", *this));
        TODO_ASSERT_EQUALS("\nNULL", "\n0", expandMacros("#define NULL 0\nNULL", *this)); // TODO: Let the tokenizer handle NULL?
    }

    void string1() {
        const char filedata[] = "int main()"
                                "{"
                                "    const char *a = \"#define A\";"
                                "}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("int main ( ) { const char * a = \"#define A\" ; }", actual.at(""));
    }

    void string2() {
        const char filedata[] = "#define AAA 123\n"
                                "str = \"AAA\"\n";

        // Compare results..
        ASSERT_EQUALS("\nstr = \"AAA\"", expandMacros(filedata, *this));
    }

    void string3() {
        const char filedata[] = "str(\";\");\n";

        // Compare results..
        ASSERT_EQUALS("str ( \";\" ) ;", expandMacros(filedata, *this));
    }


    void preprocessor_undef() {
        {
            const char filedata[] = "#define AAA int a;\n"
                                    "#undef AAA\n"
                                    "#define AAA char b=0;\n"
                                    "AAA\n";

            // Compare results..
            ASSERT_EQUALS("\n\n\nchar b = 0 ;", expandMacros(filedata, *this));
        }

        {
            // ticket #403
            const char filedata[] = "#define z p[2]\n"
                                    "#undef z\n"
                                    "int z;\n"
                                    "z = 0;\n";
            ASSERT_EQUALS("\n\nint z ;\nz = 0 ;", getcodeforcfg(settings0, *this, filedata, "", "test.c"));
        }
    }

    void defdef() {
        const char filedata[] = "#define AAA 123\n"
                                "#define AAA 456\n"
                                "#define AAA 789\n"
                                "AAA\n";

        // Compare results..
        ASSERT_EQUALS("\n\n\n789", expandMacros(filedata, *this));
    }

    void preprocessor_doublesharp() {
        // simple testcase without ##
        const char filedata1[] = "#define TEST(var,val) var = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("\nfoo = 20 ;", expandMacros(filedata1, *this));

        // simple testcase with ##
        const char filedata2[] = "#define TEST(var,val) var##_##val = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("\nfoo_20 = 20 ;", expandMacros(filedata2, *this));

        // concat macroname
        const char filedata3[] = "#define ABCD 123\n"
                                 "#define A(B) A##B\n"
                                 "A(BCD)\n";
        ASSERT_EQUALS("\n\n123", expandMacros(filedata3, *this));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata4[] = "#define A(B) A##B\n"
                                 "#define a(B) A(B)\n"
                                 "a(A(B))\n";
        ASSERT_EQUALS("\n\nAAB", expandMacros(filedata4, *this));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata5[] = "#define AB(A,B) A##B\n"
                                 "#define ab(A,B) AB(A,B)\n"
                                 "ab(a,AB(b,c))\n";
        ASSERT_EQUALS("\n\nabc", expandMacros(filedata5, *this));

        // Ticket #1802
        const char filedata6[] = "#define AB_(A,B) A ## B\n"
                                 "#define AB(A,B) AB_(A,B)\n"
                                 "#define ab(suf) AB(X, AB_(_, suf))\n"
                                 "#define X x\n"
                                 "ab(y)\n";
        ASSERT_EQUALS("\n\n\n\nx_y", expandMacros(filedata6, *this));
    }



    void preprocessor_include_in_str() {
        const char filedata[] = "int main()\n"
                                "{\n"
                                "const char *a = \"#include <string>\";\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("int main ( )\n{\nconst char * a = \"#include <string>\" ;\nreturn 0 ;\n}", actual.at(""));
    }

    void va_args_1() {
        const char filedata[] = "#define DBG(fmt...) printf(fmt)\n"
                                "DBG(\"[0x%lx-0x%lx)\", pstart, pend);\n";

        // Preprocess..
        std::string actual = expandMacros(filedata, *this);

        ASSERT_EQUALS("\nprintf ( \"[0x%lx-0x%lx)\" , pstart , pend ) ;", actual);
    }
    /*
        void va_args_2() {
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "DBG(\"hello\");\n";

            // Preprocess..
            std::string actual = expandMacros(filedata, *this);

            // invalid code ASSERT_EQUALS("\nprintf ( \"hello\" ) ;", actual);
        }
     */
    void va_args_3() {
        const char filedata[] = "#define FRED(...) { fred(__VA_ARGS__); }\n"
                                "FRED(123)\n";
        ASSERT_EQUALS("\n{ fred ( 123 ) ; }", expandMacros(filedata, *this));
    }

    void va_args_4() {
        const char filedata[] = "#define FRED(name, ...) name (__VA_ARGS__)\n"
                                "FRED(abc, 123)\n";
        ASSERT_EQUALS("\nabc ( 123 )", expandMacros(filedata, *this));
    }

    void va_args_5() {
        const char filedata1[] = "#define A(...) #__VA_ARGS__\n"
                                 "A(123)\n";
        ASSERT_EQUALS("\n\"123\"", expandMacros(filedata1, *this));

        const char filedata2[] = "#define A(X,...) X(#__VA_ARGS__)\n"
                                 "A(f,123)\n";
        ASSERT_EQUALS("\nf ( \"123\" )", expandMacros(filedata2, *this));
    }



    void multi_character_character() {
        const char filedata[] = "#define FOO 'ABCD'\n"
                                "int main()\n"
                                "{\n"
                                "if( FOO == 0 );\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("\nint main ( )\n{\nif ( $'ABCD' == 0 ) ;\nreturn 0 ;\n}", actual.at(""));
    }


    void stringify() {
        const char filedata[] = "#define STRINGIFY(x) #x\n"
                                "STRINGIFY(abc)\n";

        // expand macros..
        std::string actual = expandMacros(filedata, *this);

        ASSERT_EQUALS("\n\"abc\"", actual);
    }

    void stringify2() {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A(abc);\n";

        // expand macros..
        std::string actual = expandMacros(filedata, *this);

        ASSERT_EQUALS("\ng ( \"abc\" ) ;", actual);
    }

    void stringify3() {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A( abc);\n";

        // expand macros..
        std::string actual = expandMacros(filedata, *this);

        ASSERT_EQUALS("\ng ( \"abc\" ) ;", actual);
    }

    void stringify4() {
        const char filedata[] = "#define A(x) #x\n"
                                "1 A(\n"
                                "abc\n"
                                ") 2\n";

        // expand macros..
        std::string actual = expandMacros(filedata, *this);

        ASSERT_EQUALS("\n1 \"abc\"\n\n2", actual);
    }

    void stringify5() {
        const char filedata[] = "#define A(x) a(#x,x)\n"
                                "A(foo(\"\\\"\"))\n";
        ASSERT_EQUALS("\na ( \"foo(\\\"\\\\\\\"\\\")\" , foo ( \"\\\"\" ) )", expandMacros(filedata, *this));
    }

    void pragma() {
        const char filedata[] = "#pragma once\n"
                                "void f()\n"
                                "{\n"
                                "}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("\nvoid f ( )\n{\n}", actual.at(""));
    }

    void pragma_asm_1() {
        const char filedata[] = "#pragma asm\n"
                                "    mov r1, 11\n"
                                "#pragma endasm\n"
                                "aaa\n"
                                "#pragma asm foo\n"
                                "    mov r1, 11\n"
                                "#pragma endasm bar\n"
                                "bbb";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("asm ( )\n;\n\naaa\nasm ( ) ;\n\n\nbbb", actual.at(""));
    }

    void pragma_asm_2() {
        const char filedata[] = "#pragma asm\n"
                                "    mov @w1, 11\n"
                                "#pragma endasm ( temp=@w1 )\n"
                                "bbb";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("asm ( )\n;\n\nbbb", actual.at(""));
    }

    void endifsemicolon() {
        const char filedata[] = "void f() {\n"
                                "#ifdef A\n"
                                "#endif;\n"
                                "}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(2, actual.size());
        const std::string expected("void f ( ) {\n\n\n}");
        ASSERT_EQUALS(expected, actual.at(""));
        ASSERT_EQUALS(expected, actual.at("A=A"));
    }

    void handle_error() {
        {
            const char filedata[] = "#define A \n"
                                    "#define B don't want to \\\n"
                                    "more text\n"
                                    "void f()\n"
                                    "{\n"
                                    "  char a = 'a'; // '\n"
                                    "}\n";

            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

            ASSERT_EQUALS(0, actual.size());
            ASSERT_EQUALS("[file.c:2:14]: (error) No pair for character ('). Can't process file. File is either invalid or unicode, which is currently not supported. [syntaxError]\n", errout_str());
        }
    }

    void missing_doublequote() {
        {
            const char filedata[] = "#define a\n"
                                    "#ifdef 1\n"
                                    "\"\n"
                                    "#endif\n";

            // expand macros..
            const std::string actual(expandMacros(filedata, *this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:3:1]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported. [syntaxError]\n", errout_str());
        }

        {
            ScopedFile header("abc.h",
                              "#define a\n"
                              "\"\n");
            const char filedata[] = "#include \"abc.h\"";

            // expand macros..
            const std::string actual(expandMacros(filedata, *this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[abc.h:2:1]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported. [syntaxError]\n", errout_str());
        }

        {
            ScopedFile header("abc.h",
                              "#define a\n");
            const char filedata[] = "#include \"abc.h\"\n"
                                    "\"\n";

            // expand macros..
            const std::string actual(expandMacros(filedata, *this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:2:1]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported. [syntaxError]\n", errout_str());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#define B \"\n"
                                    "int a = A;\n";

            // expand macros..
            const std::string actual(expandMacros(filedata, *this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:2:11]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported. [syntaxError]\n", errout_str());
        }

        {
            const char filedata[] = "void foo()\n"
                                    "{\n"
                                    "\n"
                                    "\n"
                                    "\n"
                                    "int a = 0;\n"
                                    "printf(Text\");\n"
                                    "}\n";

            // expand macros..
            (void)expandMacros(filedata, *this);

            ASSERT_EQUALS("[file.cpp:7:12]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported. [syntaxError]\n", errout_str());
        }
    }


    void define_part_of_func() {
        const char filedata[] = "#define A g(\n"
                                "void f() {\n"
                                "  A );\n"
                                "  }\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("\nvoid f ( ) {\n$g $( ) ;\n}", actual.at(""));
        ASSERT_EQUALS("", errout_str());
    }

    void conditionalDefine() {
        const char filedata[] = "#ifdef A\n"
                                "#define N 10\n"
                                "#else\n"
                                "#define N 20\n"
                                "#endif\n"
                                "N";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(2, actual.size());
        ASSERT_EQUALS("\n\n\n\n\n$20", actual.at(""));
        ASSERT_EQUALS("\n\n\n\n\n$10", actual.at("A=A"));
        ASSERT_EQUALS("", errout_str());
    }

    void macro_parameters() {
        const char filedata[] = "#define BC(a, b, c, arg...) \\\n"
                                "AB(a, b, c, ## arg)\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "  BC(3);\n"
                                "}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("", actual.at(""));
        ASSERT_EQUALS("[file.c:6:3]: (error) failed to expand 'BC', Wrong number of parameters for macro 'BC'. [syntaxError]\n", errout_str());
    }

    void newline_in_macro() {
        const char filedata[] = "#define ABC(str) printf( str )\n"
                                "void f()\n"
                                "{\n"
                                "  ABC(\"\\n\");\n"
                                "}\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("\nvoid f ( )\n{\n$printf $( \"\\n\" $) ;\n}", actual.at(""));
        ASSERT_EQUALS("", errout_str());
    }

    void ifdef_ifdefined() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#if defined ABC\n"
                                "A\n"
                                "#endif\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(2, actual.size());
        ASSERT_EQUALS("", actual.at(""));
        ASSERT_EQUALS("\nA\n\n\nA", actual.at("ABC=ABC"));
    }

    void define_if1() {
        {
            const char filedata[] = "#define A 0\n"
                                    "#if A\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("", getcodeforcfg(settings0, *this, filedata,"","test.c"));
        }
        {
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\nFOO", getcodeforcfg(settings0, *this, filedata,"","test.c"));
        }
    }

    void define_if2() {
        const char filedata[] = "#define A 22\n"
                                "#define B A\n"
                                "#if (B==A) || (B==C)\n"
                                "FOO\n"
                                "#endif";
        ASSERT_EQUALS("\n\n\nFOO", getcodeforcfg(settings0, *this, filedata,"","test.c"));
    }

    void define_if3() {
        const char filedata[] = "#define A 0\n"
                                "#if (A==0)\n"
                                "FOO\n"
                                "#endif";
        ASSERT_EQUALS("\n\nFOO", getcodeforcfg(settings0, *this, filedata,"","test.c"));
    }

    void define_if4() {
        const char filedata[] = "#define X +123\n"
                                "#if X==123\n"
                                "FOO\n"
                                "#endif";
        ASSERT_EQUALS("\n\nFOO", getcodeforcfg(settings0, *this, filedata,"","test.c"));
    }

    void define_if5() { // #4516 - #define B (A & 0x00f0)
        {
            const char filedata[] = "#define A 0x0010\n"
                                    "#define B (A & 0x00f0)\n"
                                    "#if B==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\n\nFOO", getcodeforcfg(settings0, *this, filedata,"","test.c"));
        }
        {
            const char filedata[] = "#define A 0x00f0\n"
                                    "#define B (16)\n"
                                    "#define C (B & A)\n"
                                    "#if C==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\n\n\nFOO", getcodeforcfg(settings0, *this, filedata,"","test.c"));
        }
        {
            const char filedata[] = "#define A (1+A)\n" // don't hang for recursive macros
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\nFOO", getcodeforcfg(settings0, *this, filedata,"","test.c"));
        }
    }

    void define_if6() { // #4516 - #define B (A?1:-1)
        const char filedata[] = "#ifdef A\n"
                                "#define B (A?1:-1)\n"
                                "#endif\n"
                                "\n"
                                "#if B < 0\n"
                                "123\n"
                                "#endif\n"
                                "\n"
                                "#if B >= 0\n"
                                "456\n"
                                "#endif\n";
        const std::string actualA0 = getcodeforcfg(settings0, *this, filedata, "A=0", "test.c");
        ASSERT_EQUALS(true,  actualA0.find("123") != std::string::npos);
        ASSERT_EQUALS(false, actualA0.find("456") != std::string::npos);
        const std::string actualA1 = getcodeforcfg(settings0, *this, filedata, "A=1", "test.c");
        ASSERT_EQUALS(false, actualA1.find("123") != std::string::npos);
        ASSERT_EQUALS(true,  actualA1.find("456") != std::string::npos);
    }

    void define_ifdef() {
        {
            const char filedata[] = "#define ABC\n"
                                    "#ifndef ABC\n"
                                    "A\n"
                                    "#else\n"
                                    "B\n"
                                    "#endif\n";

            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

            // Compare results..
            ASSERT_EQUALS(1, actual.size());
            ASSERT_EQUALS("\n\n\n\nB", actual.at(""));
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#ifdef A\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

            // Compare results..
            ASSERT_EQUALS(1, actual.size());
            ASSERT_EQUALS("\n\n$1", actual.at(""));
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

            // Compare results..
            ASSERT_EQUALS(1, actual.size());
            ASSERT_EQUALS("\n\n$1", actual.at(""));
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if A>0\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

            // Compare results..
            ASSERT_EQUALS(1, actual.size());
            ASSERT_EQUALS("\n\n$1", actual.at(""));
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if 0\n"
                                    "#undef A\n"
                                    "#endif\n"
                                    "A\n";

            // Preprocess => actual result..
            const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

            // Compare results..
            ASSERT_EQUALS(1, actual.size());
            ASSERT_EQUALS("\n\n\n\n$1", actual.at(""));
        }
    }

    void define_ifndef1() {
        const char filedata[] = "#define A(x) (x)\n"
                                "#ifndef A\n"
                                ";\n"
                                "#endif\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("", actual.at(""));
    }

    void define_ifndef2() {
        const char filedata[] = "#ifdef A\n"
                                "#define B char\n"
                                "#endif\n"
                                "#ifndef B\n"
                                "#define B int\n"
                                "#endif\n"
                                "B me;\n";

        // Preprocess => actual result..
        ASSERT_EQUALS("\n\n\n\n\n\n$int me ;", getcodeforcfg(settings0, *this, filedata, "", "a.cpp"));
        ASSERT_EQUALS("\n\n\n\n\n\n$char me ;", getcodeforcfg(settings0, *this, filedata, "A", "a.cpp"));
    }

    void ifndef_define() {
        const char filedata[] = "#ifndef A\n"
                                "#define A(x) x\n"
                                "#endif\n"
                                "A(123);";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n123 ;", actual.at(""));
    }

    void undef_ifdef() {
        const char filedata[] = "#undef A\n"
                                "#ifdef A\n"
                                "123\n"
                                "#endif\n";

        // Preprocess => actual result..
        ASSERT_EQUALS("", getcodeforcfg(settings0, *this, filedata, "", "a.cpp"));
        ASSERT_EQUALS("", getcodeforcfg(settings0, *this, filedata, "A", "a.cpp"));
    }

    void redundant_config() {
        const char filedata[] = "int main() {\n"
                                "#ifdef FOO\n"
                                "#ifdef BAR\n"
                                "    std::cout << 1;\n"
                                "#endif\n"
                                "#endif\n"
                                "\n"
                                "#ifdef BAR\n"
                                "#ifdef FOO\n"
                                "    std::cout << 2;\n"
                                "#endif\n"
                                "#endif\n"
                                "}\n";


        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(4, actual.size());
        ASSERT(actual.find("") != actual.end());
        ASSERT(actual.find("BAR=BAR") != actual.end());
        ASSERT(actual.find("FOO=FOO") != actual.end());
        ASSERT(actual.find("BAR=BAR;FOO=FOO") != actual.end());
    }


    void endfile() {
        const char filedata[] = "char a[] = \"#endfile\";\n"
                                "char b[] = \"#endfile\";\n"
                                "#include \"notfound.h\"\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("char a [ ] = \"#endfile\" ;\nchar b [ ] = \"#endfile\" ;", actual.at(""));
    }

    void dup_defines() {
        const char filedata[] = "#ifdef A\n"
                                "#define B\n"
                                "#if defined(B) && defined(A)\n"
                                "a\n"
                                "#else\n"
                                "b\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // B will always be defined if A is defined; the following test
        // cases should be fixed whenever this other bug is fixed
        ASSERT_EQUALS(2U, actual.size());

        ASSERT_EQUALS_MSG(true, (actual.find("A=A") != actual.end()), "A is expected to be checked but it was not checked");

        ASSERT_EQUALS_MSG(true, (actual.find("A=A;A=A;B=B") == actual.end()), "A;A;B is expected to NOT be checked but it was checked");
    }

    void invalid_define_1() {
        (void)getcode(settings0, *this, "#define =\n");
        ASSERT_EQUALS("[file.c:1:2]: (error) Failed to parse #define [syntaxError]\n", errout_str());
    }

    void invalid_define_2() {  // #4036
        (void)getcode(settings0, *this, "#define () {(int f(x) }\n");
        ASSERT_EQUALS("[file.c:1:2]: (error) Failed to parse #define [syntaxError]\n", errout_str());
    }

    void inline_suppressions() {
        const auto settings = dinit(Settings,
                                    $.inlineSuppressions = true,
                                        $.checks.enable (Checks::missingInclude));

        const char code[] = "// cppcheck-suppress missingInclude\n"
                            "#include \"missing.h\"\n"
                            "// cppcheck-suppress missingIncludeSystem\n"
                            "#include <missing2.h>\n";
        SuppressionList inlineSuppr;
        (void)getcodeforcfg(settings, *this, code, "", "test.c", &inlineSuppr);

        auto suppressions = inlineSuppr.getSuppressions();
        ASSERT_EQUALS(2, suppressions.size());

        auto suppr = suppressions.front();
        suppressions.pop_front();
        ASSERT_EQUALS("missingInclude", suppr.errorId);
        ASSERT_EQUALS("test.c", suppr.fileName);
        ASSERT_EQUALS(2, suppr.lineNumber);
        ASSERT_EQUALS(false, suppr.checked);
        ASSERT_EQUALS(false, suppr.matched);

        suppr = suppressions.front();
        suppressions.pop_front();
        ASSERT_EQUALS("missingIncludeSystem", suppr.errorId);
        ASSERT_EQUALS("test.c", suppr.fileName);
        ASSERT_EQUALS(4, suppr.lineNumber);
        ASSERT_EQUALS(false, suppr.checked);
        ASSERT_EQUALS(false, suppr.matched);

        ignore_errout(); // we are not interested in the output
    }

    void inline_suppressions_not_next_line() {
        const auto settings = dinit(Settings,
                                    $.inlineSuppressions = true,
                                        $.checks.enable (Checks::missingInclude));

        const char code[] = "// cppcheck-suppress missingInclude\n"
                            "// some other comment\n"
                            "#include \"missing.h\"\n"
                            "// cppcheck-suppress missingIncludeSystem\n"
                            "\n" // Empty line
                            "#include <missing2.h>\n";
        SuppressionList inlineSuppr;
        (void)getcodeforcfg(settings, *this, code, "", "test.c", &inlineSuppr);

        auto suppressions = inlineSuppr.getSuppressions();
        ASSERT_EQUALS(2, suppressions.size());

        auto suppr = suppressions.front();
        suppressions.pop_front();
        ASSERT_EQUALS("missingInclude", suppr.errorId);
        ASSERT_EQUALS("test.c", suppr.fileName);
        ASSERT_EQUALS(3, suppr.lineNumber);

        suppr = suppressions.front();
        suppressions.pop_front();
        ASSERT_EQUALS("missingIncludeSystem", suppr.errorId);
        ASSERT_EQUALS("test.c", suppr.fileName);
        ASSERT_EQUALS(6, suppr.lineNumber);

        ignore_errout();
    }

    void remarkComment1() {
        const char code[] = "// REMARK: assignment with 1\n"
                            "x=1;\n";
        const auto remarkComments = getRemarkComments(code, *this);
        ASSERT_EQUALS(1, remarkComments.size());
        ASSERT_EQUALS(2, remarkComments[0].lineNumber);
        ASSERT_EQUALS("assignment with 1", remarkComments[0].str);
    }

    void remarkComment2() {
        const char code[] = "x=1; ///REMARK assignment with 1\n";
        const auto remarkComments = getRemarkComments(code, *this);
        ASSERT_EQUALS(1, remarkComments.size());
        ASSERT_EQUALS(1, remarkComments[0].lineNumber);
        ASSERT_EQUALS("assignment with 1", remarkComments[0].str);
    }

    void remarkComment3() {
        const char code[] = "/**   REMARK: assignment with 1 */\n"
                            "x=1;\n";
        const auto remarkComments = getRemarkComments(code, *this);
        ASSERT_EQUALS(1, remarkComments.size());
        ASSERT_EQUALS(2, remarkComments[0].lineNumber);
        ASSERT_EQUALS("assignment with 1 ", remarkComments[0].str);
    }

    void remarkComment4() {
        const char code[] = "//REMARK /";
        const auto remarkComments = getRemarkComments(code, *this);
        ASSERT_EQUALS(0, remarkComments.size());
    }

    void predefine1() {
        const char code[] = "#if defined X || Y\n"
                            "Fred & Wilma\n"
                            "#endif\n";
        std::string actual = getcodeforcfg(settings0, *this, code, "X=1", "test.c");

        ASSERT_EQUALS("\nFred & Wilma", actual);
    }

    void predefine2() {
        const char code[] = "#if defined(X) && Y\n"
                            "Fred & Wilma\n"
                            "#endif\n";
        {
            std::string actual = getcodeforcfg(settings0, *this, code, "X=1", "test.c");
            ASSERT_EQUALS("", actual);
        }

        {
            std::string actual = getcodeforcfg(settings0, *this, code, "X=1;Y=2", "test.c");
            ASSERT_EQUALS("\nFred & Wilma", actual);
        }
    }

    void predefine3() {
        // #2871 - define in source is not used if -D is used
        const char code[] = "#define X 1\n"
                            "#define Y X\n"
                            "#if (X == Y)\n"
                            "Fred & Wilma\n"
                            "#endif\n";
        const std::string actual = getcodeforcfg(settings0, *this, code, "TEST", "test.c");
        ASSERT_EQUALS("\n\n\nFred & Wilma", actual);
    }

    void predefine4() {
        // #3577
        const char code[] = "char buf[X];\n";
        const std::string actual = getcodeforcfg(settings0, *this, code, "X=123", "test.c");
        ASSERT_EQUALS("char buf [ $123 ] ;", actual);
    }

    void predefine5() {  // #3737, #5119 - automatically define __cplusplus
        // #3737...
        const char code[] = "#ifdef __cplusplus\n123\n#endif";
        ASSERT_EQUALS("",      getcodeforcfg(settings0, *this, code, "", "test.c"));
        ASSERT_EQUALS("\n123", getcodeforcfg(settings0, *this, code, "", "test.cpp"));
    }

    void predefine6() { // automatically define __STDC_VERSION__
        const char code[] = "#ifdef __STDC_VERSION__\n123\n#endif";
        ASSERT_EQUALS("\n123", getcodeforcfg(settings0, *this, code, "", "test.c"));
        ASSERT_EQUALS("",      getcodeforcfg(settings0, *this, code, "", "test.cpp"));
    }

    void strictAnsi() {
        const char code[] = "#ifdef __STRICT_ANSI__\n123\n#endif";
        Settings settings;

        settings.standards.setStd("gnu99");
        ASSERT_EQUALS("", getcodeforcfg(settings, *this, code, "", "test.c"));

        settings.standards.setStd("c99");
        ASSERT_EQUALS("\n123", getcodeforcfg(settings, *this, code, "", "test.c"));

        settings.standards.setStd("gnu++11");
        ASSERT_EQUALS("", getcodeforcfg(settings, *this, code, "", "test.cpp"));

        settings.standards.setStd("c++11");
        ASSERT_EQUALS("\n123", getcodeforcfg(settings, *this, code, "", "test.cpp"));
    }

    void invalidElIf() {
        // #2942 - segfault
        const char code[] = "#elif (){\n";
        const std::string actual = getcodeforcfg(settings0, *this, code, "TEST", "test.c");
        ASSERT_EQUALS("", actual);
        ASSERT_EQUALS("[test.c:1:2]: (error) #elif without #if [syntaxError]\n", errout_str());
    }

    void getConfigs1() {
        const char filedata[] = "#ifdef  WIN32 \n"
                                "    abcdef\n"
                                "#else  \n"
                                "    qwerty\n"
                                "#endif  \n";

        ASSERT_EQUALS("\nWIN32=WIN32\n", getConfigsStr(filedata));
    }

    void getConfigs2() {
        const char filedata[] = "# ifndef WIN32\n"
                                "    \" # ifdef WIN32\" // a comment\n"
                                "   #   else  \n"
                                "    qwerty\n"
                                "  # endif  \n";
        ASSERT_EQUALS("\nWIN32\n", getConfigsStr(filedata));
    }

    void getConfigs3() {
        const char filedata[] = "#ifdef ABC\n"
                                "a\n"
                                "#ifdef DEF\n"
                                "b\n"
                                "#endif\n"
                                "c\n"
                                "#endif\n";

        ASSERT_EQUALS("\nABC=ABC\nABC=ABC;DEF=DEF\n", getConfigsStr(filedata));
    }

    void getConfigs4() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#ifdef ABC\n"
                                "A\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }

    void getConfigs5() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#else\n"
                                "B\n"
                                "#ifdef DEF\n"
                                "C\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC=ABC\nDEF=DEF\n", getConfigsStr(filedata));
    }

    void getConfigs7() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }

    void getConfigs7a() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void getConfigs7b() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }

    void getConfigs7c() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }

    void getConfigs7d() {
        const char filedata[] = "#if defined(ABC)\n"
                                "A\n"
                                "#if defined(ABC)\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }

    void getConfigs7e() {
        ScopedFile header("test.h",
                          "#ifndef test_h\n"
                          "#define test_h\n"
                          "#ifdef ABC\n"
                          "#endif\n"
                          "#endif\n");
        const char filedata[] = "#ifdef ABC\n"
                                "#include \"test.h\"\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC=ABC\n", getConfigsStr(filedata));
    }

    void getConfigs8() {
        const char filedata[] = "#if A == 1\n"
                                "1\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA=1\n", getConfigsStr(filedata));
    }

    void getConfigs10() { // Ticket #5139
        const char filedata[] = "#define foo a.foo\n"
                                "#define bar foo\n"
                                "#define baz bar+0\n"
                                "#if 0\n"
                                "#endif";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void getConfigs11() { // #9832 - include guards
        ScopedFile header("test.h",
                          "#if !defined(test_h)\n"
                          "#define test_h\n"
                          "123\n"
                          "#endif\n");
        const char filedata[] = "#include \"test.h\"\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void getConfigs12() { // #14222
        const char filedata[] = "#ifdef INT8_MAX\n"
                                "INT8_MAX\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, nullptr, "std.cfg"));
    }

    void getConfigs13() { // #14222
        const char filedata[] = "#ifdef __builtin_bswap16\n"
                                "__builtin_bswap16(x);\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, nullptr, "gnu.cfg"));
    }

    void getConfigs_gte() { // #1059
        {
            const char filedata[] = "#if A >= 1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=1\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A >= 201112L\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=201112L\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A >= 12147483647\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=12147483647\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_lte() { // #1059
        {
            const char filedata[] = "#if A <= 1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=1\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A <= 201112L\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=201112L\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A <= 12147483647\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=12147483647\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_gt() { // #1059
        {
            const char filedata[] = "#if A > 1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 1L\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 1U\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 1UL\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 1Z\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 0x1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 01\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 0b1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 1t\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_THROW_INTERNAL_EQUALS(getConfigsStr(filedata), INTERNAL, "Internal Error. MathLib::toBigNumber: input was not completely consumed: 1t");
        }
        {
            const char filedata[] = "#if A > 1.0\n"
                                    "1\n"
                                    "#endif\n";
            TODO_ASSERT_THROW(getConfigsStr(filedata), InternalError); // floating point literals are not allowed
        }
        {
            const char filedata[] = "#if A > 12147483647\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=12147483648\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_lt() { // #1059
        {
            const char filedata[] = "#if A < 1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 1L\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 1U\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 1UL\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 1Z\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 0x1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 01\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 0b1\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 1t\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_THROW_INTERNAL_EQUALS(getConfigsStr(filedata), INTERNAL, "Internal Error. MathLib::toBigNumber: input was not completely consumed: 1t");
        }
        {
            const char filedata[] = "#if A < 1.0\n"
                                    "1\n"
                                    "#endif\n";
            TODO_ASSERT_THROW(getConfigsStr(filedata), InternalError); // floating point literals are not allowed
        }
        {
            const char filedata[] = "#if A < 12147483647\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=12147483646\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_eq_compound() { // #1059
        {
            const char filedata[] = "#if A == 1 && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=1;B=B\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A == 201112L && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=201112L;B=B\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_gte_compound() { // #1059
        {
            const char filedata[] = "#if A >= 1 && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=1;B=B\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A >= 201112L && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=201112L;B=B\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_lte_compound() { // #1059
        {
            const char filedata[] = "#if A <= 1 && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=1;B=B\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A <= 201112L && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=201112L;B=B\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_gt_compound() { // #1059
        {
            const char filedata[] = "#if A > 1 && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=2;B=B\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A > 201112L && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=201113;B=B\n", getConfigsStr(filedata));
        }
    }

    void getConfigs_lt_compound() { // #1059
        {
            const char filedata[] = "#if A < 1 && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=0;B=B\n", getConfigsStr(filedata));
        }
        {
            const char filedata[] = "#if A < 201112L && defined(B)\n"
                                    "1\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA=201111;B=B\n", getConfigsStr(filedata));
        }
    }

    void getConfigsError() {
        const char filedata1[] = "#ifndef X\n"
                                 "#error \"!X\"\n"
                                 "#endif\n";
        ASSERT_EQUALS("X\n", getConfigsStr(filedata1));

        const char filedata2[] = "#ifdef X\n"
                                 "#ifndef Y\n"
                                 "#error \"!Y\"\n"
                                 "#endif\n"
                                 "#endif\n";
        ASSERT_EQUALS("\nX=X;Y\nY\n", getConfigsStr(filedata2));
    }

    void getConfigsD1() {
        const char filedata[] = "#ifdef X\n"
                                "#else\n"
                                "#ifdef Y\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-DX"));
        ASSERT_EQUALS("\nX=X\nY=Y\n", getConfigsStr(filedata));
    }

    void getConfigsU1() {
        const char filedata[] = "#ifdef X\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX=X\n", getConfigsStr(filedata));
    }

    void getConfigsU2() {
        const char filedata[] = "#ifndef X\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\n", getConfigsStr(filedata));  // no #else
    }

    void getConfigsU3() {
        const char filedata[] = "#ifndef X\n"
                                "Fred & Wilma\n"
                                "#else\n"
                                "Barney & Betty\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX\n", getConfigsStr(filedata));
    }

    void getConfigsU4() {
        const char filedata[] = "#if defined(X) || defined(Y) || defined(Z)\n"
                                "#else\n"
                                "#endif\n";
        ASSERT_EQUALS("\nY=Y;Z=Z\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX=X;Y=Y;Z=Z\n", getConfigsStr(filedata));
    }

    void getConfigsU5() {
        const char filedata[] = "#if X==1\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX=1\n", getConfigsStr(filedata));
    }

    void getConfigsU6() {
        const char filedata[] = "#if X==0\n"
                                "#endif\n";
        ASSERT_EQUALS("\nX=0\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX=0\n", getConfigsStr(filedata));
    }

    void getConfigsU7() {
        const char code[] = "#ifndef Y\n"
                            "#else\n"
                            "#endif\n";
        ASSERT_EQUALS("\nY\n", getConfigsStr(code, "-DX"));
    }

    void getConfigsAndCodeIssue14317() {
        const char filedata[] = "bool test() {\n"
                                "return\n"
                                "#if defined(isless)\n"
                                "0 != isless(1.0, 2.0)\n"
                                "#else\n"
                                "0\n"
                                "#endif\n"
                                ";\n"
                                "}\n";
        // Test getConfigsStr()
        ASSERT_EQUALS("\nisless=isless\n", getConfigsStr(filedata));

        // Test getcode()
        // Preprocess => actual result..
        const std::map<std::string, std::string> actual = getcode(settings0, *this, filedata);

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, actual.size());
        ASSERT_EQUALS("bool test ( ) {\nreturn\n\n\n\n0\n\n;\n}", actual.at(""));
        ASSERT_EQUALS("bool test ( ) {\nreturn\n\n0 != $isless ( 1.0 , 2.0 )\n\n\n\n;\n}", actual.at("isless=isless"));
    }

    void getConfigsMostGeneralConfigIssue14317() {
        // Verifies that the most general X (out of X=X and X) and Y=Y is returned
        // For Z: First Z=Z is added to ret, then the ifndef else branch replaces Z=Z with the more general Z
        const char filedata[] = "#ifdef X\n"
                                "print(X);\n"
                                "#endif\n"
                                "#if X\n"
                                "print(X+1);\n"
                                "#endif\n"
                                "#if defined(Y)\n"
                                "print(Y);\n"
                                "#endif\n"
                                "#ifdef Z\n"
                                "print(Z);\n"
                                "#endif\n"
                                "#ifndef Z\n"
                                "print(Z+1);\n"
                                "#else\n"
                                "print(Z+2);\n"
                                "#endif\n";
        // Test getConfigsStr()
        ASSERT_EQUALS("\nX\nY=Y\nZ\n", getConfigsStr(filedata));
    }

    void if_sizeof() { // #4071
        const char code[] = "#if sizeof(unsigned short) == 2\n"
                            "Fred & Wilma\n"
                            "#elif sizeof(unsigned short) == 4\n"
                            "Fred & Wilma\n"
                            "#else\n"
                            "#endif";

        const std::map<std::string, std::string> actual = getcode(settings0, *this, code);
        ASSERT_EQUALS("\nFred & Wilma", actual.at(""));
    }

    void invalid_ifs() {
        const char filedata[] = "#ifdef\n"
                                "#endif\n"
                                "#ifdef !\n"
                                "#endif\n"
                                "#if defined\n"
                                "#endif\n"
                                "#define f(x) x\n"
                                "#if f(2\n"
                                "#endif\n"
                                "int x;\n";

        // Preprocess => don't crash..
        (void)getcode(settings0, *this, filedata);
        ASSERT_EQUALS(
            "[file.c:1:2]: (error) Syntax error in #ifdef [syntaxError]\n"
            "[file.c:1:2]: (error) Syntax error in #ifdef [syntaxError]\n", errout_str());
    }

    void garbage() {
        const char filedata[] = "V\n"
                                "#define X b   #endif #line 0 \"x\"  ;\n"
                                "#if ! defined ( Y )    #endif";

        // Preprocess => don't crash..
        (void)getcode(settings0, *this, filedata);
    }

    void wrongPathOnErrorDirective() {
        const auto settings = dinit(Settings, $.userDefines = "foo");
        const char code[] = "#error hello world!\n";
        (void)getcodeforcfg(settings, *this, code, "X", "./././test.c");
        ASSERT_EQUALS("[test.c:1:2]: (error) #error hello world! [preprocessorErrorDirective]\n", errout_str());
    }

    // test for existing local include
    void testMissingInclude() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "");

        const char code[] = "#include \"header.h\"";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("", errout_str());
    }

    // test for missing local include
    void testMissingInclude2() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        const char code[] = "#include \"header.h\"";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: \"header.h\" not found. [missingInclude]\n", errout_str());
    }

    // test for missing local include - no include path given
    void testMissingInclude3() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "", "inc");

        const char code[] = "#include \"header.h\"";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: \"header.h\" not found. [missingInclude]\n", errout_str());
    }

    // test for existing local include - include path provided
    void testMissingInclude4() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.includePaths.emplace_back ("inc"),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "", "inc");

        const char code[] = "#include \"inc/header.h\"";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("", errout_str());
    }

    // test for existing local include - absolute path
    void testMissingInclude5() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.includePaths.emplace_back ("inc"),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "", Path::getCurrentPath());

        std::string code("#include \"" + header.path() + "\"");
        (void)getcodeforcfg(settings, *this, code.data(), code.size(), "", "test.c");

        ASSERT_EQUALS("", errout_str());
    }

    // test for missing local include - absolute path
    void testMissingInclude6() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        const std::string header = Path::join(Path::getCurrentPath(), "header.h");

        std::string code("#include \"" + header + "\"");
        (void)getcodeforcfg(settings, *this, code.data(), code.size(), "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: \"" + header + "\" not found. [missingInclude]\n", errout_str());
    }

    // test for missing system include - system includes are not searched for in relative path
    void testMissingSystemInclude() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "");

        const char code[] = "#include <header.h>";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: <header.h> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n", errout_str());
    }

    // test for missing system include
    void testMissingSystemInclude2() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        const char code[] = "#include <header.h>";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: <header.h> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n", errout_str());
    }

    // test for existing system include in system include path
    void testMissingSystemInclude3() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple", // has no effect
                                        $.includePaths.emplace_back ("system")
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "", "system");

        const char code[] = "#include <header.h>";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("", errout_str());
    }

    // test for existing system include - absolute path
    void testMissingSystemInclude4() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.includePaths.emplace_back ("inc");
                                    $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "", Path::getCurrentPath());

        std::string code("#include <" + header.path() + ">");
        (void)getcodeforcfg(settings, *this, code.data(), code.size(), "", "test.c");

        ASSERT_EQUALS("", errout_str());
    }

    // test for missing system include - absolute path
    void testMissingSystemInclude5() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        const std::string header = Path::join(Path::getCurrentPath(), "header.h");

        std::string code("#include <" + header + ">");
        (void)getcodeforcfg(settings, *this, code.data(), code.size(), "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: <" + header + "> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n", errout_str());
    }

    // test for missing local and system include
    void testMissingIncludeMixed() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),

                                        $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "");
        ScopedFile header2("header2.h", "");

        const char code[] = "#include \"missing.h\"\n"
                            "#include <header.h>\n"
                            "#include <missing2.h>\n"
                            "#include \"header2.h\"";
        (void)getcodeforcfg(settings, *this, code, "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: \"missing.h\" not found. [missingInclude]\n"
                      "test.c:2:2: information: Include file: <header.h> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n"
                      "test.c:3:2: information: Include file: <missing2.h> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n", errout_str());
    }

    void testMissingIncludeCheckConfig() {
        const auto settings = dinit(Settings,
                                    $.clearIncludeCache = true,
                                        $.checks.enable (Checks::missingInclude),
                                        $.includePaths.emplace_back ("system");
                                    $.templateFormat = "simple" // has no effect
                                    );
        setTemplateFormat("simple");

        ScopedFile header("header.h", "");
        ScopedFile header2("header2.h", "");
        ScopedFile header3("header3.h", "", "system");
        ScopedFile header4("header4.h", "", "inc");
        ScopedFile header5("header5.h", "", Path::getCurrentPath());
        ScopedFile header6("header6.h", "", Path::getCurrentPath());

        const std::string missing3 = Path::join(Path::getCurrentPath(), "missing3.h");
        const std::string missing4 = Path::join(Path::getCurrentPath(), "missing4.h");

        std::string code("#include \"missing.h\"\n"
                         "#include <header.h>\n"
                         "#include <missing2.h>\n"
                         "#include \"header2.h\"\n"
                         "#include <header3.h>\n"
                         "#include \"header4.h\"\n"
                         "#include \"inc/header4.h\"\n"
                         "#include \"" + header5.path() + "\"\n"
                         "#include \"" + missing3 + "\"\n"
                         "#include <" + header6.path() + ">\n"
                         "#include <" + missing4 + ">\n");
        (void)getcodeforcfg(settings, *this, code.data(), code.size(), "", "test.c");

        ASSERT_EQUALS("test.c:1:2: information: Include file: \"missing.h\" not found. [missingInclude]\n"
                      "test.c:2:2: information: Include file: <header.h> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n"
                      "test.c:3:2: information: Include file: <missing2.h> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n"
                      "test.c:6:2: information: Include file: \"header4.h\" not found. [missingInclude]\n"
                      "test.c:9:2: information: Include file: \"" + missing3 + "\" not found. [missingInclude]\n"
                      "test.c:11:2: information: Include file: <" + missing4 + "> not found. Please note: Standard library headers do not need to be provided to get proper results. [missingIncludeSystem]\n", errout_str());
    }

    void hasInclude() {
        const char code[] = "#if __has_include(<directory/non-existent-header.h>)\n123\n#endif";
        Settings settings;

        settings.standards.setStd("c++11");
        ASSERT_EQUALS("", getcodeforcfg(settings, *this, code, "", "test.cpp"));
        ASSERT_EQUALS("[test.cpp:1:2]: (error) failed to evaluate #if condition, undefined function-like macro invocation: __has_include( ... ) [syntaxError]\n", errout_str()); // TODO: use individual ID

        settings.standards.setStd("c++17");
        ASSERT_EQUALS("", getcodeforcfg(settings, *this, code, "", "test.cpp"));
        ASSERT_EQUALS("", errout_str());

        settings.standards.setStd("gnu++11");
        ASSERT_EQUALS("", getcodeforcfg(settings, *this, code, "", "test.cpp"));
        ASSERT_EQUALS("", errout_str());
    }

    void limitsDefines() {
        // #11928 / #10045
        const char code[] = "void f(long l) {\n"
                            "  if (l > INT_MAX) {}\n"
                            "}";
        const std::string actual = getcodeforcfg(settings0, *this, code, "", "test.c");
        ASSERT_EQUALS("void f ( long l ) {\n"
                      "if ( l > $2147483647 ) { }\n"
                      "}", actual);
    }

    void hashCalculation() {
        // #12383
        const char code[] = "int a;";
        const char code2[] = "int  a;"; // extra space
        const char code3[] = "\n\nint a;"; // extra new line

        ASSERT_EQUALS(getHash(code), getHash(code));
        ASSERT_EQUALS(getHash(code2), getHash(code2));
        ASSERT_EQUALS(getHash(code3), getHash(code3));
        ASSERT(getHash(code) != getHash(code2));
        ASSERT(getHash(code) != getHash(code3));
        ASSERT(getHash(code2) != getHash(code3));
    }

    void standard() {

        const char code[] = "int a;";
        // TODO: this bypasses the standard determined from the settings - the parameter should not be exposed
        simplecpp::DUI dui;

        {
            dui.std = "c89";
            std::vector<std::string> files;
            TokenList tokenlist{settingsDefault, Standards::Language::CPP};
            preprocess(code, files, "test.cpp", tokenlist, dui);
            ASSERT(tokenlist.front());
        }

        {
            dui.std = "gnu23";
            std::vector<std::string> files;
            TokenList tokenlist{settingsDefault, Standards::Language::CPP};
            preprocess(code, files, "test.cpp", tokenlist, dui);
            ASSERT(tokenlist.front());
        }

        {
            dui.std = "c++98";
            std::vector<std::string> files;
            TokenList tokenlist{settingsDefault, Standards::Language::CPP};
            preprocess(code, files, "test.cpp", tokenlist, dui);
            ASSERT(tokenlist.front());
        }

        {
            dui.std = "gnu++26";
            std::vector<std::string> files;
            TokenList tokenlist{settingsDefault, Standards::Language::CPP};
            preprocess(code, files, "test.cpp", tokenlist, dui);
            ASSERT(tokenlist.front());
        }

        {
            dui.std = "gnu77";
            std::vector<std::string> files;
            TokenList tokenlist{settingsDefault, Standards::Language::CPP};
            // TODO: can this happen from application code? if yes we need to turn it into a proper error
            ASSERT_THROW_EQUALS(preprocess(code, files, "test.cpp", tokenlist, dui), std::runtime_error, "unexpected simplecpp::Output type 9");
            ASSERT(!tokenlist.front()); // nothing is tokenized when an unknown standard is provided
        }
    }

    void writeLocations()
    {
        const char inc[] = "class A {\n"
                           "public:\n"
                           "    void f() {}\n"
                           "};";
        const char code[] = R"(#include "test.h")";
        ScopedFile header("test.h", inc);
        const std::string processed = getcodeforcfg(settingsDefault, *this, code, "", "test.cpp");
        ASSERT_EQUALS(
            "\n"
            "#line 1 \"test.h\"\n"
            "class A {\n"
            "public :\n"
            "void f ( ) { }\n"
            "} ;",
            processed);
    }

    void pragmaAsm()
    {
        const char code[] = "#pragma asm";
        ASSERT_THROW_INTERNAL(getcodeforcfg(settingsDefault, *this, code, "", "test.cpp"), InternalError::SYNTAX);
    }
};

REGISTER_TEST(TestPreprocessor)
