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

#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "path.h"
#include "platform.h"
#include "preprocessor.h" // usually tests here should not use preprocessor...
#include "settings.h"
#include "standards.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstdint>
#include <cstring>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <simplecpp.h>

class ErrorLogger;

class TestTokenizer : public TestFixture {
public:
    TestTokenizer() : TestFixture("TestTokenizer") {}

private:
    const Settings settings1 = settingsBuilder().library("std.cfg").debugwarnings().build();
    const Settings settings2 = settingsBuilder(settings1).cpp(Standards::CPP11).c(Standards::C11).build();
    const Settings settings2_win32a = settingsBuilder(settings2).platform(Platform::Type::Win32A).build();
    const Settings settings2_win32w = settingsBuilder(settings2).platform(Platform::Type::Win32W).build();
    const Settings settings2_win64 = settingsBuilder(settings2).platform(Platform::Type::Win64).build();
    const Settings settings2_unix32 = settingsBuilder(settings2).platform(Platform::Type::Unix32).build();
    const Settings settings2_unix64 = settingsBuilder(settings2).platform(Platform::Type::Unix64).build();
    const Settings settings3 = settingsBuilder().c(Standards::C89).cpp(Standards::CPP03).build();
    const Settings settings_windows = settingsBuilder().library("windows.cfg").debugwarnings().cpp(Standards::CPP11).build();
    const Settings settings_win32a = settingsBuilder(settings_windows).platform(Platform::Type::Win32A).build();
    const Settings settings_win32w = settingsBuilder(settings_windows).platform(Platform::Type::Win32W).build();

    void run() override {
        mNewTemplate = true;
        TEST_CASE(tokenize1);
        TEST_CASE(tokenize2);
        TEST_CASE(tokenize4);
        TEST_CASE(tokenize5);
        TEST_CASE(tokenize7);
        TEST_CASE(tokenize8);
        TEST_CASE(tokenize9);
        TEST_CASE(tokenize11);
        TEST_CASE(tokenize13);  // bailout if the code contains "@" - that is not handled well.
        TEST_CASE(tokenize14);  // tokenize "0X10" => 16
        TEST_CASE(tokenizeHexWithSuffix);  // tokenize 0xFFFFFFul
        TEST_CASE(tokenize15);  // tokenize ".123"
        TEST_CASE(tokenize17);  // #2759
        TEST_CASE(tokenize18);  // tokenize "(X&&Y)" into "( X && Y )" instead of "( X & & Y )"
        TEST_CASE(tokenize19);  // #3006 (segmentation fault)
        TEST_CASE(tokenize21);  // tokenize 0x0E-7
        TEST_CASE(tokenize22);  // special marker $ from preprocessor
        TEST_CASE(tokenize25);  // #4239 (segmentation fault)
        TEST_CASE(tokenize26);  // #4245 (segmentation fault)
        TEST_CASE(tokenize27);  // #4525 (segmentation fault)
        TEST_CASE(tokenize31);  // #3503 (Wrong handling of member function taking function pointer as argument)
        TEST_CASE(tokenize32);  // #5884 (fsanitize=undefined: left shift of negative value -10000 in lib/templatesimplifier.cpp:852:46)
        TEST_CASE(tokenize33);  // #5780 Various crashes on valid template code
        TEST_CASE(tokenize34);  // #8031
        TEST_CASE(tokenize35);  // #8361
        TEST_CASE(tokenize36);  // #8436
        TEST_CASE(tokenize37);  // #8550
        TEST_CASE(tokenize38);  // #9569
        TEST_CASE(tokenize39);  // #9771
        TEST_CASE(tokenize40);  // #13181
        TEST_CASE(tokenize41);  // #13847
        TEST_CASE(tokenize42);  // #13861

        TEST_CASE(validate);

        TEST_CASE(objectiveC); // Syntax error should be written for objective C/C++ code.

        TEST_CASE(syntax_case_default);

        TEST_CASE(removePragma);

        TEST_CASE(foreach);     // #3690
        TEST_CASE(ifconstexpr);

        TEST_CASE(combineOperators);

        TEST_CASE(concatenateNegativeNumber);

        TEST_CASE(longtok);

        TEST_CASE(simplifyHeadersAndUnusedTemplates1);
        TEST_CASE(simplifyHeadersAndUnusedTemplates2);

        TEST_CASE(simplifyAt);

        TEST_CASE(inlineasm);
        TEST_CASE(simplifyAsm2);  // #4725 (writing asm() around "^{}")

        TEST_CASE(ifAddBraces1);
        TEST_CASE(ifAddBraces2);
        TEST_CASE(ifAddBraces3);
        TEST_CASE(ifAddBraces4);
        TEST_CASE(ifAddBraces5);
        TEST_CASE(ifAddBraces7);
        TEST_CASE(ifAddBraces9);
        TEST_CASE(ifAddBraces11);
        TEST_CASE(ifAddBraces12);
        TEST_CASE(ifAddBraces13);
        TEST_CASE(ifAddBraces15); // #2616 - unknown macro before if
        TEST_CASE(ifAddBraces16);
        TEST_CASE(ifAddBraces17); // '} else' should be in the same line
        TEST_CASE(ifAddBraces18); // #3424 - if if { } else else
        TEST_CASE(ifAddBraces19); // #3928 - if for if else
        TEST_CASE(ifAddBraces20); // #5012 - syntax error 'else }'
        TEST_CASE(ifAddBracesLabels); // #5332 - if (x) label: {} ..

        TEST_CASE(switchAddBracesLabels);

        TEST_CASE(whileAddBraces);
        TEST_CASE(whileAddBracesLabels);

        TEST_CASE(doWhileAddBraces);
        TEST_CASE(doWhileAddBracesLabels);

        TEST_CASE(forAddBraces1);
        TEST_CASE(forAddBraces2); // #5088
        TEST_CASE(forAddBracesLabels);

        TEST_CASE(simplifyExternC);
        TEST_CASE(simplifyKeyword); // #5842 - remove C99 static keyword between []

        TEST_CASE(isOneNumber);

        TEST_CASE(simplifyFunctionParameters);
        TEST_CASE(simplifyFunctionParameters1); // #3721
        TEST_CASE(simplifyFunctionParameters2); // #4430
        TEST_CASE(simplifyFunctionParameters3); // #4436
        TEST_CASE(simplifyFunctionParameters4); // #9421
        TEST_CASE(simplifyFunctionParametersMultiTemplate);
        TEST_CASE(simplifyFunctionParametersErrors);

        TEST_CASE(simplifyFunctionTryCatch);

        TEST_CASE(removeParentheses1);       // Ticket #61
        TEST_CASE(removeParentheses3);
        TEST_CASE(removeParentheses4);       // Ticket #390
        TEST_CASE(removeParentheses5);       // Ticket #392
        TEST_CASE(removeParentheses6);
        TEST_CASE(removeParentheses7);
        TEST_CASE(removeParentheses8);       // Ticket #1865
        TEST_CASE(removeParentheses9);       // Ticket #1962
        TEST_CASE(removeParentheses10);      // Ticket #2320
        TEST_CASE(removeParentheses11);      // Ticket #2505
        TEST_CASE(removeParentheses12);      // Ticket #2760 ',(b)='
        TEST_CASE(removeParentheses13);
        TEST_CASE(removeParentheses14);      // Ticket #3309
        TEST_CASE(removeParentheses15);      // Ticket #4142
        TEST_CASE(removeParentheses16);      // Ticket #4423 '*(x.y)='
        TEST_CASE(removeParentheses17);      // Don't remove parentheses in 'a ? b : (c>0 ? d : e);'
        TEST_CASE(removeParentheses18);      // 'float(*a)[2]' => 'float *a[2]'
        TEST_CASE(removeParentheses19);      // ((typeof(x) *)0)
        TEST_CASE(removeParentheses20);      // Ticket #5479: a<b<int>>(2);
        TEST_CASE(removeParentheses21);      // Don't "simplify" casts
        TEST_CASE(removeParentheses22);
        TEST_CASE(removeParentheses23);      // Ticket #6103 - Infinite loop upon valid input
        TEST_CASE(removeParentheses24);      // Ticket #7040
        TEST_CASE(removeParentheses25);      // daca@home - a=(b,c)
        TEST_CASE(removeParentheses26);      // Ticket #8875 a[0](0)
        TEST_CASE(removeParentheses27);
        TEST_CASE(removeParentheses28);      // #12164 - don't remove parentheses in '(expr1) ? (expr2) : (expr3);'
        TEST_CASE(removeParantheses29);      // #13735
        TEST_CASE(removeParentheses30);

        TEST_CASE(tokenize_double);
        TEST_CASE(tokenize_strings);

        TEST_CASE(simplifyStructDecl);

        TEST_CASE(vardecl1);
        TEST_CASE(vardecl2);
        TEST_CASE(vardecl3);
        TEST_CASE(vardecl4);
        TEST_CASE(vardecl5);  // #7048
        TEST_CASE(vardec_static);
        TEST_CASE(vardecl6);
        TEST_CASE(vardecl7);
        TEST_CASE(vardecl8);
        TEST_CASE(vardecl9);
        TEST_CASE(vardecl10);
        TEST_CASE(vardecl11);
        TEST_CASE(vardecl12);
        TEST_CASE(vardecl13);
        TEST_CASE(vardecl14);
        TEST_CASE(vardecl15);
        TEST_CASE(vardecl16);
        TEST_CASE(vardecl17);
        TEST_CASE(vardecl18);
        TEST_CASE(vardecl19);
        TEST_CASE(vardecl20);  // #3700 - register const int H = 0;
        TEST_CASE(vardecl21);  // #4042 - a::b const *p = 0;
        TEST_CASE(vardecl22);  // #4211 - segmentation fault
        TEST_CASE(vardecl23);  // #4276 - segmentation fault
        TEST_CASE(vardecl24);  // #4187 - variable declaration within lambda function
        TEST_CASE(vardecl25);  // #4799 - segmentation fault
        TEST_CASE(vardecl26);  // #5907 - incorrect handling of extern declarations
        TEST_CASE(vardecl27);  // #7850 - crash on valid C code
        TEST_CASE(vardecl28);
        TEST_CASE(vardecl29); // #9282
        TEST_CASE(vardecl30);
        TEST_CASE(vardecl31); // function pointer init
        TEST_CASE(vardecl32);
        TEST_CASE(vardecl33);
        TEST_CASE(vardecl34);
        TEST_CASE(vardecl_stl_1);
        TEST_CASE(vardecl_stl_2);
        TEST_CASE(vardecl_stl_3);
        TEST_CASE(vardecl_template_1);
        TEST_CASE(vardecl_template_2);
        TEST_CASE(vardecl_union);
        TEST_CASE(vardecl_par);     // #2743 - set links if variable type contains parentheses
        TEST_CASE(vardecl_par2);    // #3912 - set correct links
        TEST_CASE(vardecl_par3);    // #6556 - Fred x1(a), x2(b);
        TEST_CASE(vardecl_class_ref);
        TEST_CASE(volatile_variables);

        // unsigned i; => unsigned int i;
        TEST_CASE(implicitIntConst);
        TEST_CASE(implicitIntExtern);
        TEST_CASE(implicitIntSigned1);
        TEST_CASE(implicitIntUnsigned1);
        TEST_CASE(implicitIntUnsigned2);
        TEST_CASE(implicitIntUnsigned3);   // template arguments

        TEST_CASE(simplifyStdType); // #4947, #4950, #4951

        TEST_CASE(createLinks);
        TEST_CASE(createLinks2);

        TEST_CASE(simplifyString);
        TEST_CASE(simplifyConst);
        TEST_CASE(switchCase);

        TEST_CASE(simplifyPointerToStandardType);
        TEST_CASE(simplifyFunctionPointers1);
        TEST_CASE(simplifyFunctionPointers2);
        TEST_CASE(simplifyFunctionPointers3);
        TEST_CASE(simplifyFunctionPointers4);
        TEST_CASE(simplifyFunctionPointers5);
        TEST_CASE(simplifyFunctionPointers6);
        TEST_CASE(simplifyFunctionPointers7);
        TEST_CASE(simplifyFunctionPointers8); // #7410 - throw
        TEST_CASE(simplifyFunctionPointers9); // #6113 - function call with function pointer

        TEST_CASE(removedeclspec);
        TEST_CASE(removeattribute);
        TEST_CASE(functionAttributeBefore1);
        TEST_CASE(functionAttributeBefore2);
        TEST_CASE(functionAttributeBefore3);
        TEST_CASE(functionAttributeBefore4);
        TEST_CASE(functionAttributeBefore5); // __declspec(dllexport)
        TEST_CASE(functionAttributeAfter1);
        TEST_CASE(functionAttributeAfter2);
        TEST_CASE(functionAttributeListBefore);
        TEST_CASE(functionAttributeListAfter);
        TEST_CASE(functionAttributeListAfter2);
        TEST_CASE(cppMaybeUnusedBefore);
        TEST_CASE(cppMaybeUnusedAfter1);
        TEST_CASE(cppMaybeUnusedAfter2);
        TEST_CASE(cppMaybeUnusedStructuredBinding);

        TEST_CASE(attributeAlignasBefore);
        TEST_CASE(attributeAlignasAfter);

        TEST_CASE(splitTemplateRightAngleBrackets);

        TEST_CASE(cpp03template1);
        TEST_CASE(cpp0xtemplate1);
        TEST_CASE(cpp0xtemplate2);
        TEST_CASE(cpp0xtemplate3);
        TEST_CASE(cpp0xtemplate4); // Ticket #6181: Mishandled C++11 syntax
        TEST_CASE(cpp0xtemplate5); // Ticket #9154 change >> to > >
        TEST_CASE(cpp14template); // Ticket #6708

        TEST_CASE(arraySize);
        TEST_CASE(arraySizeAfterValueFlow);

        TEST_CASE(labels);
        TEST_CASE(simplifyInitVar);
        TEST_CASE(simplifyInitVar2);
        TEST_CASE(simplifyInitVar3);
        TEST_CASE(simplifyInitVar4);
        TEST_CASE(simplifyInitVar5);

        TEST_CASE(bitfields1);
        TEST_CASE(bitfields2);
        TEST_CASE(bitfields3);
        TEST_CASE(bitfields4); // ticket #1956
        TEST_CASE(bitfields5); // ticket #1956
        TEST_CASE(bitfields6); // ticket #2595
        TEST_CASE(bitfields7); // ticket #1987
        TEST_CASE(bitfields8);
        TEST_CASE(bitfields9); // ticket #2706
        TEST_CASE(bitfields10);
        TEST_CASE(bitfields12); // ticket #3485 (segmentation fault)
        TEST_CASE(bitfields13); // ticket #3502 (segmentation fault)
        TEST_CASE(bitfields15); // ticket #7747 (enum Foo {A,B}:4;)
        TEST_CASE(bitfields16); // Save bitfield bit count
        TEST_CASE(bitfields17);
        TEST_CASE(bitfields18);
        TEST_CASE(bitfields19); // ticket #13733
        TEST_CASE(bitfields20);
        TEST_CASE(bitfields21);

        TEST_CASE(simplifyNamespaceStd);

        TEST_CASE(microsoftMemory);
        TEST_CASE(microsoftString);

        TEST_CASE(borland);
        TEST_CASE(simplifySQL);

        TEST_CASE(simplifyCAlternativeTokens);

        // x = ({ 123; });  =>  { x = 123; }
        TEST_CASE(simplifyCompoundStatements);

        TEST_CASE(simplifyOperatorName1);
        TEST_CASE(simplifyOperatorName2);
        TEST_CASE(simplifyOperatorName3);
        TEST_CASE(simplifyOperatorName4);
        TEST_CASE(simplifyOperatorName5);
        TEST_CASE(simplifyOperatorName6); // ticket #3194
        TEST_CASE(simplifyOperatorName7); // ticket #4619
        TEST_CASE(simplifyOperatorName8); // ticket #5706
        TEST_CASE(simplifyOperatorName9); // ticket #5709 - comma operator not properly tokenized
        TEST_CASE(simplifyOperatorName10); // #8746 - using a::operator=
        TEST_CASE(simplifyOperatorName11); // #8889
        TEST_CASE(simplifyOperatorName12); // #9110
        TEST_CASE(simplifyOperatorName13); // user defined literal
        TEST_CASE(simplifyOperatorName14); // std::complex operator "" if
        TEST_CASE(simplifyOperatorName15); // ticket #9468 syntaxError
        TEST_CASE(simplifyOperatorName16); // ticket #9472
        TEST_CASE(simplifyOperatorName17);
        TEST_CASE(simplifyOperatorName18); // global namespace
        TEST_CASE(simplifyOperatorName19);
        TEST_CASE(simplifyOperatorName20);
        TEST_CASE(simplifyOperatorName21);
        TEST_CASE(simplifyOperatorName22);
        TEST_CASE(simplifyOperatorName23);
        TEST_CASE(simplifyOperatorName24);
        TEST_CASE(simplifyOperatorName25);
        TEST_CASE(simplifyOperatorName26);
        TEST_CASE(simplifyOperatorName27);
        TEST_CASE(simplifyOperatorName28);
        TEST_CASE(simplifyOperatorName29); // spaceship operator
        TEST_CASE(simplifyOperatorName30);
        TEST_CASE(simplifyOperatorName31); // #6342
        TEST_CASE(simplifyOperatorName32); // #10256
        TEST_CASE(simplifyOperatorName33); // #10138

        TEST_CASE(simplifyOverloadedOperators1);
        TEST_CASE(simplifyOverloadedOperators2); // (*this)(123)
        TEST_CASE(simplifyOverloadedOperators3); // #9881 - hang

        TEST_CASE(simplifyNullArray);

        // Some simple cleanups of unhandled macros in the global scope
        TEST_CASE(removeMacrosInGlobalScope);

        TEST_CASE(addSemicolonAfterUnknownMacro);

        // a = b = 0;
        TEST_CASE(multipleAssignment);

        TEST_CASE(platformWin);
        TEST_CASE(platformWin32A);
        TEST_CASE(platformWin32W);
        TEST_CASE(platformWin32AStringCat); // ticket #5015
        TEST_CASE(platformWin32WStringCat); // ticket #5015
        TEST_CASE(platformWinWithNamespace);

        TEST_CASE(simplifyStaticConst);

        TEST_CASE(simplifyCPPAttribute);

        TEST_CASE(simplifyCaseRange);

        TEST_CASE(simplifyEmptyNamespaces);

        TEST_CASE(prepareTernaryOpForAST);

        // AST data
        TEST_CASE(astexpr);
        TEST_CASE(astexpr2); // limit large expressions
        TEST_CASE(astpar);
        TEST_CASE(astnewdelete);
        TEST_CASE(astbrackets);
        TEST_CASE(astunaryop);
        TEST_CASE(astfunction);
        TEST_CASE(asttemplate);
        TEST_CASE(astrequires);
        TEST_CASE(astcast);
        TEST_CASE(astlambda);
        TEST_CASE(astcase);
        TEST_CASE(astrefqualifier);
        TEST_CASE(astthrowdelete);
        TEST_CASE(asttrailingdecltype);
        TEST_CASE(astnoexcept);
        TEST_CASE(astvardecl);
        TEST_CASE(astnewscoped);
        TEST_CASE(astdecltypescope);
        TEST_CASE(astdesignatedinit);
        TEST_CASE(astrvaluedecl);
        TEST_CASE(astorkeyword);
        TEST_CASE(astenumdecl);
        TEST_CASE(astcompound);
        TEST_CASE(astfuncdecl);

        TEST_CASE(startOfExecutableScope);

        TEST_CASE(removeMacroInClassDef); // #6058

        TEST_CASE(sizeofAddParentheses);

        TEST_CASE(reportUnknownMacros);

        // Make sure the Tokenizer::findGarbageCode() does not have false positives
        // The TestGarbage ensures that there are true positives
        TEST_CASE(findGarbageCode);
        TEST_CASE(checkEnableIf);
        TEST_CASE(checkTemplates);
        TEST_CASE(checkNamespaces);
        TEST_CASE(checkLambdas);
        TEST_CASE(checkIfCppCast);
        TEST_CASE(checkRefQualifiers);
        TEST_CASE(checkConditionBlock);
        TEST_CASE(checkUnknownCircularVar);
        TEST_CASE(checkRequires);

        // #9052
        TEST_CASE(noCrash1);
        TEST_CASE(noCrash2);
        TEST_CASE(noCrash3);
        TEST_CASE(noCrash4);
        TEST_CASE(noCrash5); // #10603
        TEST_CASE(noCrash6); // #10212
        TEST_CASE(noCrash7);

        // --check-config
        TEST_CASE(checkConfiguration);

        TEST_CASE(unknownMacroBeforeReturn);

        TEST_CASE(cppKeywordInCSource);

        TEST_CASE(cppcast);

        TEST_CASE(checkHeader1);

        TEST_CASE(removeExtraTemplateKeywords);

        TEST_CASE(removeAlignas1);
        TEST_CASE(removeAlignas2); // Do not remove alignof in the same way
        TEST_CASE(removeAlignas3); // remove alignas in C11 code
        TEST_CASE(dumpAlignas);

        TEST_CASE(simplifyCoroutines);

        TEST_CASE(simplifySpaceshipOperator);

        TEST_CASE(simplifyIfSwitchForInit1);
        TEST_CASE(simplifyIfSwitchForInit2);
        TEST_CASE(simplifyIfSwitchForInit3);
        TEST_CASE(simplifyIfSwitchForInit4);
        TEST_CASE(simplifyIfSwitchForInit5);

        TEST_CASE(newPlacementArgsCppInit); // #13775

        TEST_CASE(cpp20_default_bitfield_initializer);

        TEST_CASE(cpp11init);

        TEST_CASE(testDirectiveIncludeTypes);
        TEST_CASE(testDirectiveIncludeLocations);
        TEST_CASE(testDirectiveIncludeComments);
        TEST_CASE(testDirectiveRelativePath);

        TEST_CASE(funcnameInParenthesis1); // #13554
        TEST_CASE(funcnameInParenthesis2); // #13578
        TEST_CASE(funcnameInParenthesis3); // #13585

        TEST_CASE(genericInIf); // #13561

        TEST_CASE(preincrementInLambda); // #13312

        TEST_CASE(atomicCast); // #12605
        TEST_CASE(constFunctionPtrTypedef); // #12135

        TEST_CASE(simplifyPlatformTypes);

        TEST_CASE(dumpFallthrough);

        TEST_CASE(simplifyRedundantParentheses);

        TEST_CASE(simplifyEnum1);

        TEST_CASE(simplifyEnum2);
    }

    class TokenizerTest final : public Tokenizer
    {
        friend class TestTokenizer;
    public:
        TokenizerTest(TokenList tokenList, ErrorLogger &errorLogger)
            : Tokenizer(std::move(tokenList), errorLogger)
        {}
    };

    struct TokenizeOptions
    {
        bool expand = true;
        bool cpp = true;
    };

#define tokenizeAndStringify(...) tokenizeAndStringify_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tokenizeAndStringify_(const char* file, int linenr, const char (&code)[size], const TokenizeOptions& opt = make_default_obj{}) {
        // tokenize..
        SimpleTokenizer tokenizer(settings2, *this, opt.cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, linenr);

        if (tokenizer.tokens())
            return tokenizer.tokens()->stringifyList(false, opt.expand, false, true, false, nullptr, nullptr);
        return "";
    }

    // TODO: get rid of this
    std::string tokenizeAndStringify_(const char* file, int linenr, const std::string& code) {
        SimpleTokenizer tokenizer(settings2, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, linenr);

        if (tokenizer.tokens())
            return tokenizer.tokens()->stringifyList(false, true, false, true, false, nullptr, nullptr);
        return "";
    }

    template<size_t size>
    std::string tokenizeAndStringify_(const char* file, int line, const char (&code)[size], const Settings &settings, bool cpp = true, bool expand = true) {
        // tokenize..
        SimpleTokenizer tokenizer(settings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);
        if (!tokenizer.tokens())
            return "";
        return tokenizer.tokens()->stringifyList(false, expand, false, true, false, nullptr, nullptr);
    }

#define tokenizeDebugListing(...) tokenizeDebugListing_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string tokenizeDebugListing_(const char* file, int line, const char (&code)[size], bool cpp = true) {
        SimpleTokenizer tokenizer(settings3, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        // result..
        return tokenizer.tokens()->stringifyList(true,true,true,true,false);
    }

    template<size_t size>
    void directiveDump(const char (&code)[size], std::ostream& ostr) {
        directiveDump(code, "test.c", settingsDefault, ostr);
    }

    template<size_t size>
    void directiveDump(const char (&code)[size], const char filename[], const Settings& settings, std::ostream& ostr) {
        simplecpp::OutputList outputList;
        std::vector<std::string> files;
        simplecpp::TokenList tokens1(code, files, filename, &outputList);
        Preprocessor preprocessor(tokens1, settings, *this, Path::identify(tokens1.getFiles()[0], false));
        (void)preprocessor.reportOutput(outputList, true);
        ASSERT(preprocessor.loadFiles(files));
        std::list<Directive> directives = preprocessor.createDirectives();

        TokenList tokenlist{settings, Path::identify(filename, false)};
        Tokenizer tokenizer(std::move(tokenlist), *this);
        tokenizer.setDirectives(std::move(directives));

        tokenizer.dump(ostr);
    }

    void tokenize1() {
        const char code[] = "void f ( )\n"
                            "{ if ( p . y ( ) > yof ) { } }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:2:20]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable yof [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void tokenize2() {
        const char code[] = "{ sizeof a, sizeof b }";
        ASSERT_EQUALS("{ sizeof ( a ) , sizeof ( b ) }", tokenizeAndStringify(code));
    }

    void tokenize4() {
        const char code[] = "class foo\n"
                            "{\n"
                            "public:\n"
                            "    const int i;\n"
                            "}";
        ASSERT_EQUALS("class foo\n"
                      "{\n"
                      "public:\n"
                      "const int i ;\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());
    }

    void tokenize5() {
        // Tokenize values
        ASSERT_EQUALS("; + 1E3 ;", tokenizeAndStringify("; +1E3 ;"));
        ASSERT_EQUALS("; 1E-2 ;", tokenizeAndStringify("; 1E-2 ;"));
    }

    void tokenize7() {
        const char code[] = "void f() {\n"
                            "    int x1 = 1;\n"
                            "    int x2(x1);\n"
                            "}\n";
        ASSERT_EQUALS("void f ( ) {\nint x1 ; x1 = 1 ;\nint x2 ( x1 ) ;\n}",
                      tokenizeAndStringify(code));
    }

    void tokenize8() {
        const char code[] = "void f() {\n"
                            "    int x1(g());\n"
                            "    int x2(x1);\n"
                            "}\n";
        ASSERT_EQUALS("1: void f ( ) {\n"
                      "2: int x1@1 ( g ( ) ) ;\n"
                      "3: int x2@2 ( x1@1 ) ;\n"
                      "4: }\n",
                      tokenizeDebugListing(code));
    }

    void tokenize9() {
        const char code[] = "typedef void (*fp)();\n"
                            "typedef fp (*fpp)();\n"
                            "void f() {\n"
                            "    fpp x = (fpp)f();\n"
                            "}";
        (void)tokenizeAndStringify(code);
        ASSERT_EQUALS("", errout_str());
    }

    void tokenize11() {
        ASSERT_EQUALS("X * sizeof ( Y ( ) ) ;", tokenizeAndStringify("X * sizeof(Y());"));
    }

    // bailout if there is "@" - it is not handled well
    void tokenize13() {
        const char code[] = "@implementation\n"
                            "-(Foo *)foo: (Bar *)bar\n"
                            "{ }\n"
                            "@end\n";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code), SYNTAX);
    }

    // Ticket #2361: 0X10 => 16
    void tokenize14() {
        ASSERT_EQUALS("; 0x10 ;", tokenizeAndStringify(";0x10;"));
        ASSERT_EQUALS("; 0X10 ;", tokenizeAndStringify(";0X10;"));
        ASSERT_EQUALS("; 0444 ;", tokenizeAndStringify(";0444;"));
    }

    // Ticket #8050
    void tokenizeHexWithSuffix() {
        ASSERT_EQUALS("; 0xFFFFFF ;", tokenizeAndStringify(";0xFFFFFF;"));
        ASSERT_EQUALS("; 0xFFFFFFu ;", tokenizeAndStringify(";0xFFFFFFu;"));
        ASSERT_EQUALS("; 0xFFFFFFul ;", tokenizeAndStringify(";0xFFFFFFul;"));

        // Number of digits decides about internal representation...
        ASSERT_EQUALS("; 0xFFFFFFFF ;", tokenizeAndStringify(";0xFFFFFFFF;"));
        ASSERT_EQUALS("; 0xFFFFFFFFu ;", tokenizeAndStringify(";0xFFFFFFFFu;"));
        ASSERT_EQUALS("; 0xFFFFFFFFul ;", tokenizeAndStringify(";0xFFFFFFFFul;"));
    }

    // Ticket #2429: 0.125
    void tokenize15() {
        ASSERT_EQUALS("0.125 ;", tokenizeAndStringify(".125;"));
        ASSERT_EQUALS("005.125 ;", tokenizeAndStringify("005.125;")); // Don't confuse with octal values
    }

    void tokenize17() { // #2759
        ASSERT_EQUALS("class B : private :: A { } ;", tokenizeAndStringify("class B : private ::A { };"));
    }

    void tokenize18() { // tokenize "(X&&Y)" into "( X && Y )" instead of "( X & & Y )"
        ASSERT_EQUALS("( X && Y ) ;", tokenizeAndStringify("(X&&Y);"));
    }

    void tokenize19() {
        // #3006 - added hasComplicatedSyntaxErrorsInTemplates to avoid segmentation fault
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("x < () <"), SYNTAX);

        // #3496 - make sure hasComplicatedSyntaxErrorsInTemplates works
        ASSERT_EQUALS("void a ( Fred * f ) { for ( ; n < f . x ( ) ; ) { } }",
                      tokenizeAndStringify("void a(Fred* f) MACRO { for (;n < f->x();) {} }"));

        // #6216 - make sure hasComplicatedSyntaxErrorsInTemplates works
        ASSERT_EQUALS("C :: C ( )\n"
                      ": v { }\n"
                      "{\n"
                      "for ( int dim = 0 ; dim < v . size ( ) ; ++ dim ) {\n"
                      "v [ dim ] . f ( ) ;\n"
                      "}\n"
                      "} ;",
                      tokenizeAndStringify("C::C()\n"
                                           ":v{}\n"
                                           "{\n"
                                           "    for (int dim = 0; dim < v.size(); ++dim) {\n"
                                           "        v[dim]->f();\n"
                                           "    }\n"
                                           "};"));
        ignore_errout(); // we do not care about the output
    }

    void tokenize21() { // tokenize 0x0E-7
        ASSERT_EQUALS("0x0E - 7 ;", tokenizeAndStringify("0x0E-7;"));
    }

    void tokenize22() { // tokenize special marker $ from preprocessor
        ASSERT_EQUALS("a$b", tokenizeAndStringify("a$b"));
        ASSERT_EQUALS("a $b\nc", tokenizeAndStringify("a $b\nc"));
        ASSERT_EQUALS("a = $0 ;", tokenizeAndStringify("a = $0;"));
        ASSERT_EQUALS("a$ ++ ;", tokenizeAndStringify("a$++;"));
        ASSERT_EQUALS("$if ( ! p )", tokenizeAndStringify("$if(!p)"));
    }

    // #4239 - segfault for "f ( struct { int typedef T x ; } ) { }"
    void tokenize25() {
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("f ( struct { int typedef T x ; } ) { }"), SYNTAX);
    }

    // #4245 - segfault
    void tokenize26() {
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("class x { protected : template < int y = } ;"), SYNTAX); // Garbage code
    }

    void tokenize27() {
        // #4525 - segfault
        (void)tokenizeAndStringify("struct except_spec_d_good : except_spec_a, except_spec_b {\n"
                                   "~except_spec_d_good();\n"
                                   "};\n"
                                   "struct S { S(); };\n"
                                   "S::S() __attribute((pure)) = default;"
                                   );

        // original code: glibc-2.18/posix/bug-regex20.c
        (void)tokenizeAndStringify("static unsigned int re_string_context_at (const re_string_t *input, int idx, int eflags) internal_function __attribute__ ((pure));");
    }

    // #3503 - don't "simplify" SetFunction member function to a variable
    void tokenize31() {
        ASSERT_EQUALS("struct TTestClass { TTestClass ( ) { }\n"
                      "void SetFunction ( Other ( * m_f ) ( ) ) { }\n"
                      "} ;",
                      tokenizeAndStringify("struct TTestClass { TTestClass() { }\n"
                                           "    void SetFunction(Other(*m_f)()) { }\n"
                                           "};"));

        ASSERT_EQUALS("struct TTestClass { TTestClass ( ) { }\n"
                      "void SetFunction ( Other ( * m_f ) ( ) ) ;\n"
                      "} ;",
                      tokenizeAndStringify("struct TTestClass { TTestClass() { }\n"
                                           "    void SetFunction(Other(*m_f)());\n"
                                           "};"));
    }

    // #5884 - Avoid left shift of negative integer value.
    void tokenize32() {
        // Do not simplify negative integer left shifts.
        const char code[] = "void f ( ) { int max_x ; max_x = -10000 << 16 ; }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    // #5780 Various crashes on valid template code in Tokenizer::setVarId()
    void tokenize33() {
        const char code[] = "template<typename T, typename A = Alloc<T>> struct vector {};\n"
                            "void z() {\n"
                            "    vector<int> VI;\n"
                            "}\n";
        (void)tokenizeAndStringify(code);
    }

    void tokenize34() { // #8031
        {
            const char code[] = "struct Container {\n"
                                "  Container();\n"
                                "  int* mElements;\n"
                                "};\n"
                                "Container::Container() : mElements(nullptr) {}\n"
                                "Container intContainer;";
            const char exp[] = "1: struct Container {\n"
                               "2: Container ( ) ;\n"
                               "3: int * mElements@1 ;\n"
                               "4: } ;\n"
                               "5: Container :: Container ( ) : mElements@1 ( nullptr ) { }\n"
                               "6: Container intContainer@2 ;\n";
            ASSERT_EQUALS(exp, tokenizeDebugListing(code));
        }
        {
            const char code[] = "template<class T> struct Container {\n"
                                "  Container();\n"
                                "  int* mElements;\n"
                                "};\n"
                                "template <class T> Container<T>::Container() : mElements(nullptr) {}\n"
                                "Container<int> intContainer;";
            const char exp[] = "1: struct Container<int> ;\n"
                               "2:\n"
                               "|\n"
                               "5:\n"
                               "6: Container<int> intContainer@1 ;\n"
                               "1: struct Container<int> {\n"
                               "2: Container<int> ( ) ;\n"
                               "3: int * mElements@2 ;\n"
                               "4: } ;\n"
                               "5: Container<int> :: Container<int> ( ) : mElements@2 ( nullptr ) { }\n";
            ASSERT_EQUALS(exp, tokenizeDebugListing(code));
        }
    }

    void tokenize35() { // #8361
        ASSERT_NO_THROW(tokenizeAndStringify("typedef int CRCWord; "
                                             "template<typename T> ::CRCWord const Compute(T const t) { return 0; }"));
    }

    void tokenize36() { // #8436
        const char code[] = "int foo ( int i ) { return i ? * new int { 5 } : int { i ? 0 : 1 } ; }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void tokenize37() { // #8550
        const char codeC[] = "class name { public: static void init ( ) {} } ; "
                             "typedef class name N; "
                             "void foo ( ) { return N :: init ( ) ; }";
        const char expC[] = "class name { public: static void init ( ) { } } ; "
                            "void foo ( ) { return name :: init ( ) ; }";
        ASSERT_EQUALS(expC, tokenizeAndStringify(codeC));
        const char codeS[] = "class name { public: static void init ( ) {} } ; "
                             "typedef struct name N; "
                             "void foo ( ) { return N :: init ( ) ; }";
        const char expS[] = "class name { public: static void init ( ) { } } ; "
                            "void foo ( ) { return name :: init ( ) ; }";
        ASSERT_EQUALS(expS, tokenizeAndStringify(codeS));
    }

    void tokenize38() { // #9569
        const char code[] = "using Binary = std::vector<char>; enum Type { Binary };";
        const char exp[]  = "enum Type { Binary } ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void tokenize39() { // #9771
        const char code[] = "template <typename T> class Foo;"
                            "template <typename T> bool operator!=(const Foo<T> &, const Foo<T> &);"
                            "template <typename T> class Foo { friend bool operator!= <> (const Foo<T> &, const Foo<T> &); };";
        const char exp[]  = "template < typename T > class Foo ; "
                            "template < typename T > bool operator!= ( const Foo < T > & , const Foo < T > & ) ; "
                            "template < typename T > class Foo { friend bool operator!= < > ( const Foo < T > & , const Foo < T > & ) ; } ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void tokenize40() { // #13181
        const char code[] = "struct A { double eps(double); };\n"
                            "A operator \"\"_a(long double);\n"
                            "void f() {\n"
                            "    double d = 1.23;\n"
                            "    if (d == 1.2_a .eps(.1)) {}\n"
                            "}\n";
        (void) tokenizeAndStringify(code);
        ASSERT_EQUALS("", errout_str());
    }

    void tokenize41() { // #13847
        const char code[] = "int main() {\n"
                            "     using x::a;\n"
                            "     b = a + 2;\n"
                            "}\n";
        ASSERT_EQUALS("int main ( ) {\n"
                      "\n"
                      "b = x :: a + 2 ;\n"
                      "}", tokenizeAndStringify(code));
        (void)errout_str();
    }

    void tokenize42() { // #13861
        const char code[] = "struct AB { int a; int b; };\n"
                            "namespace ns { typedef AB S[10]; }\n"
                            "void foo(void) {\n"
                            "    ns::S x = {0};\n"
                            "    x[1].a = 2;\n"
                            "}\n";
        ASSERT_EQUALS("struct AB { int a ; int b ; } ;\n"
                      "\n"
                      "void foo ( ) {\n"
                      "AB x [ 10 ] = { 0 } ;\n"
                      "x [ 1 ] . a = 2 ;\n"
                      "}", tokenizeAndStringify(code));
        (void)errout_str();
    }

    void validate() {
        // C++ code in C file
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(";using namespace std;",dinit(TokenizeOptions, $.expand = false, $.cpp = false)), SYNTAX);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(";std::map<int,int> m;",dinit(TokenizeOptions, $.expand = false, $.cpp = false)), SYNTAX);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(";template<class T> class X { };",dinit(TokenizeOptions, $.expand = false, $.cpp = false)), SYNTAX);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("int X<Y>() {};",dinit(TokenizeOptions, $.expand = false, $.cpp = false)), SYNTAX);
        {
            TokenList tokenlist{settings1, Standards::Language::C}; // headers are treated as C files
            const char code[] = "void foo(int i) { reinterpret_cast<char>(i) };";
            tokenlist.appendFileIfNew("test.h");
            ASSERT(tokenlist.createTokensFromString(code));
            Tokenizer tokenizer(std::move(tokenlist), *this);
            ASSERT_THROW_INTERNAL(tokenizer.simplifyTokens1(""), SYNTAX);
        }
    }

    void objectiveC() {
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void f() { [foo bar]; }"), SYNTAX);
    }

    void syntax_case_default() { // correct syntax
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case 0: z(); break;}}"));
        ASSERT_EQUALS("", errout_str());

        (void)tokenizeAndStringify("void f(int n) {switch (n) { case 0:; break;}}");
        ASSERT_EQUALS("", errout_str());

        // TODO: Do not throw AST validation exception
        TODO_ASSERT_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case 0?1:2 : z(); break;}}"), InternalError);
        //ASSERT_EQUALS("", errout_str());

        // TODO: Do not throw AST validation exception
        TODO_ASSERT_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case 0?(1?3:4):2 : z(); break;}}"), InternalError);
        ASSERT_EQUALS("", errout_str());

        //allow GCC '({ %name%|%num%|%bool% ; })' statement expression extension
        // TODO: Do not throw AST validation exception
        TODO_ASSERT_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case 0?({0;}):1: z(); break;}}"), InternalError);
        ASSERT_EQUALS("", errout_str());

        //'b' can be or a macro or an undefined enum
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case b: z(); break;}}"));
        ASSERT_EQUALS("", errout_str());

        //valid, when there's this declaration: 'constexpr int g() { return 2; }'
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case g(): z(); break;}}"));
        ASSERT_EQUALS("", errout_str());

        //valid, when there's also this declaration: 'constexpr int g[1] = {0};'
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case g[0]: z(); break;}}"));
        ASSERT_EQUALS(
            "[test.cpp:1:34]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable g [valueFlowBailoutIncompleteVar]\n",
            errout_str());

        //valid, similar to above case
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case *g: z(); break;}}"));
        ASSERT_EQUALS("", errout_str());

        //valid, when 'x' and 'y' are constexpr.
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int n) {switch (n) { case sqrt(x+y): z(); break;}}"));
        ASSERT_EQUALS(
            "[test.cpp:1:39]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void removePragma() {
        const char code[] = "_Pragma(\"abc\") int x;";
        const Settings s_c89 = settingsBuilder().c(Standards::C89).build();
        ASSERT_EQUALS("_Pragma ( \"abc\" ) int x ;", tokenizeAndStringify(code, s_c89, false));
        const Settings s_clatest;
        ASSERT_EQUALS("int x ;", tokenizeAndStringify(code, s_clatest, false));

        const Settings s_cpp03 = settingsBuilder().cpp(Standards::CPP03).build();
        ASSERT_EQUALS("_Pragma ( \"abc\" ) int x ;", tokenizeAndStringify(code, s_cpp03, true));
        const Settings s_cpplatest;
        ASSERT_EQUALS("int x ;", tokenizeAndStringify(code, s_cpplatest, true));
    }

    void foreach () {
        // #3690,#5154
        const char code[] ="void f() { for each ( char c in MyString ) { Console::Write(c); } }";
        ASSERT_EQUALS("void f ( ) { for ( char c : MyString ) { Console :: Write ( c ) ; } }", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:1:33]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable MyString [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void ifconstexpr() {
        ASSERT_EQUALS("void f ( ) { if ( FOO ) { bar ( c ) ; } }", tokenizeAndStringify("void f() { if constexpr ( FOO ) { bar(c); } }"));
        ASSERT_EQUALS(
            "[test.cpp:1:27]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable FOO [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void combineOperators() {
        ASSERT_EQUALS("; private: ;", tokenizeAndStringify(";private:;"));
        ASSERT_EQUALS("; protected: ;", tokenizeAndStringify(";protected:;"));
        ASSERT_EQUALS("; public: ;", tokenizeAndStringify(";public:;"));
        ASSERT_EQUALS("; __published: ;", tokenizeAndStringify(";__published:;"));
        ASSERT_EQUALS("a . public : ;", tokenizeAndStringify("a.public:;"));
        ASSERT_EQUALS("void f ( x & = 2 ) ;", tokenizeAndStringify("void f(x &= 2);"));
        ASSERT_EQUALS("const_cast < a * > ( & e )", tokenizeAndStringify("const_cast<a*>(&e)"));
    }

    void concatenateNegativeNumber() {
        ASSERT_EQUALS("i = -12 ;", tokenizeAndStringify("i = -12;"));
        ASSERT_EQUALS("1 - 2 ;", tokenizeAndStringify("1-2;"));
        ASSERT_EQUALS("foo ( -1 ) - 2 ;", tokenizeAndStringify("foo(-1)-2;"));
        ASSERT_EQUALS("int f ( ) { return -2 ; }", tokenizeAndStringify("int f(){return -2;}"));
        ASSERT_EQUALS("int x [ 2 ] = { -2 , 1 }", tokenizeAndStringify("int x[2] = {-2,1}"));

        ASSERT_EQUALS("f ( 123 )", tokenizeAndStringify("f(+123)"));

        ASSERT_EQUALS("std :: extent_v < A > - 1 ;", tokenizeAndStringify("std::extent_v<A> - 1;")); // #11341
    }



    void longtok() {
        const std::string filedata(10000, 'a');
        ASSERT_EQUALS(filedata, tokenizeAndStringify(filedata));
    }


    void simplifyHeadersAndUnusedTemplates1() {
        const Settings s = settingsBuilder().checkUnusedTemplates(false).build();
        ASSERT_EQUALS(";",
                      tokenizeAndStringify("; template <typename... a> uint8_t b(std::tuple<uint8_t> d) {\n"
                                           "  std::tuple<a...> c{std::move(d)};\n"
                                           "  return std::get<0>(c);\n"
                                           "}", s));
        ASSERT_EQUALS("int g ( int ) ;",
                      tokenizeAndStringify("int g(int);\n"
                                           "template <class F, class... Ts> auto h(F f, Ts... xs) {\n"
                                           "    auto e = f(g(xs)...);\n"
                                           "    return e;\n"
                                           "}", s));
    }

    void simplifyHeadersAndUnusedTemplates2() {
        const char code[] = "; template< typename T, u_int uBAR = 0 >\n"
                            "class Foo {\n"
                            "public:\n"
                            "    void FooBar() {\n"
                            "        new ( (uBAR ? uBAR : sizeof(T))) T;\n"
                            "    }\n"
                            "};";

        {
            const Settings s = settingsBuilder().checkUnusedTemplates(false).build();
            ASSERT_EQUALS(";", tokenizeAndStringify(code, s));
        }

        {
            ASSERT_EQUALS("; template < typename T , u_int uBAR = 0 >\n"
                          "class Foo {\n"
                          "public:\n"
                          "void FooBar ( ) {\n"
                          "new ( uBAR ? uBAR : sizeof ( T ) ) T ;\n"
                          "}\n"
                          "} ;", tokenizeAndStringify(code, settingsDefault));
        }
    }

    void simplifyAt() {
        ASSERT_EQUALS("int x ;", tokenizeAndStringify("int x@123;"));
        ASSERT_EQUALS("bool x ;", tokenizeAndStringify("bool x@123:1;"));
        ASSERT_EQUALS("char PORTB ; bool PB3 ;", tokenizeAndStringify("char PORTB @ 0x10; bool PB3 @ PORTB:3;\n"));
        ASSERT_EQUALS("int x ;", tokenizeAndStringify("int x @ (0x1000 + 18);"));

        ASSERT_EQUALS("int x [ 10 ] ;", tokenizeAndStringify("int x[10]@0x100;"));

        ASSERT_EQUALS("void ( * f [ ] ) ( void ) ;", tokenizeAndStringify("void (*f[])(void)@0x100;")); // #13458

        ASSERT_EQUALS("interrupt@ f ( ) { }", tokenizeAndStringify("@interrupt f() {}"));

        ASSERT_EQUALS("const short MyVariable = 0xF0F0 ;", tokenizeAndStringify("const short MyVariable @ \"MYOWNSECTION\" = 0xF0F0; ")); // #12602
    }

    void inlineasm() {
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("asm { mov ax,bx };"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("_asm { mov ax,bx };"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("_asm mov ax,bx"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ;", tokenizeAndStringify("__asm { mov ax,bx };"));
        ASSERT_EQUALS("asm ( \"\"mov ax,bx\"\" ) ;", tokenizeAndStringify("__asm__ __volatile__ ( \"mov ax,bx\" );"));
        ASSERT_EQUALS("asm ( \"_emit 12h\" ) ;", tokenizeAndStringify("__asm _emit 12h ;"));
        ASSERT_EQUALS("asm ( \"mov a , b\" ) ;", tokenizeAndStringify("__asm mov a, b ;"));
        ASSERT_EQUALS("asm ( \"\"fnstcw %0\" : \"= m\" ( old_cw )\" ) ;", tokenizeAndStringify("asm volatile (\"fnstcw %0\" : \"= m\" (old_cw));"));
        ASSERT_EQUALS("asm ( \"\"fnstcw %0\" : \"= m\" ( old_cw )\" ) ;", tokenizeAndStringify(" __asm__ (\"fnstcw %0\" : \"= m\" (old_cw));"));
        ASSERT_EQUALS("asm ( \"\"ddd\"\" ) ;", tokenizeAndStringify(" __asm __volatile__ (\"ddd\") ;"));
        ASSERT_EQUALS("asm ( \"\"ddd\"\" ) ;", tokenizeAndStringify(" __asm __volatile (\"ddd\") ;"));
        ASSERT_EQUALS("asm ( \"\"mov ax,bx\"\" ) ;", tokenizeAndStringify("__asm__ volatile ( \"mov ax,bx\" );"));
        ASSERT_EQUALS("asm ( \"mov ax , bx\" ) ; int a ;", tokenizeAndStringify("asm { mov ax,bx } int a;"));
        ASSERT_EQUALS("asm\n\n( \"mov ax , bx\" ) ;", tokenizeAndStringify("__asm\nmov ax,bx\n__endasm;"));
        ASSERT_EQUALS("asm\n\n( \"push b ; for if\" ) ;", tokenizeAndStringify("__asm\npush b ; for if\n__endasm;"));
        ASSERT_EQUALS("asm ( \"\"mov ax , bx\"\" ) ;", tokenizeAndStringify("asm volatile (\"mov ax , bx\");"));
        ASSERT_EQUALS("asm ( \"\"mov ax , bx\"\" ) ;", tokenizeAndStringify("asm goto (\"mov ax , bx\");"));
        ASSERT_EQUALS("asm ( \"\"mov ax , bx\"\" ) ;", tokenizeAndStringify("asm inline (\"mov ax , bx\");"));

        // 'asm ( ) ;' should be in the same line
        ASSERT_EQUALS(";\n\nasm ( \"\"mov ax,bx\"\" ) ;", tokenizeAndStringify(";\n\n__asm__ volatile ( \"mov ax,bx\" );"));

        ASSERT_EQUALS("void func1 ( ) ;", tokenizeAndStringify("void func1() __asm__(\"...\") __attribute__();"));

        // #14250 - assembler function
        const char code[] = "__asm void dostuff(uint32_t x) { "
                            "%reg x "
                            "  e_lis r7, (lf)@h "
                            "%error "
                            "}";
        ASSERT_EQUALS("void dostuff ( uint32_t x ) { asm ( \"% reg x e_lis r7 , ( lf ) @ h % error\" ) ; }",
                      tokenizeAndStringify(code));
    }

    // #4725 - ^{}
    void simplifyAsm2() {
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { ^{} }"), SYNTAX, "syntax error");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { x(^{}); }"), SYNTAX, "syntax error");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { foo(A(), ^{ bar(); }); }"), SYNTAX, "syntax error");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("int f0(Args args) {\n"
                                                          "    return ^{\n"
                                                          "        return sizeof...(Args);\n"
                                                          "    }() + ^ {\n"
                                                          "        return sizeof...(args);\n"
                                                          "    }();\n"
                                                          "};"), SYNTAX, "syntax error");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("int(^block)(void) = ^{\n"
                                                          "    static int test = 0;\n"
                                                          "    return test;\n"
                                                          "};"), SYNTAX, "syntax error");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("; return f(a[b=c],^{});"), SYNTAX, "syntax error: keyword 'return' is not allowed in global scope"); // #7185
        ASSERT_EQUALS("{ return f ( asm ( \"^(void){somecode}\" ) ) ; }",
                      tokenizeAndStringify("{ return f(^(void){somecode}); }"));
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify(";a?(b?(c,^{}):0):^{};"), SYNTAX, "syntax error");
        ASSERT_EQUALS("template < typename T > "
                      "CImg < T > operator| ( const char * const expression , const CImg < T > & img ) { "
                      "return img | expression ; "
                      "} "
                      "template < typename T > "
                      "CImg < T > operator^ ( const char * const expression , const CImg < T > & img ) { "
                      "return img ^ expression ; "
                      "} "
                      "template < typename T > "
                      "CImg < T > operator== ( const char * const expression , const CImg < T > & img ) { "
                      "return img == expression ; "
                      "}",
                      tokenizeAndStringify("template < typename T >"
                                           "inline CImg<T> operator|(const char *const expression, const CImg<T>& img) {"
                                           "  return img | expression ;"
                                           "}"
                                           "template<typename T>"
                                           "inline CImg<T> operator^(const char *const expression, const CImg<T>& img) {"
                                           "  return img ^ expression;"
                                           "}"
                                           "template<typename T>"
                                           "inline CImg<T> operator==(const char *const expression, const CImg<T>& img) {"
                                           "  return img == expression;"
                                           "}"));
    }

    void ifAddBraces1() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a);\n"
                            "    else ;\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { ; }\n"
                      "else { ; }\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void ifAddBraces2() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) if (b) { }\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { if ( b ) { } }\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void ifAddBraces3() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a) for (;;) { }\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) { for ( ; ; ) { } }\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void ifAddBraces4() {
        const char code[] = "char * foo ()\n"
                            "{\n"
                            "    char *str = malloc(10);\n"
                            "    if (somecondition)\n"
                            "        for ( ; ; )\n"
                            "        { }\n"
                            "    return str;\n"
                            "}\n";
        ASSERT_EQUALS("char * foo ( )\n"
                      "{\n"
                      "char * str ; str = malloc ( 10 ) ;\n"
                      "if ( somecondition ) {\n"
                      "for ( ; ; )\n"
                      "{ } }\n"
                      "return str ;\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:4:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable somecondition [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void ifAddBraces5() {
        const char code[] = "void f()\n"
                            "{\n"
                            "for(int i = 0; i < 2; i++)\n"
                            "if(true)\n"
                            "return;\n"
                            "\n"
                            "return;\n"
                            "}\n";

        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "for ( int i = 0 ; i < 2 ; i ++ ) {\n"
                      "if ( true ) {\n"
                      "return ; } }\n\n"
                      "return ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void ifAddBraces7() {
        const char code[] = "void f()\n"
                            "{\n"
                            "int a;\n"
                            "if( a )\n"
                            "  ({a=4;}),({a=5;});\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "int a ;\n"
                      "if ( a ) {\n"
                      "( { a = 4 ; } ) , ( { a = 5 ; } ) ; }\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));
    }

    void ifAddBraces9() {
        // ticket #990
        const char code[] =
            "void f() {"
            "    for (int k=0; k<VectorSize; k++)"
            "        LOG_OUT(ID_Vector[k])"
            "}";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code), UNKNOWN_MACRO);
    }

    void ifAddBraces11() {
        const char code[] = "{ if (x) if (y) ; else ; }";
        const char expected[] = "{ if ( x ) { if ( y ) { ; } else { ; } } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void ifAddBraces12() {
        // ticket #1424
        const char code[] = "{ if (x) do { } while(x); }";
        const char expected[] = "{ if ( x ) { do { } while ( x ) ; } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void ifAddBraces13() {
        // ticket #1809
        const char code[] = "{ if (x) if (y) { } else { } else { } }";
        const char expected[] = "{ if ( x ) { if ( y ) { } else { } } else { } }";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));

        // ticket #1809
        const char code2[] = "{ if (x) while (y) { } else { } }";
        const char expected2[] = "{ if ( x ) { while ( y ) { } } else { } }";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));
    }

    void ifAddBraces15() {
        // ticket #2616 - unknown macro before if
        // TODO: Remove "A" or change it to ";A;". Then cleanup Tokenizer::ifAddBraces().
        ASSERT_EQUALS("{ A if ( x ) { y ( ) ; } }", tokenizeAndStringify("{A if(x)y();}"));
    }

    void ifAddBraces16() {
        // ticket #2873 - the fix is not needed anymore.
        {
            const char code[] = "void f() { "
                                "(void) ( { if(*p) (*p) = x(); } ) "
                                "}";
            ASSERT_EQUALS("void f ( ) { ( void ) ( { if ( * p ) { ( * p ) = x ( ) ; } } ) }",
                          tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:1:27]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable p [valueFlowBailoutIncompleteVar]\n",
                filter_valueflow(errout_str()));
        }
    }

    void ifAddBraces17() {
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a)\n"
                            "        bar1 ();\n"
                            "\n"
                            "    else\n"
                            "        bar2 ();\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) {\n"
                      "bar1 ( ) ; }\n"
                      "\n"
                      "else {\n"
                      "bar2 ( ) ; }\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void ifAddBraces18() {
        // ticket #3424 - if if { } else else
        ASSERT_EQUALS("{ if ( x ) { if ( y ) { } else { ; } } else { ; } }",
                      tokenizeAndStringify("{ if(x) if(y){}else;else;}"));

        ASSERT_EQUALS("{ if ( x ) { if ( y ) { if ( z ) { } else { ; } } else { ; } } else { ; } }",
                      tokenizeAndStringify("{ if(x) if(y) if(z){}else;else;else;}"));
    }

    void ifAddBraces19() {
        // #3928 - if for if else
        const char code[] = "void f()\n"
                            "{\n"
                            "    if (a)\n"
                            "        for (;;)\n"
                            "            if (b)\n"
                            "                bar1();\n"
                            "            else\n"
                            "                bar2();\n"
                            "}\n";
        ASSERT_EQUALS("void f ( )\n"
                      "{\n"
                      "if ( a ) {\n"
                      "for ( ; ; ) {\n"
                      "if ( b ) {\n"
                      "bar1 ( ) ; }\n"
                      "else {\n"
                      "bar2 ( ) ; } } }\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:3:9]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void ifAddBraces20() { // #5012 - syntax error 'else }'
        const char code[] = "void f() { if(x) {} else }";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code), SYNTAX);
    }

    void ifAddBracesLabels() {
        // Labels before statement
        ASSERT_EQUALS("int f ( int x ) {\n"
                      "if ( x ) {\n"
                      "l1 : ; l2 : ; return x ; }\n"
                      "}",
                      tokenizeAndStringify("int f(int x) {\n"
                                           "  if (x)\n"
                                           "  l1: l2: return x;\n"
                                           "}"));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));

        // Labels before {
        ASSERT_EQUALS("int f ( int x ) {\n"
                      "if ( x )\n"
                      "{ l1 : ; l2 : ; return x ; }\n"
                      "}",
                      tokenizeAndStringify("int f(int x) {\n"
                                           "  if (x)\n"
                                           "  l1: l2: { return x; }\n"
                                           "}"));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));

        // Labels before try/catch
        ASSERT_EQUALS("int f ( int x ) {\n"
                      "if ( x ) {\n"
                      "l1 : ; l2 : ;\n"
                      "try { throw 1 ; }\n"
                      "catch ( ... ) { return x ; } }\n"
                      "}",
                      tokenizeAndStringify("int f(int x) {\n"
                                           "  if (x)\n"
                                           "  l1: l2:\n"
                                           "    try { throw 1; }\n"
                                           "    catch(...) { return x; }\n"
                                           "}"));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));
    }

    void switchAddBracesLabels() {
        // Labels before statement
        ASSERT_EQUALS("int f ( int x ) {\n"
                      "switch ( x ) {\n"
                      "l1 : ; case 0 : ; l2 : ; case ( 1 ) : ; return x ; }\n"
                      "}",
                      tokenizeAndStringify("int f(int x) {\n"
                                           "  switch (x)\n"
                                           "  l1: case 0: l2: case (1): return x;\n"
                                           "}"));

        // Labels before {
        ASSERT_EQUALS("int f ( int x ) {\n"
                      "switch ( x )\n"
                      "{ l1 : ; case 0 : ; l2 : ; case ( 1 ) : ; return x ; }\n"
                      "}",
                      tokenizeAndStringify("int f(int x) {\n"
                                           "  switch (x)\n"
                                           "  l1: case 0: l2: case (1): { return x; }\n"
                                           "}"));

        // Labels before try/catch
        ASSERT_EQUALS("int f ( int x ) {\n"
                      "switch ( x ) {\n"
                      "l1 : ; case 0 : ; l2 : ; case ( 1 ) : ;\n"
                      "try { throw 1 ; }\n"
                      "catch ( ... ) { return x ; } }\n"
                      "}",
                      tokenizeAndStringify("int f(int x) {\n"
                                           "  switch (x)\n"
                                           "  l1: case 0: l2: case (1):\n"
                                           "    try { throw 1; }\n"
                                           "    catch(...) { return x; }\n"
                                           "}"));
    }

    void whileAddBraces() {
        const char code[] = "{while(a);}";
        ASSERT_EQUALS("{ while ( a ) { ; } }", tokenizeAndStringify(code));
    }

    void whileAddBracesLabels() {
        // Labels before statement
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "while ( x ) {\n"
                      "l1 : ; l2 : ; -- x ; }\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  while (x)\n"
                                           "  l1: l2: --x;\n"
                                           "}"));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));

        // Labels before {
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "while ( x )\n"
                      "{ l1 : ; l2 : ; -- x ; }\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  while (x)\n"
                                           "  l1: l2: { -- x; }\n"
                                           "}"));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));

        // Labels before try/catch
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "while ( x ) {\n"
                      "l1 : ; l2 : ;\n"
                      "try { throw 1 ; }\n"
                      "catch ( ... ) { -- x ; } }\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  while (x)\n"
                                           "  l1: l2:\n"
                                           "    try { throw 1; }\n"
                                           "    catch(...) { --x; }\n"
                                           "}"));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));
    }

    void doWhileAddBraces() {
        {
            const char code[] = "{do ; while (0);}";
            const char result[] = "{ do { ; } while ( 0 ) ; }";

            ASSERT_EQUALS(result, tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "{ UNKNOWN_MACRO ( do ) ; while ( a -- ) ; }";
            ASSERT_THROW_INTERNAL(tokenizeAndStringify(code), SYNTAX);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "{ UNKNOWN_MACRO ( do , foo ) ; while ( a -- ) ; }";
            ASSERT_THROW_INTERNAL(tokenizeAndStringify(code), SYNTAX);
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void foo ( int c , int d ) {\n"
                                " do\n"
                                "  if ( c ) {\n"
                                "   while ( c ) { c -- ; }\n"
                                "  }\n"
                                " while ( -- d > 0 ) ;\n"
                                " return 0 ;\n"
                                "}\n";
            const char result[] =   "void foo ( int c , int d ) {\n"
                                  "do {\n"
                                  "if ( c ) {\n"
                                  "while ( c ) { c -- ; }\n"
                                  "} }\n"
                                  "while ( -- d > 0 ) ;\n"
                                  "return 0 ;\n"
                                  "}";
            ASSERT_EQUALS(result, tokenizeAndStringify(code));
            ASSERT_EQUALS("", filter_valueflow(errout_str()));
        }

        {
            const char code[] = "void foo ( int c , int d ) {\n"
                                " do\n"
                                "   do c -- ; while ( c ) ;\n"
                                " while ( -- d > 0 ) ;\n"
                                " return 0 ;\n"
                                "}\n";
            const char result[] =   "void foo ( int c , int d ) {\n"
                                  "do {\n"
                                  "do { c -- ; } while ( c ) ; }\n"
                                  "while ( -- d > 0 ) ;\n"
                                  "return 0 ;\n"
                                  "}";
            ASSERT_EQUALS(result, tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            // #8148 - while inside the do-while body
            const char code[] = "void foo() {\n"
                                "    do { while (x) f(); } while (y);\n"
                                "}";
            const char result[] = "void foo ( ) {\n"
                                  "do { while ( x ) { f ( ) ; } } while ( y ) ;\n"
                                  "}";
            ASSERT_EQUALS(result, tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:2:17]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
                filter_valueflow(errout_str()));
        }
    }

    void doWhileAddBracesLabels() {
        // Labels before statement
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "do {\n"
                      "l1 : ; l2 : ; -- x ; }\n"
                      "while ( x ) ;\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  do\n"
                                           "  l1: l2: --x;\n"
                                           "  while (x);\n"
                                           "}"));

        // Labels before {
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "do\n"
                      "{ l1 : ; l2 : ; -- x ; }\n"
                      "while ( x ) ;\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  do\n"
                                           "  l1: l2: { -- x; }\n"
                                           "  while (x);\n"
                                           "}"));

        // Labels before try/catch
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "do {\n"
                      "l1 : ; l2 : ;\n"
                      "try { throw 1 ; }\n"
                      "catch ( ... ) { -- x ; } }\n"
                      "while ( x ) ;\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  do\n"
                                           "  l1: l2:\n"
                                           "    try { throw 1; }\n"
                                           "    catch(...) { --x; }\n"
                                           "  while (x);\n"
                                           "}"));
    }

    void forAddBraces1() {
        {
            const char code[] = "void f() {\n"
                                "     for(;;)\n"
                                "         if (a) { }\n"
                                "         else { }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "for ( ; ; ) {\n"
                                    "if ( a ) { }\n"
                                    "else { } }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:3:14]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
                filter_valueflow(errout_str()));
        }

        {
            const char code[] = "void f() {\n"
                                "     for(;;)\n"
                                "         if (a) { }\n"
                                "         else if (b) { }\n"
                                "         else { }\n"
                                "}";
            const char expected[] = "void f ( ) {\n"
                                    "for ( ; ; ) {\n"
                                    "if ( a ) { }\n"
                                    "else { if ( b ) { }\n"
                                    "else { } } }\n"
                                    "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:3:14]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable a [valueFlowBailoutIncompleteVar]\n",
                filter_valueflow(errout_str()));
        }
    }

    void forAddBraces2() { // #5088
        const char code[] = "void f() {\n"
                            "    for(;;) try { } catch (...) { }\n"
                            "}";
        const char expected[] = "void f ( ) {\n"
                                "for ( ; ; ) { try { } catch ( ... ) { } }\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void forAddBracesLabels() {
        // Labels before statement
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "for ( ; x ; ) {\n"
                      "l1 : ; l2 : ; -- x ; }\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  for ( ; x; )\n"
                                           "  l1: l2: --x;\n"
                                           "}"));

        // Labels before {
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "for ( ; x ; )\n"
                      "{ l1 : ; l2 : ; -- x ; }\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  for ( ; x; )\n"
                                           "  l1: l2: { -- x; }\n"
                                           "}"));

        // Labels before try/catch
        ASSERT_EQUALS("void f ( int x ) {\n"
                      "for ( ; x ; ) {\n"
                      "l1 : ; l2 : ;\n"
                      "try { throw 1 ; }\n"
                      "catch ( ... ) { -- x ; } }\n"
                      "}",
                      tokenizeAndStringify("void f(int x) {\n"
                                           "  for ( ; x; )\n"
                                           "  l1: l2:\n"
                                           "    try { throw 1; }\n"
                                           "    catch(...) { --x; }\n"
                                           "}"));
    }


    void simplifyExternC() {
        const char expected[] = "int foo ( ) ;";
        {
            const char code[] = "extern \"C\" int foo();";
            // tokenize..
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            // Expected result..
            ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
            ASSERT(tokenizer.tokens()->next()->isExternC());
        }
        {
            const char code[] = "extern \"C\" { int foo(); }";
            // tokenize..
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            // Expected result..
            ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
            ASSERT(tokenizer.tokens()->next()->isExternC());
        }
        {
            const char code[] = "extern \"C++\" int foo();";
            // tokenize..
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            // Expected result..
            ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
            ASSERT(!tokenizer.tokens()->next()->isExternC());
        }
        {
            const char code[] = "extern \"C++\" { int foo(); }";
            // tokenize..
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            // Expected result..
            ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));
            ASSERT(!tokenizer.tokens()->next()->isExternC());
        }
    }

    void simplifyFunctionParameters() {
        {
            const char code[] = "char a [ ABC ( DEF ) ] ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "module ( a , a , sizeof ( a ) , 0444 ) ;";
            ASSERT_EQUALS("module ( a , a , sizeof ( a ) , 0444 ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        ASSERT_EQUALS("void f ( int x ) { }", tokenizeAndStringify("void f(x) int x; { }"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("void f ( int x , char y ) { }", tokenizeAndStringify("void f(x,y) int x; char y; { }"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("int main ( int argc , char * argv [ ] ) { }", tokenizeAndStringify("int main(argc,argv) int argc; char *argv[]; { }"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("int f ( int p , int w , float d ) { }", tokenizeAndStringify("int f(p,w,d) float d; { }"));
        ASSERT_EQUALS("", errout_str());

        // #1067 - Not simplified. Feel free to fix so it is simplified correctly but this syntax is obsolescent.
        ASSERT_EQUALS("int ( * d ( a , b , c ) ) ( ) int a ; int b ; int c ; { }", tokenizeAndStringify("int (*d(a,b,c))()int a,b,c; { }"));
        ASSERT_EQUALS("", errout_str());

        {
            // This is not a function but the pattern is similar..
            const char code[] = "void foo()"
                                "{"
                                "    if (x)"
                                "        int x;"
                                "    { }"
                                "}";
            ASSERT_EQUALS("void foo ( ) { if ( x ) { int x ; } { } }", tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:1:20]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
                filter_valueflow(errout_str()));
        }
    }

    void simplifyFunctionParameters1() { // ticket #3721

        const char code[] = "typedef float ufloat;\n"
                            "typedef short ftnlen;\n"
                            "int f(p,w,d,e,len) ufloat *p; ftnlen len;\n"
                            "{\n"
                            "}\n";
        ASSERT_EQUALS("int f ( float * p , int w , int d , int e , short len )\n"
                      "{\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyFunctionParameters2() { // #4430
        const char code[] = "class Item { "
                            "int i ; "
                            "public: "
                            "Item ( int i ) ; "
                            "} ; "
                            "Item :: Item ( int i ) : i ( i ) { }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyFunctionParameters3() { // #4436
        const char code[] = "class Item { "
                            "int i ; "
                            "int j ; "
                            "public: "
                            "Item ( int i , int j ) ; "
                            "} ; "
                            "Item :: Item ( int i , int j ) : i ( i ) , j ( j ) { }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyFunctionParameters4() { // #9421
        const char code[] = "int foo :: bar ( int , int ) const ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyFunctionParametersMultiTemplate() {
        const char code[] = "template < typename T1 > template < typename T2 > "
                            "void A < T1 > :: foo ( T2 ) { }";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
        ASSERT_EQUALS("[test.cpp:1:68]: (debug) Executable scope 'foo' with unknown function. [symbolDatabaseWarning]\n", errout_str());
    }

    void simplifyFunctionParametersErrors() {
        //same parameters...
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void foo(x, x)\n"
                                                   " int x;\n"
                                                   " int x;\n"
                                                   "{}\n"), SYNTAX);

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void foo(x, y)\n"
                                                   " int x;\n"
                                                   " int x;\n"
                                                   "{}\n"), SYNTAX);

        ASSERT_NO_THROW(tokenizeAndStringify("void foo(int, int)\n"
                                             "{}"));
        ASSERT_EQUALS("", errout_str());

        // #3848 - Don't hang
        (void)tokenizeAndStringify("sal_Bool ShapeHasText(sal_uLong, sal_uLong) const {\n"
                                   "    return sal_True;\n"
                                   "}\n"
                                   "void CreateSdrOLEFromStorage() {\n"
                                   "    comphelper::EmbeddedObjectContainer aCnt( xDestStorage );\n"
                                   "    { }\n"
                                   "}");
        ignore_errout();
    }

    void simplifyFunctionTryCatch() {
        ASSERT_EQUALS("void foo ( ) { try {\n"
                      "} catch ( int ) {\n"
                      "} catch ( char ) {\n"
                      "} }",
                      tokenizeAndStringify("void foo() try {\n"
                                           "} catch (int) {\n"
                                           "} catch (char) {\n"
                                           "}"));

        ASSERT_EQUALS("void foo ( ) { try {\n"
                      "struct S {\n"
                      "void bar ( ) { try {\n"
                      "} catch ( int ) {\n"
                      "} catch ( char ) {\n"
                      "} }\n"
                      "} ;\n"
                      "} catch ( long ) {\n"
                      "} }",
                      tokenizeAndStringify("void foo() try {\n"
                                           "  struct S {\n"
                                           "    void bar() try {\n"
                                           "    } catch (int) {\n"
                                           "    } catch (char) {\n"
                                           "    }\n"
                                           "  };\n"
                                           "} catch (long) {\n"
                                           "}"));

        ASSERT_EQUALS("struct S { void func ( ) const ; } ;\n"
                      "void S :: func ( ) const {\n"
                      "try { f ( ) ; }\n"
                      "catch ( ... ) { g ( ) ; } }",
                      tokenizeAndStringify("struct S { void func() const; };\n"
                                           "void S::func() const\n"
                                           "try { f(); }\n"
                                           "catch (...) { g(); }\n"));
    }

    // Simplify "((..))" into "(..)"
    void removeParentheses1() {
        const char code[] = "void foo()"
                            "{"
                            "    free(((void*)p));"
                            "}";

        ASSERT_EQUALS("void foo ( ) { free ( ( void * ) p ) ; }", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:1:29]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable p [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void removeParentheses3() {
        {
            const char code[] = "void foo()"
                                "{"
                                "    if (( true )==(true)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { if ( true == true ) { } }", tokenizeAndStringify(code));
        }

        {
            const char code[] = "void foo()"
                                "{"
                                "    if (( 2 )==(2)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { if ( 2 == 2 ) { } }", tokenizeAndStringify(code));
        }

        {
            const char code[] = "void foo()"
                                "{"
                                "    if( g(10)){}"
                                "}";
            ASSERT_EQUALS("void foo ( ) { if ( g ( 10 ) ) { } }", tokenizeAndStringify(code));
        }
    }

    // Simplify "( function (..))" into "function (..)"
    void removeParentheses4() {
        const char code[] = "void foo()"
                            "{"
                            "    (free(p));"
                            "}";
        ASSERT_EQUALS("void foo ( ) { free ( p ) ; }", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:1:22]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable p [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void removeParentheses5() {
        // Simplify "( delete x )" into "delete x"
        {
            const char code[] = "void foo()"
                                "{"
                                "    (delete p);"
                                "}";
            ASSERT_EQUALS("void foo ( ) { delete p ; }", tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:1:24]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable p [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }

        // Simplify "( delete [] x )" into "delete [] x"
        {
            const char code[] = "void foo()"
                                "{"
                                "    (delete [] p);"
                                "}";
            ASSERT_EQUALS("void foo ( ) { delete [ ] p ; }", tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:1:27]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable p [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }
    }

    // "!(abc.a)" => "!abc.a"
    void removeParentheses6() {
        {
            const char code[] = "(!(abc.a));";
            ASSERT_EQUALS("( ! abc . a ) ;", tokenizeAndStringify(code));
        }
        //handle more complex member selections
        {
            const char code[] = "(!(a.b.c.d));";
            ASSERT_EQUALS("( ! a . b . c . d ) ;", tokenizeAndStringify(code));
        }
    }

    void removeParentheses7() {
        const char code[] = ";char *p; (delete(p), (p)=0);";
        ASSERT_EQUALS("; char * p ; delete ( p ) , p = 0 ;", tokenizeAndStringify(code));
    }

    void removeParentheses8() {
        const char code[] = "struct foo {\n"
                            "    void operator delete(void *obj, size_t sz);\n"
                            "}\n";
        const std::string actual(tokenizeAndStringify(code, settings2_win32a));

        const char expected[] = "struct foo {\n"
                                "void operatordelete ( void * obj , unsigned long sz ) ;\n"
                                "}";

        ASSERT_EQUALS(expected, actual);
    }

    void removeParentheses9() {
        ASSERT_EQUALS("void delete ( double num ) ;", tokenizeAndStringify("void delete(double num);"));
    }

    void removeParentheses10() {
        ASSERT_EQUALS("p = buf + 8 ;", tokenizeAndStringify("p = (buf + 8);"));
    }

    void removeParentheses11() {
        // #2502
        ASSERT_EQUALS("{ } x ( ) ;", tokenizeAndStringify("{}(x());"));
    }

    void removeParentheses12() {
        // #2760
        ASSERT_EQUALS(", x = 0 ;", tokenizeAndStringify(",(x)=0;"));
    }

    void removeParentheses13() {
        ASSERT_EQUALS("; f ( a + b , c ) ;", tokenizeAndStringify(";f((a+b),c);"));
        ASSERT_EQUALS("; x = y [ a + b ] ;", tokenizeAndStringify(";x=y[(a+b)];"));
    }

    void removeParentheses14() {
        ASSERT_EQUALS("{ if ( ( i & 1 ) == 0 ) { ; } }", tokenizeAndStringify("{ if ( (i & 1) == 0 ); }"));
    }

    void removeParentheses15() {
        ASSERT_EQUALS("a = b ? c : 123 ;", tokenizeAndStringify("a = b ? c : (123);"));
        ASSERT_EQUALS("a = b ? c : ( 123 + 456 ) ;", tokenizeAndStringify("a = b ? c : ((123)+(456));"));
        ASSERT_EQUALS("a = b ? ( 123 ) : c ;", tokenizeAndStringify("a = b ? (123) : c;"));

        // #4316
        ASSERT_EQUALS("a = b ? c : ( d = 1 , 0 ) ;", tokenizeAndStringify("a = b ? c : (d=1,0);"));
    }

    void removeParentheses16() { // *(x.y)=
        // #4423
        ASSERT_EQUALS("; * x = 0 ;", tokenizeAndStringify(";*(x)=0;"));
        ASSERT_EQUALS("; * x . y = 0 ;", tokenizeAndStringify(";*(x.y)=0;"));
    }

    void removeParentheses17() { // a ? b : (c > 0 ? d : e)
        ASSERT_EQUALS("a ? b : ( c > 0 ? d : e ) ;", tokenizeAndStringify("a?b:(c>0?d:e);"));
    }

    void removeParentheses18() {
        ASSERT_EQUALS("float ( * a ) [ 2 ] ;", tokenizeAndStringify("float(*a)[2];"));
    }

    void removeParentheses19() {
        ASSERT_EQUALS("( ( ( typeof ( X ) ) * ) 0 ) ;", tokenizeAndStringify("(((typeof(X))*)0);"));
    }

    void removeParentheses20() {
        ASSERT_EQUALS("a < b < int > > ( 2 ) ;", tokenizeAndStringify("a<b<int>>(2);"));
    }

    void removeParentheses21() {
        ASSERT_EQUALS("a = ( int ) - b ;", tokenizeAndStringify("a = ((int)-b);"));
    }

    void removeParentheses22() {
        static char code[] = "struct S { "
                             "char *(a); "
                             "char &(b); "
                             "const static char *(c); "
                             "} ;";
        static const char exp[] = "struct S { "
                                  "char * a ; "
                                  "char & b ; "
                                  "static const char * c ; "
                                  "} ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParentheses23() { // Ticket #6103
        // Reported case
        {
            static char code[] = "; * * p f ( ) int = { new int ( * [ 2 ] ) ; void }";
            static const char exp[] = "; * * p f ( ) int = { new int ( * [ 2 ] ) ; void }";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        // Various valid cases
        {
            static char code[] = "int * f [ 1 ] = { new ( int ) } ;";
            static const char exp[] = "int * f [ 1 ] = { new int } ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        {
            static char code[] = "int * * f [ 1 ] = { new ( int ) [ 1 ] } ;";
            static const char exp[] = "int * * f [ 1 ] = { new int [ 1 ] } ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        {
            static char code[] = "list < int > * f [ 1 ] = { new ( list < int > ) } ;";
            static const char exp[] = "list < int > * f [ 1 ] = { new list < int > } ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
        // don't remove parentheses in operator new overload
        {
            static char code[] = "void *operator new(__SIZE_TYPE__, int);";
            static const char exp[] = "void * operatornew ( __SIZE_TYPE__ , int ) ;";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        }
    }

    void removeParentheses24() { // Ticket #7040
        static char code[] = "std::hash<decltype(t._data)>()(t._data);";
        static const char exp[] = "std :: hash < decltype ( t . _data ) > ( ) ( t . _data ) ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParentheses25() { // daca@home - a=(b,c)
        static char code[] = "a=(b,c);";
        static const char exp[] = "a = ( b , c ) ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParentheses26() { // Ticket #8875 a[0](0)
        static char code[] = "a[0](0);";
        static const char exp[] = "a [ 0 ] ( 0 ) ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParentheses27() {
        static char code[] = "struct S { int i; };\n"
                             "void g(int, int);\n"
                             "void f(S s, int j) {\n"
                             "    g(j, (decltype(s.i))j * s.i);\n"
                             "}\n";
        static const char exp[] = "struct S { int i ; } ;\n"
                                  "void g ( int , int ) ;\n"
                                  "void f ( S s , int j ) {\n"
                                  "g ( j , ( decltype ( s . i ) ) j * s . i ) ;\n"
                                  "}";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParentheses28() { // Ticket #12164
        static char code[] = "temp1 = (value > 100U) ? (value+100U) : (value-50U);";
        static const char exp[] = "temp1 = ( value > 100U ) ? ( value + 100U ) : ( value - 50U ) ;";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParantheses29() { // Ticket #13735
        static char code[] = "double foo(void)\n"
                             "{\n"
                             "return (modf)(12.3, NULL);\n"
                             "}";
        static const char exp[] = "double foo ( )\n"
                                  "{\n"
                                  "return modf ( 12.3 , NULL ) ;\n"
                                  "}";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
    }

    void removeParentheses30() {
        static char code[] = "void f (Node *node) {\n"
                             "    if (node->data && (node->provider)->free)\n"
                             "        (node->provider)->free (node);\n"
                             "}\n";
        static const char exp[] = "void f ( Node * node ) {\n"
                                  "if ( node . data && ( node . provider ) . free ) {\n"
                                  "node . provider . free ( node ) ; }\n"
                                  "}";
        ASSERT_EQUALS(exp, tokenizeAndStringify(code));
        (void) errout_str();
    }

    void tokenize_double() {
        const char code[] = "void f() {\n"
                            "    double a = 4.2;\n"
                            "    float b = 4.2f;\n"
                            "    double c = 4.2e+10;\n"
                            "    double d = 4.2e-10;\n"
                            "    int e = 4+2;\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "double a ; a = 4.2 ;\n"
                      "float b ; b = 4.2f ;\n"
                      "double c ; c = 4.2e+10 ;\n"
                      "double d ; d = 4.2e-10 ;\n"
                      "int e ; e = 4 + 2 ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void tokenize_strings() {
        const char code[] =   "void f() {\n"
                            "const char *a =\n"
                            "{\n"
                            "\"hello \"\n"
                            "\"more \"\n"
                            "\"world\"\n"
                            "};\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "const char * a ; a =\n"
                      "{\n"
                      "\"hello more world\"\n"
                      "\n"
                      "\n"
                      "} ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyStructDecl() {
        const char code[] = "const struct A { int a; int b; } a;";
        ASSERT_EQUALS("struct A { int a ; int b ; } ; const struct A a ;", tokenizeAndStringify(code));

        // #9519
        const char code2[] = "enum A {} (a);";
        const char expected2[] = "enum A { } ; enum A a ;";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));

        // #11052
        const char code3[] = "struct a { int b; } static e[1];";
        const char expected3[] = "struct a { int b ; } ; struct a static e [ 1 ] ;";
        ASSERT_EQUALS(expected3, tokenizeAndStringify(code3));

        // #11013 - Do not remove unnamed struct in union
        const char code4[] = "union U { struct { int a; int b; }; int ab[2]; };";
        const char expected4[] = "union U { struct { int a ; int b ; } ; int ab [ 2 ] ; } ;";
        ASSERT_EQUALS(expected4, tokenizeAndStringify(code4));
    }

    void vardecl1() {
        const char code[] = "unsigned int a, b;";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("unsigned int a ; unsigned int b ;", actual);
    }

    void vardecl2() {
        const char code[] = "void foo(a,b) unsigned int a, b; { }";

        const std::string actual(tokenizeAndStringify(code));

        ASSERT_EQUALS("void foo ( unsigned int a , unsigned int b ) { }", actual);
    }

    void vardecl3() {
        const char code[] = "void f() { char * p = foo<10,char>(); }";
        const std::string actual(tokenizeAndStringify(code));
        ASSERT_EQUALS("void f ( ) { char * p ; p = foo < 10 , char > ( ) ; }", actual);
    }

    void vardecl4() {
        // ticket #346

        const char code1[] = "void *p = NULL;";
        const char res1[]  = "void * p ; p = NULL ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));

        const char code2[] = "const void *p = NULL;";
        const char res2[]  = "const void * p ; p = NULL ;";
        ASSERT_EQUALS(res2, tokenizeAndStringify(code2));

        const char code3[] = "void * const p = NULL;";
        const char res3[]  = "void * const p ; p = NULL ;";
        ASSERT_EQUALS(res3, tokenizeAndStringify(code3));

        const char code4[] = "const void * const p = NULL;";
        const char res4[]  = "const void * const p ; p = NULL ;";
        ASSERT_EQUALS(res4, tokenizeAndStringify(code4));

        const char code5[] = "const void * volatile p = NULL;";
        const char res5[]  = "const void * volatile p ; p = NULL ;";
        ASSERT_EQUALS(res5, tokenizeAndStringify(code5));
    }

    void vardecl5() {
        ASSERT_EQUALS("void foo ( int nX ) {\n"
                      "int addI ; addI = frontPoint == 2 || frontPoint == 1 ? ( i = 0 , 1 ) : ( i = nX - 2 , -1 ) ;\n"
                      "}", tokenizeAndStringify("void foo(int nX) {\n"
                                                "    int addI = frontPoint == 2 || frontPoint == 1 ? i = 0, 1 : (i = nX - 2, -1);\n"
                                                "}"));
        ASSERT_EQUALS(
            "[test.cpp:2:16]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable frontPoint [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void vardecl_stl_1() {
        // ticket #520

        const char code1[] = "std::vector<std::string>a, b;";
        const char res1[]  = "std :: vector < std :: string > a ; std :: vector < std :: string > b ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));

        const char code2[] = "std::vector<std::string>::const_iterator it, cit;";
        const char res2[]  = "std :: vector < std :: string > :: const_iterator it ; std :: vector < std :: string > :: const_iterator cit ;";
        ASSERT_EQUALS(res2, tokenizeAndStringify(code2));

        const char code3[] = "std::vector<std::pair<std::string, std::string > > *c, d;";
        const char res3[]  = "std :: vector < std :: pair < std :: string , std :: string > > * c ; std :: vector < std :: pair < std :: string , std :: string > > d ;";
        ASSERT_EQUALS(res3, tokenizeAndStringify(code3));
    }

    void vardecl_stl_2() {
        const char code1[] = "{ std::string x = \"abc\"; }";
        ASSERT_EQUALS("{ std :: string x ; x = \"abc\" ; }", tokenizeAndStringify(code1));

        const char code2[] = "{ std::vector<int> x = y; }";
        ASSERT_EQUALS("{ std :: vector < int > x ; x = y ; }", tokenizeAndStringify(code2));
    }

    void vardecl_stl_3()
    {
        const char code1[] = "{ std::string const x = \"abc\"; }";
        ASSERT_EQUALS("{ const std :: string x = \"abc\" ; }", tokenizeAndStringify(code1));

        const char code2[] = "{ std::vector<int> const x = y; }";
        ASSERT_EQUALS("{ const std :: vector < int > x = y ; }", tokenizeAndStringify(code2));
    }

    void vardecl_template_1() {
        // ticket #1046
        const char code1[] = "b<(1<<24),10,24> u, v;";
        const char res1[]  = "b < 16777216 , 10 , 24 > u ; b < 16777216 , 10 , 24 > v ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));
        // ticket #3571 (segmentation fault)
        (void)tokenizeAndStringify("template <int i = (3>4) > class X4 {};");
    }

    void vardecl_template_2() {
        // ticket #3650
        const char code[] = "const string str = x<8,int>();";
        const char expected[]  = "const string str = x < 8 , int > ( ) ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void vardecl_union() {
        // ticket #1976
        const char code1[] = "class Fred { public: union { int a ; int b ; } ; } ;";
        ASSERT_EQUALS(code1, tokenizeAndStringify(code1));

        // ticket #2039
        const char code2[] = "void f() {\n"
                             "     union {\n"
                             "         int x;\n"
                             "         long y;\n"
                             "     };\n"
                             "}";
        ASSERT_EQUALS("void f ( ) {\nunion {\nint x ;\nlong y ;\n} ;\n}", tokenizeAndStringify(code2));

        // ticket #3927
        const char code3[] = "union xy *p = NULL;";
        ASSERT_EQUALS("union xy * p ; p = NULL ;", tokenizeAndStringify(code3));
    }

    void vardecl_par() {
        // ticket #2743 - set links if variable type contains parentheses
        const char code[] = "Fred<int(*)()> fred1=a, fred2=b;";
        ASSERT_EQUALS("Fred < int ( * ) ( ) > fred1 ; fred1 = a ; Fred < int ( * ) ( ) > fred2 ; fred2 = b ;", tokenizeAndStringify(code));
    }

    void vardecl_par2() {
        // ticket #3912 - set correct links
        const char code[] = "function<void (shared_ptr<MyClass>)> v;";
        ASSERT_EQUALS("function < void ( shared_ptr < MyClass > ) > v ;", tokenizeAndStringify(code));
    }

    void vardecl_par3() {
        // ticket #6556- Fred x1(a), x2(b);
        const char code[] = "Fred x1(a), x2(b);";
        ASSERT_EQUALS("Fred x1 ( a ) ; Fred x2 ( b ) ;", tokenizeAndStringify(code));
    }

    void vardecl_class_ref() {
        const char code[] = "class A { B &b1,&b2; };";
        ASSERT_EQUALS("class A { B & b1 ; B & b2 ; } ;", tokenizeAndStringify(code));
    }

    void vardec_static() {
        {
            // don't simplify declarations of static variables
            // "static int i = 0;" is not the same as "static int i; i = 0;"
            const char code[] = "static int i = 0 ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
        }

        {
            const char code[] = "static int a, b;";
            ASSERT_EQUALS("static int a ; static int b ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static unsigned int a, b;";
            ASSERT_EQUALS("static unsigned int a ; static unsigned int b ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static int a=1, b=1;";
            ASSERT_EQUALS("static int a = 1 ; static int b = 1 ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static int *a, *b;";
            ASSERT_EQUALS("static int * a ; static int * b ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "static unsigned int *a=0, *b=0;";
            ASSERT_EQUALS("static unsigned int * a = 0 ; static unsigned int * b = 0 ;", tokenizeAndStringify(code));
        }

        {
            // Ticket #4450
            const char code[] = "static int large_eeprom_type = (13 | (5)), "
                                "default_flash_type = 42;";
            ASSERT_EQUALS("static int large_eeprom_type = 13 | 5 ; static int default_flash_type = 42 ;",
                          tokenizeAndStringify(code));
        }

        {
            // Ticket #5121
            const char code[] = "unsigned int x;"
                                "static const unsigned int A = 1, B = A, C = 0, D = (A), E = 0;"
                                "void f() {"
                                "  unsigned int *foo = &x;"
                                "}";
            ASSERT_EQUALS("unsigned int x ; "
                          "static const unsigned int A = 1 ; "
                          "static const unsigned int B = A ; "
                          "static const unsigned int C = 0 ; "
                          "static const unsigned int D = A ; "
                          "static const unsigned int E = 0 ; "
                          "void f ( ) { "
                          "unsigned int * foo ; "
                          "foo = & x ; "
                          "}",
                          tokenizeAndStringify(code));
        }

        {
            // Ticket #5266
            const char code[] = "class Machine {\n"
                                "  static int const STACK_ORDER = 10, STACK_MAX = 1 << STACK_ORDER,"
                                "                   STACK_GUARD = 2;\n"
                                "};";
            ASSERT_EQUALS("class Machine {\n"
                          "static const int STACK_ORDER = 10 ; static const int STACK_MAX = 1 << STACK_ORDER ; "
                          "static const int STACK_GUARD = 2 ;\n"
                          "} ;",
                          tokenizeAndStringify(code));
        }

        {
            // Ticket #9515
            const char code[] = "void(a)(void) {\n"
                                "    static int b;\n"
                                "    if (b) {}\n"
                                "}\n";
            ASSERT_EQUALS("void ( a ) ( void ) {\n"
                          "static int b ;\n"
                          "if ( b ) { }\n"
                          "}",
                          tokenizeAndStringify(code));
        }
    }

    void vardecl6() {
        // ticket #565

        const char code1[] = "int z = x >> 16;";
        const char res1[]  = "int z ; z = x >> 16 ;";
        ASSERT_EQUALS(res1, tokenizeAndStringify(code1));
    }

    void vardecl7() {
        // ticket #603
        const char code[] = "void f() {\n"
                            "    for (int c = 0; c < 0; ++c) {}\n"
                            "    int t;\n"
                            "    D(3 > t, \"T\");\n"
                            "}";
        const char res[] = "void f ( ) {\n"
                           "for ( int c = 0 ; c < 0 ; ++ c ) { }\n"
                           "int t ;\n"
                           "D ( 3 > t , \"T\" ) ;\n"
                           "}";

        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl8() {
        // ticket #696
        const char code[] = "char a[10]={'\\0'}, b[10]={'\\0'};";
        const char res[]  = "char a [ 10 ] = { '\\0' } ; char b [ 10 ] = { '\\0' } ;";
        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl9() {
        const char code[] = "char a[2] = {'A', '\\0'}, b[2] = {'B', '\\0'};";
        const char res[]  = "char a [ 2 ] = { 'A' , '\\0' } ; char b [ 2 ] = { 'B' , '\\0' } ;";
        ASSERT_EQUALS(res, tokenizeAndStringify(code));
    }

    void vardecl10() {
        // ticket #732
        const char code[] = "char a [ 2 ] = { '-' } ; memset ( a , '-' , sizeof ( a ) ) ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void vardecl11() {
        // ticket #1684
        const char code[] = "char a[5][8], b[5][8];";
        ASSERT_EQUALS("char a [ 5 ] [ 8 ] ; char b [ 5 ] [ 8 ] ;", tokenizeAndStringify(code));
    }

    void vardecl12() {
        const char code[] = "struct A { public: B a, b, c, d; };";
        ASSERT_EQUALS("struct A { public: B a ; B b ; B c ; B d ; } ;", tokenizeAndStringify(code));
    }

    void vardecl13() {
        const char code[] = "void f() {\n"
                            "    int a = (x < y) ? 1 : 0;\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\nint a ; a = ( x < y ) ? 1 : 0 ;\n}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:2:14]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void vardecl14() {
        const Settings s = settingsBuilder(settings1).cpp(Standards::CPP03).build();
        const char code[] = "::std::tr1::shared_ptr<int> pNum1, pNum2;\n";
        ASSERT_EQUALS(":: std :: tr1 :: shared_ptr < int > pNum1 ; :: std :: tr1 :: shared_ptr < int > pNum2 ;", tokenizeAndStringify(code, s, true, false));
    }

    void vardecl15() {
        const char code[] = "const char x[] = \"foo\", y[] = \"bar\";\n";
        ASSERT_EQUALS("const char x [ 4 ] = \"foo\" ; const char y [ 4 ] = \"bar\" ;", tokenizeAndStringify(code));
    }

    void vardecl16() {
        {
            const char code[] = "const a::b<c,d(e),f>::g::h<i>::l *x [] = foo(),y [][] = bar();\n";
            ASSERT_EQUALS("const a :: b < c , d ( e ) , f > :: g :: h < i > :: l * x [ ] = foo ( ) ; "
                          "const a :: b < c , d ( e ) , f > :: g :: h < i > :: l y [ ] [ ] = bar ( ) ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "const ::b<c,d(e),f>::g::h<i>::l *x [] = foo(),y [][] = bar();\n";
            ASSERT_EQUALS("const :: b < c , d ( e ) , f > :: g :: h < i > :: l * x [ ] = foo ( ) ; "
                          "const :: b < c , d ( e ) , f > :: g :: h < i > :: l y [ ] [ ] = bar ( ) ;", tokenizeAndStringify(code));
        }
    }

    void vardecl17() {
        const char code[] = "a < b > :: c :: d :: e < f > x = foo(), y = bar();\n";
        ASSERT_EQUALS("a < b > :: c :: d :: e < f > x ; x = foo ( ) ; "
                      "a < b > :: c :: d :: e < f > y ; y = bar ( ) ;", tokenizeAndStringify(code));
    }

    void vardecl18() {
        const char code[] = "void f() {\n"
                            "    g((double)v1*v2, v3, v4);\n"
                            "}\n";

        ASSERT_EQUALS("void f ( ) {\n"
                      "g ( ( double ) v1 * v2 , v3 , v4 ) ;\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:2:15]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable v1 [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void vardecl19() {
        {
            const char code[] = "void func(in, r, m)\n"
                                "int in;"
                                "int r,m;"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void func (\n"
                          "int in , int r , int m\n"
                          ")\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(r,f)\n"
                                "char *r;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r\n"
                          ")\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(f)\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f ( )\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(f,r)\n"
                                "char *r;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r\n"
                          ")\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(r,f,s)\n"
                                "char *r;\n"
                                "char *s;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r ,\n"
                          "char * s\n"
                          ")\n"
                          "\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(r,s,t)\n"
                                "char *r,*s,*t;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f (\n"
                          "char * r , char * s , char * t\n"
                          ")\n"
                          "\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
        {
            const char code[] = "void f(a, b) register char *a, *b;\n"
                                "{\n"
                                "}\n";

            ASSERT_EQUALS("void f ( char * a , char * b )\n"
                          "{\n"
                          "}", tokenizeAndStringify(code));
        }
    }

    void vardecl20() {
        // #3700
        const char code[] = "void a::b() const\n"
                            "{\n"
                            "    register const int X = 0;\n"
                            "}\n";
        ASSERT_EQUALS("void a :: b ( ) const\n"
                      "{\n"
                      "const int X = 0 ;\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS("[test.cpp:1:9]: (debug) Executable scope 'b' with unknown function. [symbolDatabaseWarning]\n", errout_str());
    }

    void vardecl21() { // type in namespace
        // #4042 - a::b const *p = 0;
        const char code1[] = "void f() {\n"
                             "    a::b const *p = 0;\n"
                             "}\n";
        ASSERT_EQUALS("void f ( ) {\n"
                      "const a :: b * p ; p = 0 ;\n"
                      "}"
                      , tokenizeAndStringify(code1));

        // #4226 - ::a::b const *p = 0;
        const char code2[] = "void f() {\n"
                             "    ::a::b const *p = 0;\n"
                             "}\n";
        ASSERT_EQUALS("void f ( ) {\n"
                      "const :: a :: b * p ; p = 0 ;\n"
                      "}"
                      , tokenizeAndStringify(code2));
    }

    void vardecl22() {  // #4211 - segmentation fault
        (void)tokenizeAndStringify("A<B<C<int>> >* p = 0;");
    }

    void vardecl23() {  // #4276 - segmentation fault
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("class a { protected : template < class int x = 1 ; public : int f ( ) ; }"), SYNTAX);
    }

    void vardecl24() {  // #4187 - variable declaration within lambda function
        const char code1[] = "void f() {\n"
                             "    std::for_each(ints.begin(), ints.end(), [](int val)\n"
                             "    {\n"
                             "        int temp = 0;\n"
                             "    });\n"
                             "}";

        const char expected1[] = "void f ( ) {\n"
                                 "std :: for_each ( ints . begin ( ) , ints . end ( ) , [ ] ( int val )\n"
                                 "{\n"
                                 "int temp ; temp = 0 ;\n"
                                 "} ) ;\n"
                                 "}";

        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1));

        const char code2[] = "void f(int j) {\n"
                             "    g( [](){int temp = 0;} , j );\n"
                             "}";

        const char expected2[] = "void f ( int j ) {\n"
                                 "g ( [ ] ( ) { int temp ; temp = 0 ; } , j ) ;\n"
                                 "}";

        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));
    }

    void vardecl25() {  // #4799 - segmentation fault
        (void)tokenizeAndStringify("void A::func(P g) const {}\n"
                                   "void A::a() {\n"
                                   "   b = new d(  [this]( const P & p) -> double { return this->func(p);}  );\n"
                                   "}");
        ignore_errout();
    }

    void vardecl26() { // #5907
        const char code[] = "extern int *new, obj, player;";
        const char expected[] = "extern int * new ; extern int obj ; extern int player ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS("[test.cpp:1:13]: (debug) Scope::checkVariable found variable 'new' with varid 0. [varid0]\n", errout_str());
    }

    void vardecl27() { // #7850 (segmentation fault)
        const char code[] = "extern int foo(char);\n"
                            "void* class(char c) {\n"
                            "  if (foo(c))\n"
                            "    return 0;\n"
                            "  return 0;\n"
                            "}";
        (void)tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false));
    }

    void vardecl28() {
        const char code[] = "unsigned short f(void) {\n"
                            "    unsigned short const int x = 1;\n"
                            "    return x;\n"
                            "}";
        ASSERT_EQUALS("unsigned short f ( ) {\n"
                      "const unsigned short x ; x = 1 ;\n"
                      "return x ;\n"
                      "}",
                      tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void vardecl29() { // #9282
        {
            const char code[] = "double f1() noexcept, f2(double) noexcept;";
            ASSERT_EQUALS("double f1 ( ) noexcept ( true ) ; double f2 ( double ) noexcept ( true ) ;",
                          tokenizeAndStringify(code));
        }

        {
            const char code[] = "class C {\n"
                                "    double f1() const noexcept, f2 (double) const noexcept;\n"
                                "};\n";
            ASSERT_EQUALS("class C {\n"
                          "double f1 ( ) const noexcept ( true ) ; double f2 ( double ) const noexcept ( true ) ;\n"
                          "} ;",
                          tokenizeAndStringify(code));
        }
    }

    void vardecl30() {
        const char code[] = "struct D {} const d;";
        ASSERT_EQUALS("struct D { } ; struct D const d ;",
                      tokenizeAndStringify(code));
        ASSERT_EQUALS("struct D { } ; struct D const d ;",
                      tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void vardecl31() {
        {
            const char code[] = "void foo() { int (*fptr)() = 0; }";
            ASSERT_EQUALS("void foo ( ) { int ( * fptr ) ( ) ; fptr = 0 ; }", tokenizeAndStringify(code));
        }

        {
            const char code[] = "void foo() { int (*fptr)(int) = 0; }";
            ASSERT_EQUALS("void foo ( ) { int ( * fptr ) ( int ) ; fptr = 0 ; }", tokenizeAndStringify(code));
        }
    }

    void vardecl32() {
        {
            const char code[] = "static enum { E } f() { return E; }";
            ASSERT_EQUALS("enum Anonymous0 { E } ; static enum Anonymous0 f ( ) { return E ; }", tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
    }

    void vardecl33() {
        {
            const char code[] = "static enum { E } *f() { return NULL; }";
            ASSERT_EQUALS("enum Anonymous0 { E } ; static enum Anonymous0 * f ( ) { return NULL ; }", tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
    }

    void vardecl34() {
        {
            const char code[] = "static enum { E } const *f() { return NULL; }";
            ASSERT_EQUALS("enum Anonymous0 { E } ; static enum Anonymous0 const * f ( ) { return NULL ; }", tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
    }

    void volatile_variables() {
        {
            const char code[] = "volatile int a=0;\n"
                                "volatile int b=0;\n"
                                "volatile int c=0;\n";

            const std::string actual(tokenizeAndStringify(code));

            ASSERT_EQUALS("volatile int a ; a = 0 ;\nvolatile int b ; b = 0 ;\nvolatile int c ; c = 0 ;", actual);
        }
        {
            const char code[] = "char *volatile s1, *volatile s2;\n"; // #11004

            const std::string actual(tokenizeAndStringify(code));

            ASSERT_EQUALS("char * volatile s1 ; char * volatile s2 ;", actual);
        }
    }


    void simplifyKeyword() {
        {
            const char code[] = "void f (int a [ static 5] );";
            ASSERT_EQUALS("void f ( int a [ 5 ] ) ;", tokenizeAndStringify(code));
        }
        {
            const char in4[] = "struct B final : A { void foo(); };";
            const char out4[] = "struct B : A { void foo ( ) ; } ;";
            ASSERT_EQUALS(out4, tokenizeAndStringify(in4));

            const char in5[] = "struct ArrayItemsValidator final {\n"
                               "    SchemaError validate() const override {\n"
                               "        for (; pos < value.size(); ++pos) {\n"
                               "        }\n"
                               "        return none;\n"
                               "    }\n"
                               "};\n";
            const char out5[] =
                "struct ArrayItemsValidator {\n"
                "SchemaError validate ( ) const override {\n"
                "for ( ; pos < value . size ( ) ; ++ pos ) {\n"
                "}\n"
                "return none ;\n"
                "}\n"
                "} ;";

            ASSERT_EQUALS(out5, tokenizeAndStringify(in5));
            ASSERT_EQUALS(
                "[test.cpp:3:16]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable pos [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }
        {
            // Ticket #8679
            const char code[] = "thread_local void *thread_local_var; "
                                "__thread void *thread_local_var_2;";
            ASSERT_EQUALS("static void * thread_local_var ; "
                          "void * thread_local_var_2 ;", tokenizeAndStringify(code));
        }

        ASSERT_EQUALS("class Fred { } ;", tokenizeAndStringify("class DLLEXPORT Fred final { };"));
    }

    void implicitIntConst() {
        ASSERT_EQUALS("const int x ;", tokenizeAndStringify("const x;", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("const int * x ;", tokenizeAndStringify("const *x;", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("const int * f ( ) ;", tokenizeAndStringify("const *f();", dinit(TokenizeOptions, $.cpp = false)));
    }

    void implicitIntExtern() {
        ASSERT_EQUALS("extern int x ;", tokenizeAndStringify("extern x;", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("extern int * x ;", tokenizeAndStringify("extern *x;", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("const int * f ( ) ;", tokenizeAndStringify("const *f();", dinit(TokenizeOptions, $.cpp = false)));
    }

    /**
     * tokenize "signed i" => "signed int i"
     */
    void implicitIntSigned1() {
        {
            const char code1[] = "void foo ( signed int , float ) ;";
            ASSERT_EQUALS(code1, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "signed i ;";
            const char code2[] = "signed int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "signed int i ;";
            ASSERT_EQUALS(code1, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "int signed i ;";
            const char code2[] = "signed int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "void f() { for (signed i=0; i<10; i++) {} }";
            const char code2[] = "void f ( ) { for ( signed int i = 0 ; i < 10 ; i ++ ) { } }";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }
    }

    /**
     * tokenize "unsigned i" => "unsigned int i"
     * tokenize "unsigned" => "unsigned int"
     */
    void implicitIntUnsigned1() {
        // No changes..
        {
            const char code[] = "void foo ( unsigned int , float ) ;";
            ASSERT_EQUALS(code, tokenizeAndStringify(code));
        }

        // insert "int" after "unsigned"..
        {
            const char code1[] = "unsigned i ;";
            const char code2[] = "unsigned int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        {
            const char code1[] = "int unsigned i ;";
            const char code2[] = "unsigned int i ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        // insert "int" after "unsigned"..
        {
            const char code1[] = "void f() { for (unsigned i=0; i<10; i++) {} }";
            const char code2[] = "void f ( ) { for ( unsigned int i = 0 ; i < 10 ; i ++ ) { } }";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }

        // "extern unsigned x;" => "extern int x;"
        {
            const char code1[] = "; extern unsigned x;";
            const char code2[] = "; extern unsigned int x ;";
            ASSERT_EQUALS(code2, tokenizeAndStringify(code1));
        }
    }

    void implicitIntUnsigned2() {
        const char code[] = "i = (unsigned)j;";
        const char expected[] = "i = ( unsigned int ) j ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    // simplify "unsigned" when using templates..
    void implicitIntUnsigned3() {
        {
            const char code[] = "; foo<unsigned>();";
            const char expected[] = "; foo < unsigned int > ( ) ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        {
            const char code[] = "; foo<unsigned int>();";
            const char expected[] = "; foo < unsigned int > ( ) ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
    }

    void simplifyStdType() { // #4947, #4950, #4951
        // unsigned long long
        {
            const char code[] = "long long unsigned int x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "long long int unsigned x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned long long int x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned int long long x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int unsigned long long x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int long long unsigned x;";
            const char expected[] = "unsigned long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        // signed long long
        {
            const char code[] = "long long signed int x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "long long int signed x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed long long int x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed int long long x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int signed long long x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int long long signed x;";
            const char expected[] = "signed long long x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        // unsigned short
        {
            const char code[] = "short unsigned int x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "short int unsigned x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned short int x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned int short x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int unsigned short x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int short unsigned x;";
            const char expected[] = "unsigned short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        // signed short
        {
            const char code[] = "short signed int x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "short int signed x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed short int x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "signed int short x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int signed short x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "int short signed x;";
            const char expected[] = "signed short x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "unsigned static short const int i;";
            const char expected[] = "static const unsigned short i ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "float complex x;";
            const char expected[] = "float complex x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
        {
            const char code[] = "float complex x;";
            const char expected[] = "_Complex float x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
        {
            const char code[] = "complex float x;";
            const char expected[] = "_Complex float x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
        {
            const char code[] = "complex long double x;";
            const char expected[] = "_Complex long double x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
        {
            const char code[] = "long double complex x;";
            const char expected[] = "_Complex long double x ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
        {
            const char code[] = "double complex;";
            const char expected[] = "double complex ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }
    }

    void createLinks() {
        {
            const char code[] = "class A{\n"
                                " void f() {}\n"
                                "};";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            // A body {}
            ASSERT_EQUALS(true, tok->linkAt(2) == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(2));

            // f body {}
            ASSERT_EQUALS(true, tok->linkAt(7) == tok->tokAt(8));
            ASSERT_EQUALS(true, tok->linkAt(8) == tok->tokAt(7));

            // f ()
            ASSERT_EQUALS(true, tok->linkAt(5) == tok->tokAt(6));
            ASSERT_EQUALS(true, tok->linkAt(6) == tok->tokAt(5));

            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f(){\n"
                                " char a[10];\n"
                                " char *b ; b = new char[a[0]];\n"
                                "};";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            // a[10]
            ASSERT_EQUALS(true, tok->linkAt(7) == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(7));

            // new char[]
            ASSERT_EQUALS(true, tok->linkAt(19) == tok->tokAt(24));
            ASSERT_EQUALS(true, tok->linkAt(24) == tok->tokAt(19));

            // a[0]
            ASSERT_EQUALS(true, tok->linkAt(21) == tok->tokAt(23));
            ASSERT_EQUALS(true, tok->linkAt(23) == tok->tokAt(21));

            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void f(){\n"
                                " foo(g());\n"
                                "};";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            // foo(
            ASSERT_EQUALS(true, tok->linkAt(6) == tok->tokAt(10));
            ASSERT_EQUALS(true, tok->linkAt(10) == tok->tokAt(6));

            // g(
            ASSERT_EQUALS(true, tok->linkAt(8) == tok->tokAt(9));
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(8));

            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "bool foo(C<z> a, bar<int, x<float>>& f, int b) {\n"
                                "    return(a<b && b>f);\n"
                                "}";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            // template<
            ASSERT_EQUALS(true, tok->tokAt(6) == tok->linkAt(4));
            ASSERT_EQUALS(true, tok->tokAt(4) == tok->linkAt(6));

            // bar<
            ASSERT_EQUALS(true, tok->tokAt(17) == tok->linkAt(10));
            ASSERT_EQUALS(true, tok->tokAt(10) == tok->linkAt(17));

            // x<
            ASSERT_EQUALS(true, tok->tokAt(16) == tok->linkAt(14));
            ASSERT_EQUALS(true, tok->tokAt(14) == tok->linkAt(16));

            // a<b && b>f
            ASSERT_EQUALS(true, nullptr == tok->linkAt(28));
            ASSERT_EQUALS(true, nullptr == tok->linkAt(32));

            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void foo() {\n"
                                "    return static_cast<bar>(a);\n"
                                "}";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            // static_cast<
            ASSERT_EQUALS(true, tok->tokAt(9) == tok->linkAt(7));
            ASSERT_EQUALS(true, tok->tokAt(7) == tok->linkAt(9));

            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "void foo() {\n"
                                "    nvwa<(x > y)> ERROR_nnn;\n"
                                "}";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            // nvwa<(x > y)>
            ASSERT_EQUALS(true, tok->tokAt(12) == tok->linkAt(6));
            ASSERT_EQUALS(true, tok->tokAt(6) == tok->linkAt(12));

            ASSERT_EQUALS("", errout_str());
        }

        {
            // #4860
            const char code[] = "class A : public B<int> {};";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            // B<..>
            ASSERT_EQUALS(true, tok->tokAt(5) == tok->linkAt(7));
            ASSERT_EQUALS(true, tok->linkAt(5) == tok->tokAt(7));

            ASSERT_EQUALS("", errout_str());
        }

        {
            // #4860
            const char code[] = "Bar<Typelist< int, Typelist< int, Typelist< int, FooNullType>>>>::set(1, 2, 3);";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->tokAt(1) == tok->linkAt(18));
            ASSERT_EQUALS(true, tok->tokAt(3) == tok->linkAt(17));
            ASSERT_EQUALS(true, tok->tokAt(7) == tok->linkAt(16));
            ASSERT_EQUALS(true, tok->tokAt(11) == tok->linkAt(15));

            ASSERT_EQUALS("", errout_str());
        }

        {
            // #5627
            const char code[] = "new Foo<Bar>[10];";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->tokAt(2) == tok->linkAt(4));
            ASSERT_EQUALS(true, tok->tokAt(4) == tok->linkAt(2));
            ASSERT_EQUALS(true, tok->tokAt(5) == tok->linkAt(7));
            ASSERT_EQUALS(true, tok->tokAt(7) == tok->linkAt(5));

            ASSERT_EQUALS("", errout_str());
        }
        {
            // #6242
            const char code[] = "func = integral_<uchar, int, double>;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->tokAt(3) == tok->linkAt(9));
            ASSERT_EQUALS(true, tok->linkAt(3) == tok->tokAt(9));

            ASSERT_EQUALS("", errout_str());
        }

        {
            // if (a < b || c > d) { }
            const char code[] = "{ if (a < b || c > d); }";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(3) == nullptr);
        }

        {
            // bool f = a < b || c > d
            const char code[] = "bool f = a < b || c > d;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(4) == nullptr);
        }

        {
            // template
            const char code[] = "a < b || c > d;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(5));
        }

        {
            // if (a < ... > d) { }
            const char code[] = "{ if (a < b || c == 3 || d > e); }";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(3) == nullptr);
        }

        {
            // template
            const char code[] = "a<b==3 || c> d;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(7));
        }

        {
            // template
            const char code[] = "a<b || c==4> d;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(7));
        }

        {
            const char code[] = "template < f = b || c > struct S;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(7));
            ASSERT_EQUALS(true, tok->tokAt(1) == tok->linkAt(7));
        }

        {
            const char code[] = "struct A : B<c&&d> {};";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(4) == tok->tokAt(8));
            ASSERT_EQUALS(true, tok->tokAt(4) == tok->linkAt(8));
        }

        {
            const char code[] = "Data<T&&>;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();
            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(4));
            ASSERT_EQUALS(true, tok->tokAt(1) == tok->linkAt(4));
        }

        {
            // #6601
            const char code[] = "template<class R> struct FuncType<R(&)()> : FuncType<R()> { };";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = tokenizer.tokens();

            ASSERT_EQUALS(true, tok->linkAt(1) == tok->tokAt(4)); // <class R>
            ASSERT_EQUALS(true, tok->linkAt(7) == tok->tokAt(14)); // <R(&)()>
            ASSERT_EQUALS(true, tok->linkAt(9) == tok->tokAt(11)); // (&)
            ASSERT_EQUALS(true, tok->linkAt(12) == tok->tokAt(13)); // ()
            ASSERT_EQUALS(true, tok->linkAt(17) == tok->tokAt(21)); // <R()>
            ASSERT_EQUALS(true, tok->linkAt(19) == tok->tokAt(20)); // ()
            ASSERT_EQUALS(true, tok->linkAt(22) == tok->tokAt(23)); // {}
        }
    }

    void createLinks2() {
        {
            // #7158
            const char code[] = "enum { value = boost::mpl::at_c<B, C> };";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok = Token::findsimplematch(tokenizer.tokens(), "<");
            ASSERT_EQUALS(true, tok->link() == tok->tokAt(4));
            ASSERT_EQUALS(true, tok->linkAt(4) == tok);
        }

        {
            // #7865
            const char code[] = "template <typename T, typename U>\n"
                                "struct CheckedDivOp< T, U, typename std::enable_if<std::is_floating_point<T>::value || std::is_floating_point<U>::value>::type> {\n"
                                "};\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "struct")->tokAt(2);
            const Token *tok2 = Token::findsimplematch(tokenizer.tokens(), "{")->previous();
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #7975
            const char code[] = "template <class C> X<Y&&Z, C*> copy() {};\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "< Y");
            const Token *tok2 = Token::findsimplematch(tok1, "> copy");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #8006
            const char code[] = "C<int> && a = b;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok1 = tokenizer.tokens()->next();
            const Token *tok2 = tok1->tokAt(2);
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #8115
            const char code[] = "void Test(C<int> && c);";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "<");
            const Token *tok2 = tok1->tokAt(2);
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }
        {
            // #8654
            const char code[] = "template<int N> struct A {}; "
                                "template<int... Ns> struct foo : A<Ns>... {};";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *A = Token::findsimplematch(tokenizer.tokens(), "A <");
            ASSERT_EQUALS(true, A->linkAt(1) == A->tokAt(3));
        }
        {
            // #8851
            const char code[] = "template<typename std::enable_if<!(std::value1) && std::value2>::type>"
                                "void basic_json() {}";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT_EQUALS(true, Token::simpleMatch(tokenizer.tokens()->linkAt(1), "> void"));
        }

        {
            // #9094 - template usage or comparison?
            const char code[] = "a = f(x%x<--a==x>x);";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr == Token::findsimplematch(tokenizer.tokens(), "<")->link());
        }

        {
            // #11319
            const char code[] = "using std::same_as;\n"
                                "template<same_as<int> T>\n"
                                "void f();";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "template <");
            const Token *tok2 = Token ::findsimplematch(tokenizer.tokens(), "same_as <");
            ASSERT(tok1->linkAt(1) == tok1->tokAt(7));
            ASSERT(tok2->linkAt(1) == tok2->tokAt(3));
        }

        {
            // #9131 - template usage or comparison?
            const char code[] = "using std::list; list<t *> l;";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr != Token::findsimplematch(tokenizer.tokens(), "<")->link());
        }

        {
            const char code[] = "using std::set;\n"
                                "void foo()\n"
                                "{\n"
                                "    for (set<ParticleSource*>::iterator i = sources.begin(); i != sources.end(); ++i) {}\n"
                                "}";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr != Token::findsimplematch(tokenizer.tokens(), "<")->link());
        }

        {
            // #8890
            const char code[] = "void f() {\n"
                                "  a<> b;\n"
                                "  b.a<>::c();\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr != Token::findsimplematch(tokenizer.tokens(), "> ::")->link());
        }

        {
            // #9136
            const char code[] = "template <char> char * a;\n"
                                "template <char... b> struct c {\n"
                                "  void d() { a<b...>[0]; }\n"
                                "};\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr != Token::findsimplematch(tokenizer.tokens(), "> [")->link());
        }

        {
            // #9057
            const char code[] = "template <bool> struct a;\n"
                                "template <bool b, typename> using c = typename a<b>::d;\n"
                                "template <typename e> using f = c<e() && sizeof(int), int>;\n"
                                "template <typename e, typename = f<e>> struct g {};\n"
                                "template <typename e> using baz = g<e>;\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr != Token::findsimplematch(tokenizer.tokens(), "> ;")->link());
        }

        {
            // #9141
            const char code[] = "struct a {\n"
                                "  typedef int b;\n"
                                "  operator b();\n"
                                "};\n"
                                "template <int> using c = a;\n"
                                "template <int d> c<d> e;\n"
                                "auto f = -e<1> == 0;\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr != Token::findsimplematch(tokenizer.tokens(), "> ==")->link());
        }

        {
            // #9145
            const char code[] = "template <typename a, a> struct b {\n"
                                "  template <typename c> constexpr void operator()(c &&) const;\n"
                                "};\n"
                                "template <int d> struct e { b<int, d> f; };\n"
                                "template <int g> using h = e<g>;\n"
                                "template <int g> h<g> i;\n"
                                "template <typename a, a d>\n"
                                "template <typename c>\n"
                                "constexpr void b<a, d>::operator()(c &&) const {\n"
                                "  i<3>.f([] {});\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(nullptr != Token::findsimplematch(tokenizer.tokens(), "> . f (")->link());
        }

        {
            // #10491
            const char code[] = "template <template <class> class> struct a;\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< class");
            const Token* tok2 = Token::findsimplematch(tok1, "> class");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #10491
            const char code[] = "template <template <class> class> struct a;\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< template");
            const Token* tok2 = Token::findsimplematch(tok1, "> struct");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #10552
            const char code[] = "v.value<QPair<int, int>>()\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< QPair");
            const Token* tok2 = Token::findsimplematch(tok1, "> (");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #10552
            const char code[] = "v.value<QPair<int, int>>()\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< int");
            const Token* tok2 = Token::findsimplematch(tok1, "> > (");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #10615
            const char code[] = "struct A : public B<__is_constructible()>{};\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< >");
            const Token* tok2 = Token::findsimplematch(tok1, "> { } >");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #10664
            const char code[] = "class C1 : public T1<D2<C2>const> {};\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< C2");
            const Token* tok2 = Token::findsimplematch(tok1, "> const");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #11453
            const char code[] = "template<typename T>\n"
                                "std::array<T, 1> a{};\n"
                                "void f() {\n"
                                "    if (a<int>[0]) {}\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< int");
            const Token* tok2 = Token::findsimplematch(tok1, "> [");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            // #11490
            const char code[] = "void g() {\n"
                                "    int b[2] = {};\n"
                                "    if (b[idx<1>]) {}\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< 1");
            const Token* tok2 = Token::findsimplematch(tok1, "> ]");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        { // #11275
            const char code[] = "void f() {\n"
                                "    []<typename T>() {};\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< T");
            const Token* tok2 = Token::findsimplematch(tok1, "> (");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        { // #11810
            const char code[] = "void f() {\n"
                                "    auto g = [] <typename A, typename B> (A a, B&& b) { return a < b; };\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< A");
            const Token* tok2 = Token::findsimplematch(tok1, "> (");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            const char code[] = "void f() {\n"
                                "    auto g = [] <typename U> () {\n"
                                "        return [] <typename T> () {};\n"
                                "    };\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< T");
            const Token* tok2 = Token::findsimplematch(tok1, "> (");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            const char code[] = "struct S {\n" // #11840
                                "    template<typename T, typename U>\n"
                                "    void operator() (int);\n"
                                "};\n"
                                "void f() {\n"
                                "    S s;\n"
                                "    s.operator()<int, int>(1);\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< int");
            const Token* tok2 = Token::findsimplematch(tok1, "> (");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            const char code[] = "template<typename T>\n"
                                "auto f() {\n"
                                "    return std::integral_constant<\n"
                                "        bool,\n"
                                "        std::is_same<typename T::size_type, std::size_t>::value && std::is_same<typename T::size_type, std::size_t>::value\n"
                                "        >();\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< bool");
            const Token* tok2 = Token::findsimplematch(tok1, "> (");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            const char code[] = "double f() {\n"
                                "    return std::is_same_v<int, double> ? 1e-7 : 1e-3;\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< int");
            const Token* tok2 = Token::findsimplematch(tok1, "> ?");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            const char code[] = "template <typename Fn, typename... Args>\n" // #14612
                                "void f(Fn&& fn, Args&&... args) {\n"
                                "    static_assert(std::is_invocable_v<Fn&&, Args&&...>);\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< Fn");
            const Token* tok2 = Token::findsimplematch(tok1, "> )");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }

        {
            const char code[] = "template <typename Fn, typename... Args>\n" // #14615
                                "void f(Fn&& fn, Args&&... args) {\n"
                                "    constexpr bool b{ std::is_invocable_v<Fn&&, Args&&...> };\n"
                                "}\n";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            const Token* tok1 = Token::findsimplematch(tokenizer.tokens(), "< Fn");
            const Token* tok2 = Token::findsimplematch(tok1, "> }");
            ASSERT_EQUALS(true, tok1->link() == tok2);
            ASSERT_EQUALS(true, tok2->link() == tok1);
        }
    }

    void simplifyString() {
        TokenList tokenlist{settingsDefault, Standards::Language::CPP};
        TokenizerTest tokenizer(std::move(tokenlist), *this);
        ASSERT_EQUALS("\"abc\"", tokenizer.simplifyString("\"abc\""));
        ASSERT_EQUALS("\"\n\"", tokenizer.simplifyString("\"\\xa\""));
        ASSERT_EQUALS("\"3\"", tokenizer.simplifyString("\"\\x33\""));
        ASSERT_EQUALS("\"33\"", tokenizer.simplifyString("\"\\x333\""));

        ASSERT_EQUALS("\"a\"", tokenizer.simplifyString("\"\\x61\""));
        ASSERT_EQUALS("\"\n1\"", tokenizer.simplifyString("\"\\0121\""));

        ASSERT_EQUALS("\"3\"", tokenizer.simplifyString("\"\\x33\""));
        ASSERT_EQUALS("\" 0\"", tokenizer.simplifyString("\"\\0400\""));

        ASSERT_EQUALS("\"\\nhello\"", tokenizer.simplifyString("\"\\nhello\""));

        ASSERT_EQUALS("\"aaa\"", tokenizer.simplifyString("\"\\x61\\x61\\x61\""));
        ASSERT_EQUALS("\"\n1\n1\n1\"", tokenizer.simplifyString("\"\\0121\\0121\\0121\""));

        ASSERT_EQUALS("\"\\\\x61\"", tokenizer.simplifyString("\"\\\\x61\""));
        ASSERT_EQUALS("\"b\"", tokenizer.simplifyString("\"\\x62\""));
        ASSERT_EQUALS("\" 7\"", tokenizer.simplifyString("\"\\0407\""));

        // terminate a string at null character.
        ASSERT_EQUALS(std::string("\"a") + '\0' + "\"", tokenizer.simplifyString("\"a\\0\""));
    }

    void simplifyConst() {
        ASSERT_EQUALS("void foo ( ) { const int x ; }",
                      tokenizeAndStringify("void foo(){ int const x;}"));

        ASSERT_EQUALS("void foo ( ) { { } const long x ; }",
                      tokenizeAndStringify("void foo(){ {} long const x;}"));

        ASSERT_EQUALS("void foo ( int b , const unsigned int x ) { }",
                      tokenizeAndStringify("void foo(int b,unsigned const x){}"));

        ASSERT_EQUALS("void foo ( ) { bar ( ) ; const char x ; }",
                      tokenizeAndStringify("void foo(){ bar(); char const x;}"));

        ASSERT_EQUALS("void foo ( const char x ) { }",
                      tokenizeAndStringify("void foo(char const x){}"));

        ASSERT_EQUALS("void foo ( int b , const char x ) { }",
                      tokenizeAndStringify("void foo(int b,char const x){}"));

        ASSERT_EQUALS("void foo ( ) { int * const x ; }",
                      tokenizeAndStringify("void foo(){ int * const x;}"));

        ASSERT_EQUALS("const int foo ( ) ;", tokenizeAndStringify("int const foo ();"));

        ASSERT_EQUALS("const int x ;", tokenizeAndStringify("int const x;"));
        ASSERT_EQUALS("const unsigned int x ;", tokenizeAndStringify("unsigned const x;"));
        ASSERT_EQUALS("const struct X x ;", tokenizeAndStringify("struct X const x;"));
    }

    void switchCase() {
        ASSERT_EQUALS("void foo ( int i ) { switch ( i ) { case -1 : ; break ; } }",
                      tokenizeAndStringify("void foo (int i) { switch(i) { case -1: break; } }"));
        //ticket #3227
        ASSERT_EQUALS("void foo ( int n ) { switch ( n ) { label : ; case 1 : ; label1 : ; label2 : ; break ; } }",
                      tokenizeAndStringify("void foo(int n){ switch (n){ label: case 1: label1: label2: break; }}"));
        //ticket #8345
        ASSERT_EQUALS("void foo ( ) { switch ( 0 ) { case 0 : ; default : ; } }",
                      tokenizeAndStringify("void foo () { switch(0) case 0 : default : ; }"));
        //ticket #8477
        ASSERT_EQUALS("void foo ( ) { enum Anonymous0 : int { Six = 6 } ; return Six ; }",
                      tokenizeAndStringify("void foo () { enum : int { Six = 6 } ; return Six ; }"));
        // ticket #8281
        ASSERT_NO_THROW(tokenizeAndStringify("void lzma_decode(int i) { "
                                             "  bool state; "
                                             "  switch (i) "
                                             "  while (true) { "
                                             "     state=false; "
                                             "   case 1: "
                                             "      ; "
                                             "  }"
                                             "}"));
        // ticket #8417
        ASSERT_NO_THROW(tokenizeAndStringify("void printOwnedAttributes(int mode) { "
                                             "  switch(mode) case 0: { break; } "
                                             "}"));
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void printOwnedAttributes(int mode) { "
                                                   "  switch(mode) case 0: { break; } case 1: ; "
                                                   "}"),
                              SYNTAX);
    }

    void simplifyPointerToStandardType() {
        // Pointer to standard type
        ASSERT_EQUALS("char buf [ 100 ] ; readlink ( path , buf , 99 ) ;",
                      tokenizeAndStringify("char buf[100] ; readlink(path, &buf[0], 99);",
                                           dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS("void foo ( char * c ) { if ( 1 == ( 1 & c [ 0 ] ) ) { } }",
                      tokenizeAndStringify("void foo(char *c) { if (1==(1 & c[0])) {} }",
                                           dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("", filter_valueflow(errout_str()));

        // Simplification of unknown type - C only
        ASSERT_EQUALS("foo data [ 100 ] ; something ( foo ) ;",
                      tokenizeAndStringify("foo data[100]; something(&foo[0]);", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("", errout_str());

        // C++: No pointer simplification
        ASSERT_EQUALS("foo data [ 100 ] ; something ( & foo [ 0 ] ) ;",
                      tokenizeAndStringify("foo data[100]; something(&foo[0]);"));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyFunctionPointers1() {
        ASSERT_EQUALS("void ( * f ) ( ) ;", tokenizeAndStringify("void (*f)();"));
        ASSERT_EQUALS("void * ( * f ) ( ) ;", tokenizeAndStringify("void *(*f)();"));
        ASSERT_EQUALS("unsigned int ( * f ) ( ) ;", tokenizeAndStringify("unsigned int (*f)();"));
        ASSERT_EQUALS("unsigned int * ( * f ) ( ) ;", tokenizeAndStringify("unsigned int * (*f)();"));
        ASSERT_EQUALS("void ( * f [ 2 ] ) ( ) ;", tokenizeAndStringify("void (*f[2])();"));
        ASSERT_EQUALS("void ( * f [ 2 ] ) ( void ) ;", tokenizeAndStringify("typedef void func_t(void); func_t *f[2];")); // #10581
    }

    void simplifyFunctionPointers2() {
        const char code[] = "typedef void (* PF)();"
                            "void f1 ( ) { }"
                            "PF pf = &f1;"
                            "PF pfs[] = { &f1, &f1 };";
        const char expected[] = "void f1 ( ) { } "
                                "void ( * pf ) ( ) ; pf = & f1 ; "
                                "void ( * pfs [ ] ) ( ) = { & f1 , & f1 } ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void simplifyFunctionPointers3() {
        // Related with ticket #2873
        const char code[] = "void f() {\n"
                            "(void)(xy(*p)(0));"
                            "\n}";
        const char expected[] = "void f ( ) {\n"
                                "( void ) ( xy ( * p ) ( 0 ) ) ;\n"
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:2:12]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable p [valueFlowBailoutIncompleteVar]\n",
            errout_str());
    }

    void simplifyFunctionPointers4() {
        const char code[] = "struct S\n"
                            "{\n"
                            "    typedef void (*FP)();\n"
                            "    virtual FP getFP();\n"
                            "};";
        const char expected[] = "1: struct S\n"
                                "2: {\n"
                                "3:\n"
                                "4: virtual void * getFP ( ) ;\n"
                                "5: } ;\n";
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void simplifyFunctionPointers5() {
        const char code[] = ";void (*fp[])(int a) = {0,0,0};";
        const char expected[] = "1: ; void ( * fp@1 [ ] ) ( int ) = { 0 , 0 , 0 } ;\n"; // TODO: Array dimension
        ASSERT_EQUALS(expected, tokenizeDebugListing(code));
    }

    void simplifyFunctionPointers6() {
        const char code1[] = "void (*fp(void))(int) {}";
        const char expected1[] = "1: void * fp ( ) { }\n";
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1));

        const char code2[] = "std::string (*fp(void))(int);";
        const char expected2[] = "1: std :: string * fp ( ) ;\n";
        ASSERT_EQUALS(expected2, tokenizeDebugListing(code2));
    }

    void simplifyFunctionPointers7() {
        const char code1[] = "void (X::*y)();";
        const char expected1[] = "1: void ( * y@1 ) ( ) ;\n";
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1));
    }

    void simplifyFunctionPointers8() {
        const char code1[] = "int (*f)() throw(int);";
        const char expected1[] = "1: int ( * f@1 ) ( ) ;\n";
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1));
    }

    void simplifyFunctionPointers9() { // function call with function pointer
        const char code1[] = "int f() { (*f)(); }";
        const char expected1[] = "1: int f ( ) { ( * f ) ( ) ; }\n";
        ASSERT_EQUALS(expected1, tokenizeDebugListing(code1));

        const char code2[] = "int f() { return (*f)(); }";
        const char expected2[] = "1: int f ( ) { return ( * f ) ( ) ; }\n";
        ASSERT_EQUALS(expected2, tokenizeDebugListing(code2));

        const char code3[] = "int f() { throw (*f)(); }";
        const char expected3[] = "1: int f ( ) { throw ( * f ) ( ) ; }\n";
        ASSERT_EQUALS(expected3, tokenizeDebugListing(code3));
    }

    void removedeclspec() {
        ASSERT_EQUALS("a b", tokenizeAndStringify("a __declspec ( dllexport ) b"));
        ASSERT_EQUALS("a b", tokenizeAndStringify("a _declspec ( dllexport ) b"));
        ASSERT_EQUALS("int a ;", tokenizeAndStringify("__declspec(thread) __declspec(align(32)) int a;"));
        ASSERT_EQUALS("int i ;", tokenizeAndStringify("__declspec(allocate(\"mycode\")) int i;"));
        ASSERT_EQUALS("struct IUnknown ;", tokenizeAndStringify("struct __declspec(uuid(\"00000000-0000-0000-c000-000000000046\")) IUnknown;"));
        ASSERT_EQUALS("__property int x [ ] ;", tokenizeAndStringify("__declspec(property(get=GetX, put=PutX)) int x[];"));
    }

    void removeattribute() {
        ASSERT_EQUALS("short array [ 3 ] ;", tokenizeAndStringify("short array[3] __attribute__ ((aligned));"));
        ASSERT_EQUALS("int x [ 2 ] ;", tokenizeAndStringify("int x[2] __attribute__ ((packed));"));
        ASSERT_EQUALS("int vecint ;", tokenizeAndStringify("int __attribute__((mode(SI))) __attribute__((vector_size (16))) vecint;"));

        // alternate spelling #5328
        ASSERT_EQUALS("short array [ 3 ] ;", tokenizeAndStringify("short array[3] __attribute ((aligned));"));
        ASSERT_EQUALS("int x [ 2 ] ;", tokenizeAndStringify("int x[2] __attribute ((packed));"));
        ASSERT_EQUALS("int vecint ;", tokenizeAndStringify("int __attribute((mode(SI))) __attribute((vector_size (16))) vecint;"));

        ASSERT_EQUALS("struct Payload_IR_config { uint8_t tap [ 16 ] ; } ;", tokenizeAndStringify("struct __attribute__((packed, gcc_struct)) Payload_IR_config { uint8_t tap[16]; };"));
    }

    void functionAttributeBefore1() {
        const char code[] = "void __attribute__((pure)) __attribute__((nothrow)) __attribute__((const)) func1();\n"
                            "void __attribute__((__pure__)) __attribute__((__nothrow__)) __attribute__((__const__)) func2();\n"
                            "void __attribute__((nothrow)) __attribute__((pure)) __attribute__((const)) func3();\n"
                            "void __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__const__)) func4();\n"
                            "void __attribute__((noreturn)) func5();\n"
                            "void __attribute__((__visibility__(\"default\"))) func6();";
        const char expected[] = "void func1 ( ) ; void func2 ( ) ; void func3 ( ) ; void func4 ( ) ; void func5 ( ) ; void func6 ( ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");
        const Token * func2 = Token::findsimplematch(tokenizer.tokens(), "func2");
        const Token * func3 = Token::findsimplematch(tokenizer.tokens(), "func3");
        const Token * func4 = Token::findsimplematch(tokenizer.tokens(), "func4");
        const Token * func5 = Token::findsimplematch(tokenizer.tokens(), "func5");
        const Token * func6 = Token::findsimplematch(tokenizer.tokens(), "func6");

        ASSERT(func1 && func1->isAttributePure() && func1->isAttributeNothrow() && func1->isAttributeConst() && !func1->isAttributeExport());
        ASSERT(func2 && func2->isAttributePure() && func2->isAttributeNothrow() && func2->isAttributeConst() && !func2->isAttributeExport());
        ASSERT(func3 && func3->isAttributePure() && func3->isAttributeNothrow() && func3->isAttributeConst() && !func3->isAttributeExport());
        ASSERT(func4 && func4->isAttributePure() && func4->isAttributeNothrow() && func4->isAttributeConst() && !func4->isAttributeExport());
        ASSERT(func5 && func5->isAttributeNoreturn());
        ASSERT(func6 && func6->isAttributeExport());
    }

    void functionAttributeBefore2() {
        const char code[] = "extern vas_f *VAS_Fail __attribute__((__noreturn__));";
        const char expected[] = "extern vas_f * VAS_Fail ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token * VAS_Fail = Token::findsimplematch(tokenizer.tokens(), "VAS_Fail");
        ASSERT(VAS_Fail && VAS_Fail->isAttributeNoreturn());
    }

    void functionAttributeBefore3() { // #10978
        const char code[] = "void __attribute__((__noreturn__)) (*func_notret)(void);";
        const char expected[] = "void ( * func_notret ) ( void ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token* func_notret = Token::findsimplematch(tokenizer.tokens(), "func_notret");
        ASSERT(func_notret && func_notret->isAttributeNoreturn());
    }

    void functionAttributeBefore4() {
        const char code[] = "__attribute__((const)) int& foo();";
        const char expected[] = "int & foo ( ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token* foo = Token::findsimplematch(tokenizer.tokens(), "foo");
        ASSERT(foo && foo->isAttributeConst());
    }

    void functionAttributeBefore5() { // __declspec(dllexport)
        const char code[] = "void __declspec(dllexport) func1();\n";
        const char expected[] = "void func1 ( ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");

        ASSERT(func1 && func1->isAttributeExport());
    }

    void functionAttributeAfter1() {
        const char code[] = "void func1() __attribute__((pure)) __attribute__((nothrow)) __attribute__((const));\n"
                            "void func2() __attribute__((__pure__)) __attribute__((__nothrow__)) __attribute__((__const__));\n"
                            "void func3() __attribute__((nothrow)) __attribute__((pure)) __attribute__((const));\n"
                            "void func4() __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__const__));"
                            "void func5() __attribute__((noreturn));";
        const char expected[] = "void func1 ( ) ; void func2 ( ) ; void func3 ( ) ; void func4 ( ) ; void func5 ( ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");
        const Token * func2 = Token::findsimplematch(tokenizer.tokens(), "func2");
        const Token * func3 = Token::findsimplematch(tokenizer.tokens(), "func3");
        const Token * func4 = Token::findsimplematch(tokenizer.tokens(), "func4");
        const Token * func5 = Token::findsimplematch(tokenizer.tokens(), "func5");

        ASSERT(func1 && func1->isAttributePure() && func1->isAttributeNothrow() && func1->isAttributeConst());
        ASSERT(func2 && func2->isAttributePure() && func2->isAttributeNothrow() && func2->isAttributeConst());
        ASSERT(func3 && func3->isAttributePure() && func3->isAttributeNothrow() && func3->isAttributeConst());
        ASSERT(func4 && func4->isAttributePure() && func4->isAttributeNothrow() && func4->isAttributeConst());
        ASSERT(func5 && func5->isAttributeNoreturn());
    }

    void functionAttributeAfter2() {
        const char code[] = "class foo {\n"
                            "public:\n"
                            "    bool operator==(const foo &) __attribute__((__pure__));\n"
                            "};";
        const char expected[] = "class foo { public: bool operator== ( const foo & ) ; } ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "operator==");
        ASSERT(tok && tok->isAttributePure());
    }

    void functionAttributeListBefore() {
        const char code[] = "void __attribute__((pure,nothrow,const)) func1();\n"
                            "void __attribute__((__pure__,__nothrow__,__const__)) func2();\n"
                            "void __attribute__((nothrow,pure,const)) func3();\n"
                            "void __attribute__((__nothrow__,__pure__,__const__)) func4();\n"
                            "void __attribute__((noreturn,format(printf,1,2))) func5();\n"
                            "void __attribute__((__nothrow__)) __attribute__((__pure__,__const__)) func6();\n"
                            "void __attribute__((__nothrow__,__pure__)) __attribute__((__const__)) func7();\n"
                            "void __attribute__((noreturn)) __attribute__(()) __attribute__((nothrow,pure,const)) func8();";
        const char expected[] = "void func1 ( ) ; void func2 ( ) ; void func3 ( ) ; void func4 ( ) ; void func5 ( ) ; "
                                "void func6 ( ) ; void func7 ( ) ; void func8 ( ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");
        const Token * func2 = Token::findsimplematch(tokenizer.tokens(), "func2");
        const Token * func3 = Token::findsimplematch(tokenizer.tokens(), "func3");
        const Token * func4 = Token::findsimplematch(tokenizer.tokens(), "func4");
        const Token * func5 = Token::findsimplematch(tokenizer.tokens(), "func5");
        const Token * func6 = Token::findsimplematch(tokenizer.tokens(), "func6");
        const Token * func7 = Token::findsimplematch(tokenizer.tokens(), "func7");
        const Token * func8 = Token::findsimplematch(tokenizer.tokens(), "func8");

        ASSERT(func1 && func1->isAttributePure() && func1->isAttributeNothrow() && func1->isAttributeConst());
        ASSERT(func2 && func2->isAttributePure() && func2->isAttributeNothrow() && func2->isAttributeConst());
        ASSERT(func3 && func3->isAttributePure() && func3->isAttributeNothrow() && func3->isAttributeConst());
        ASSERT(func4 && func4->isAttributePure() && func4->isAttributeNothrow() && func4->isAttributeConst());
        ASSERT(func5 && func5->isAttributeNoreturn());
        ASSERT(func6 && func6->isAttributePure() && func6->isAttributeNothrow() && func6->isAttributeConst());
        ASSERT(func7 && func7->isAttributePure() && func7->isAttributeNothrow() && func7->isAttributeConst());
        ASSERT(func8 && func8->isAttributeNoreturn() && func8->isAttributePure() && func8->isAttributeNothrow() && func8->isAttributeConst());
    }

    void functionAttributeListAfter() {
        const char code[] = "void func1() __attribute__((pure,nothrow,const));\n"
                            "void func2() __attribute__((__pure__,__nothrow__,__const__));\n"
                            "void func3() __attribute__((nothrow,pure,const));\n"
                            "void func4() __attribute__((__nothrow__,__pure__,__const__));\n"
                            "void func5() __attribute__((noreturn,format(printf,1,2)));\n"
                            "void func6() __attribute__((__nothrow__)) __attribute__((__pure__,__const__));\n"
                            "void func7() __attribute__((__nothrow__,__pure__)) __attribute__((__const__));\n"
                            "void func8() __attribute__((noreturn)) __attribute__(()) __attribute__((nothrow,pure,const));";
        const char expected[] = "void func1 ( ) ; void func2 ( ) ; void func3 ( ) ; void func4 ( ) ; void func5 ( ) ; "
                                "void func6 ( ) ; void func7 ( ) ; void func8 ( ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");
        const Token * func2 = Token::findsimplematch(tokenizer.tokens(), "func2");
        const Token * func3 = Token::findsimplematch(tokenizer.tokens(), "func3");
        const Token * func4 = Token::findsimplematch(tokenizer.tokens(), "func4");
        const Token * func5 = Token::findsimplematch(tokenizer.tokens(), "func5");
        const Token * func6 = Token::findsimplematch(tokenizer.tokens(), "func6");
        const Token * func7 = Token::findsimplematch(tokenizer.tokens(), "func7");
        const Token * func8 = Token::findsimplematch(tokenizer.tokens(), "func8");

        ASSERT(func1 && func1->isAttributePure() && func1->isAttributeNothrow() && func1->isAttributeConst());
        ASSERT(func2 && func2->isAttributePure() && func2->isAttributeNothrow() && func2->isAttributeConst());
        ASSERT(func3 && func3->isAttributePure() && func3->isAttributeNothrow() && func3->isAttributeConst());
        ASSERT(func4 && func4->isAttributePure() && func4->isAttributeNothrow() && func4->isAttributeConst());
        ASSERT(func5 && func5->isAttributeNoreturn());
        ASSERT(func6 && func6->isAttributePure() && func6->isAttributeNothrow() && func6->isAttributeConst());
        ASSERT(func7 && func7->isAttributePure() && func7->isAttributeNothrow() && func7->isAttributeConst());
        ASSERT(func8 && func8->isAttributeNoreturn() && func8->isAttributePure() && func8->isAttributeNothrow() && func8->isAttributeConst());
    }

    void functionAttributeListAfter2() {
        const char code[] = "[[noreturn]] void func1(const char *format, ...) __attribute__((format(printf, 1, 2)));\n" // #14181
                            "void func2() __attribute__((unused));\n"; // #14183
        const char expected[] = "void func1 ( const char * format , ... ) ; void func2 ( ) ;";

        // tokenize..
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        // Expected result..
        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token * func1 = Token::findsimplematch(tokenizer.tokens(), "func1");
        const Token * func2 = Token::findsimplematch(tokenizer.tokens(), "func2");

        ASSERT(func1 && func1->isAttributeNoreturn());
        ASSERT(func2 && func2->isAttributeUnused());
    }

    void cppMaybeUnusedBefore() {
        const char code[] = "[[maybe_unused]] int var {};";
        const char expected[] = "int var { } ;";

        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token *var = Token::findsimplematch(tokenizer.tokens(), "var");
        ASSERT(var && var->isAttributeMaybeUnused());
    }

    void cppMaybeUnusedAfter1() {
        const char code[] = "int var [[maybe_unused]] {};";
        const char expected[] = "int var { } ;";

        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token *var = Token::findsimplematch(tokenizer.tokens(), "var");
        ASSERT(var && var->isAttributeMaybeUnused());
    }

    void cppMaybeUnusedAfter2() {
        const char code[] = "std::string var [[maybe_unused]];";
        const char expected[] = "std :: string var ;";

        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token *var = Token::findsimplematch(tokenizer.tokens(), "var");
        ASSERT(var && var->isAttributeMaybeUnused());
    }

    void cppMaybeUnusedStructuredBinding() {
        const char code[] = "[[maybe_unused]] auto [var1, var2] = f();";
        const char expected[] = "auto [ var1 , var2 ] = f ( ) ;";

        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token *var1 = Token::findsimplematch(tokenizer.tokens(), "var1");
        ASSERT(var1 && var1->isAttributeMaybeUnused());
        const Token *var2 = Token::findsimplematch(tokenizer.tokens(), "var2");
        ASSERT(var2 && var2->isAttributeMaybeUnused());
    }

    void attributeAlignasBefore() {
        const char code[] = "alignas(long) unsigned char buffer[sizeof(long)];";
        const char expected[] = "char buffer [ sizeof ( long ) ] ;";
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token *var = Token::findsimplematch(tokenizer.tokens(), "buffer");
        ASSERT(var);
        ASSERT(var->hasAttributeAlignas());
        ASSERT(var->getAttributeAlignas().size() == 1);
        ASSERT_EQUALS(var->getAttributeAlignas()[0], "long");
    }

    void attributeAlignasAfter() {
        const char code[] = "unsigned char buffer[sizeof(long)] alignas(long);";
        const char expected[] = "char buffer [ sizeof ( long ) ] ;";
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        ASSERT_EQUALS(expected, tokenizer.tokens()->stringifyList(nullptr, false));

        const Token *var = Token::findsimplematch(tokenizer.tokens(), "buffer");
        ASSERT(var);
        ASSERT(var->hasAttributeAlignas());
        ASSERT(var->getAttributeAlignas().size() == 1);
        ASSERT_EQUALS(var->getAttributeAlignas()[0], "long");
    }

    void splitTemplateRightAngleBrackets() {
        {
            const char code[] = "; z = x < 0 ? x >> y : x >> y;";
            ASSERT_EQUALS("; z = x < 0 ? x >> y : x >> y ;", tokenizeAndStringify(code));
        }
        {
            // ftp://ftp.de.debian.org/debian/pool/main/f/ffmpeg/ffmpeg_4.3.1.orig.tar.xz
            // ffmpeg-4.3.1/libavcodec/mpeg4videodec.c:376
            const char code[] = "void f ( ) {\n"
                                "    int shift_y = ctx->sprite_shift[0];\n"
                                "    int shift_c = ctx->sprite_shift[1];\n"
                                "    if ( shift_c < 0 || shift_y < 0 ||\n"
                                "         FFABS ( sprite_offset [ 0 ] [ i ] ) >= INT_MAX >> shift_y ||\n"
                                "         FFABS ( sprite_offset [ 1 ] [ i ] ) >= INT_MAX >> shift_c ||\n"
                                "         FFABS ( sprite_delta [ 0 ] [ i ] ) >= INT_MAX >> shift_y ||\n"
                                "         FFABS ( sprite_delta [ 1 ] [ i ] ) >= INT_MAX >> shift_y ) ;\n"
                                "}";
            ASSERT_EQUALS(std::string::npos, tokenizeAndStringify(code).find("> >"));
            ASSERT_EQUALS(
                "[test.cpp:2:24]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable sprite_shift [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }
        {
            const char code[] = "struct S { bool vector; };\n"
                                "struct T { std::vector<std::shared_ptr<int>> v; };\n";
            ASSERT_EQUALS("struct S { bool vector ; } ;\n"
                          "struct T { std :: vector < std :: shared_ptr < int > > v ; } ;",
                          tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }
    }

    void cpp03template1() {
        {
            const char code[] = "template<typename> struct extent {};";
            ASSERT_EQUALS("template < typename > struct extent { } ;", tokenizeAndStringify(code));
        }
        {
            const char code[] = "template<typename> struct extent;";
            ASSERT_EQUALS("template < typename > struct extent ;", tokenizeAndStringify(code));
        }
        {
            const char code[] = "template<typename, unsigned = 0> struct extent;";
            ASSERT_EQUALS("template < typename , unsigned int = 0 > struct extent ;", tokenizeAndStringify(code));
        }
    }

    void cpp0xtemplate1() {
        const char code[] = "template <class T>\n"
                            "void fn2 (T t = []{return 1;}())\n"
                            "{}\n"
                            "int main()\n"
                            "{\n"
                            "  fn2<int>();\n"
                            "}\n";
        ASSERT_EQUALS("void fn2<int> ( int t = [ ] { return 1 ; } ( ) ) ;\n"
                      "\n"
                      "int main ( )\n"
                      "{\n"
                      "fn2<int> ( ) ;\n"
                      "}\n"
                      "void fn2<int> ( int t = [ ] { return 1 ; } ( ) )\n"
                      "{ }", tokenizeAndStringify(code));
    }

    void cpp0xtemplate2() {
        // tokenize ">>" into "> >"
        const char code[] = "list<list<int>> ints;\n";
        ASSERT_EQUALS("list < list < int > > ints ;", tokenizeAndStringify(code));
    }

    void cpp0xtemplate3() {
        // #2549
        const char code[] = "template<class T, T t = (T)0>\n"
                            "struct S\n"
                            "{};\n"
                            "S<int> s;\n";
        ASSERT_EQUALS("struct S<int,0> ;\n"
                      "\n"
                      "\n"
                      "S<int,0> s ;\n"
                      "struct S<int,0>\n"
                      "{ } ;",
                      tokenizeAndStringify(code));
    }

    void cpp0xtemplate4() { // #6181, #6354, #6414
        // #6181
        ASSERT_NO_THROW(tokenizeAndStringify("class A; "
                                             "template <class T> class Disposer; "
                                             "template <typename T, class D = Disposer<T>> class Shim {}; "
                                             "class B : public Shim<A> {};"));
        // #6354 (segmentation fault)
        (void)tokenizeAndStringify("template <class ELFT> class ELFObjectImage {}; "
                                   "ObjectImage *createObjectImage() { "
                                   "  return new ELFObjectImage<ELFType<little>>(Obj); "
                                   "} "
                                   "void resolveX86_64Relocation() { "
                                   "  reinterpret_cast<int>(0); "
                                   "}");
        // #6414
        ASSERT_NO_THROW(tokenizeAndStringify("template<typename value_type, typename function_type> "
                                             "value_type Base(const value_type x, const value_type dx, function_type func, int type_deriv) { "
                                             "   return 0.0; "
                                             "}; "
                                             "namespace { "
                                             "  template<class DC> class C { "
                                             "    void Fun(int G, const double x); "
                                             "  }; "
                                             "  template<class DC> void C<DC>::Fun(int G, const double x) {"
                                             "    Base<double, CDFFunctor<DC>>(2, 2, f, 0); "
                                             "  }; "
                                             "  template<class DC> class C2 {}; "
                                             "}"));

        ignore_errout();
    }

    void cpp0xtemplate5() { // #9154
        {
            const char code[] = "struct s<x<u...>>;";
            ASSERT_EQUALS("struct s < x < u ... > > ;",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "template <class f> using c = e<i<q<f,r>,b...>>;";
            ASSERT_EQUALS("template < class f > using c = e < i < q < f , r > , b ... > > ;",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "struct s<x<u...>> { };";
            ASSERT_EQUALS("struct s < x < u ... > > { } ;",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "struct q : s<x<u...>> { };";
            ASSERT_EQUALS("struct q : s < x < u ... > > { } ;",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "struct q : private s<x<u...>> { };";
            ASSERT_EQUALS("struct q : private s < x < u ... > > { } ;",
                          tokenizeAndStringify(code));
        }
    }

    void cpp14template() { // Ticket #6708
        (void)tokenizeAndStringify("template <typename T> "
                                   "decltype(auto) forward(T& t) { return 0; }");
        ASSERT_EQUALS("[test.cpp:1:32]: (debug) auto token with no type. [autoNoType]\n", errout_str());
    }

    void arraySize() {
        ASSERT_EQUALS("; int a [ 3 ] = { 1 , 2 , 3 } ;", tokenizeAndStringify(";int a[]={1,2,3};"));
        ASSERT_EQUALS("; int a [ 3 ] = { 1 , 2 , 3 } ;", tokenizeAndStringify(";int a[]={1,2,3,};"));
        ASSERT_EQUALS("; foo a [ 3 ] = { { 1 , 2 } , { 3 , 4 } , { 5 , 6 } } ;", tokenizeAndStringify(";foo a[]={{1,2},{3,4},{5,6}};"));
        ASSERT_EQUALS("; int a [ 1 ] = { foo < bar1 , bar2 > ( 123 , 4 ) } ;", tokenizeAndStringify(";int a[]={foo<bar1,bar2>(123,4)};"));
        ASSERT_EQUALS("; int a [ 2 ] = { b > c ? 1 : 2 , 3 } ;", tokenizeAndStringify(";int a[]={ b>c?1:2,3};"));
        ASSERT_EQUALS("int main ( ) { int a [ 2 ] = { b < c ? 1 : 2 , 3 } }", tokenizeAndStringify("int main(){int a[]={b<c?1:2,3}}"));
        ASSERT_EQUALS(
            "[test.cpp:1:21]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable b [valueFlowBailoutIncompleteVar]\n",
            errout_str());
        ASSERT_EQUALS("; int a [ 3 ] = { ABC , 2 , 3 } ;", tokenizeAndStringify(";int a[]={ABC,2,3};"));
        ASSERT_EQUALS("; int a [ 3 ] = { [ 2 ] = 5 } ;", tokenizeAndStringify(";int a[]={ [2] = 5 };"));
        ASSERT_EQUALS("; int a [ 5 ] = { 1 , 2 , [ 2 ] = 5 , 3 , 4 } ;", tokenizeAndStringify(";int a[]={ 1, 2, [2] = 5, 3, 4 };"));
        ASSERT_EQUALS("; int a [ ] = { 1 , 2 , [ x ] = 5 , 3 , 4 } ;", tokenizeAndStringify(";int a[]={ 1, 2, [x] = 5, 3, 4 };"));
        ASSERT_EQUALS("; const char c [ 4 ] = \"abc\" ;", tokenizeAndStringify(";const char c[] = { \"abc\" };"));
    }

    void arraySizeAfterValueFlow() {
        const char code[] = "enum {X=10}; int a[] = {[X]=1};";
        ASSERT_EQUALS("enum Anonymous0 { X = 10 } ; int a [ 11 ] = { [ X ] = 1 } ;", tokenizeAndStringify(code));
    }

    void labels() {
        ASSERT_EQUALS("void f ( ) { ab : ; a = 0 ; }", tokenizeAndStringify("void f() { ab: a=0; }"));
        //ticket #3176
        ASSERT_EQUALS("void f ( ) { ab : ; ( * func ) ( ) ; }", tokenizeAndStringify("void f() { ab: (*func)(); }"));
        //with '*' operator
        ASSERT_EQUALS("void f ( ) { ab : ; * b = 0 ; }", tokenizeAndStringify("void f() { ab: *b=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; * * b = 0 ; }", tokenizeAndStringify("void f() { ab: **b=0; }"));
        //with '&' operator
        ASSERT_EQUALS("void f ( ) { ab : ; & b = 0 ; }", tokenizeAndStringify("void f() { ab: &b=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; & ( b . x ) = 0 ; }", tokenizeAndStringify("void f() { ab: &(b->x)=0; }"));
        //with '(' parentheses
        ASSERT_EQUALS("void f ( ) { ab : ; * ( * b ) . x = 0 ; }", tokenizeAndStringify("void f() { ab: *(* b)->x=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; ( * * b ) . x = 0 ; }", tokenizeAndStringify("void f() { ab: (** b).x=0; }"));
        ASSERT_EQUALS("void f ( ) { ab : ; & ( * b . x ) = 0 ; }", tokenizeAndStringify("void f() { ab: &(*b.x)=0; }"));
        //with '{' parentheses
        ASSERT_EQUALS("void f ( ) { ab : ; { b = 0 ; } }", tokenizeAndStringify("void f() { ab: {b=0;} }"));
        ASSERT_EQUALS("void f ( ) { ab : ; { * b = 0 ; } }", tokenizeAndStringify("void f() { ab: { *b=0;} }"));
        ASSERT_EQUALS("void f ( ) { ab : ; { & b = 0 ; } }", tokenizeAndStringify("void f() { ab: { &b=0;} }"));
        ASSERT_EQUALS("void f ( ) { ab : ; { & ( * b . x ) = 0 ; } }", tokenizeAndStringify("void f() { ab: {&(*b.x)=0;} }"));
        //with unhandled MACRO() code
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void f() { MACRO(ab: b=0;, foo)}"), UNKNOWN_MACRO);
        ASSERT_EQUALS("void f ( ) { MACRO ( bar , ab : { & ( * b . x ) = 0 ; } ) }", tokenizeAndStringify("void f() { MACRO(bar, ab: {&(*b.x)=0;})}"));

        ignore_errout();
    }

    void simplifyInitVar() {
        {
            const char code[] = "int i ; int p(0);";
            ASSERT_EQUALS("int i ; int p ( 0 ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int i; int *p(0);";
            ASSERT_EQUALS("int i ; int * p ( 0 ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int p(0);";
            ASSERT_EQUALS("int p ( 0 ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int *p(0);";
            ASSERT_EQUALS("int * p ( 0 ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int i ; int p(i);";
            ASSERT_EQUALS("int i ; int p ( i ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int i; int *p(&i);";
            ASSERT_EQUALS("int i ; int * p ( & i ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int i; void *p(&i);";
            ASSERT_EQUALS("int i ; void * p ( & i ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S *p(&s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S * p ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "struct S { }; S s; S *p(&s);";
            ASSERT_EQUALS("struct S { } ; S s ; S * p ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "union S { int i; float f; }; union S s; union S *p(&s);";
            ASSERT_EQUALS("union S { int i ; float f ; } ; union S s ; union S * p ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "union S { int i; float f; }; S s; S *p(&s);";
            ASSERT_EQUALS("union S { int i ; float f ; } ; S s ; S * p ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "class C { }; class C c; class C *p(&c);";
            ASSERT_EQUALS("class C { } ; class C c ; class C * p ( & c ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "class C { }; C c; C *p(&c);";
            ASSERT_EQUALS("class C { } ; C c ; C * p ( & c ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S s1(s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S s1 ( s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "struct S { }; S s; S s1(s);";
            ASSERT_EQUALS("struct S { } ; S s ; S s1 ( s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "struct S { }; struct S s; struct S s1(&s);";
            ASSERT_EQUALS("struct S { } ; struct S s ; struct S s1 ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "struct S { }; S s; S s1(&s);";
            ASSERT_EQUALS("struct S { } ; S s ; S s1 ( & s ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "class S { int function(); };";
            ASSERT_EQUALS("class S { int function ( ) ; } ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "class S { int function(void); };";
            ASSERT_EQUALS("class S { int function ( ) ; } ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "class S { int function(int); };";
            ASSERT_EQUALS("class S { int function ( int ) ; } ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int function(void);";
            ASSERT_EQUALS("int function ( ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int function(int);";
            ASSERT_EQUALS("int function ( int ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "extern int function(void);";
            ASSERT_EQUALS("extern int function ( ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int function1(void); int function2(void);";
            ASSERT_EQUALS("int function1 ( ) ; int function2 ( ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int function(A);";
            // We can't tell if this a function prototype or a variable without knowing
            // what A is. Since A is undefined, just leave it alone.
            ASSERT_EQUALS("int function ( A ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int i; int function(A);";
            ASSERT_EQUALS("int i ; int function ( A ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "class A { } ; int foo(A);";
            ASSERT_EQUALS("class A { } ; int foo ( A ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "class A { } ; A a; int foo(a);";
            ASSERT_EQUALS("class A { } ; A a ; int foo ( a ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "int x(f());";
            ASSERT_EQUALS("int x ( f ( ) ) ;", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            const char code[] = "{ return doSomething(X), 0; }";
            ASSERT_EQUALS("{ return doSomething ( X ) , 0 ; }", tokenizeAndStringify(code));
            ASSERT_EQUALS("", errout_str());
        }

        {
            // 13893
            const char code[] = "namespace test { bool foo; }";
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT(tokenizer.tokenize(code));
            ASSERT(!tokenizer.tokens()->tokAt(2)->isInitBracket());
        }
    }

    void simplifyInitVar2() {
        // ticket #5131 - unsigned
        const char code[] = "void f() {\n"
                            "    unsigned int a(0),b(0);\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "unsigned int a ( 0 ) ; unsigned int b ( 0 ) ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyInitVar3() {
        const char code[] = "void f() {\n"
                            "    int *a(0),b(0);\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "int * a ( 0 ) ; int b ( 0 ) ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyInitVar4() {
        const char code[] = "void f() {\n"
                            "    uint32_t x{0};\n"
                            "}";
        ASSERT_EQUALS("void f ( ) {\n"
                      "uint32_t x { 0 } ;\n"
                      "}", tokenizeAndStringify(code));
    }

    void simplifyInitVar5() { // #14218
        const char code[] = "int c[1]{}, b;";
        ASSERT_EQUALS("int c [ 1 ] { } ; int b ;", tokenizeAndStringify(code));
    }

    void bitfields1() {
        const char code1[] = "struct A { bool x : 1; };";
        ASSERT_EQUALS("struct A { bool x ; } ;", tokenizeAndStringify(code1));

        const char code2[] = "struct A { char x : 3; };";
        ASSERT_EQUALS("struct A { char x ; } ;", tokenizeAndStringify(code2));

        const char code3[] = "struct A { short x : 3; };";
        ASSERT_EQUALS("struct A { short x ; } ;", tokenizeAndStringify(code3));

        const char code4[] = "struct A { int x : 3; };";
        ASSERT_EQUALS("struct A { int x ; } ;", tokenizeAndStringify(code4));

        const char code5[] = "struct A { long x : 3; };";
        ASSERT_EQUALS("struct A { long x ; } ;", tokenizeAndStringify(code5));

        const char code6[] = "struct A { __int8 x : 3; };";
        ASSERT_EQUALS("struct A { char x ; } ;", tokenizeAndStringify(code6, settings_win32a));

        const char code7[] = "struct A { __int16 x : 3; };";
        ASSERT_EQUALS("struct A { short x ; } ;", tokenizeAndStringify(code7,settings_win32a));

        const char code8[] = "struct A { __int32 x : 3; };";
        ASSERT_EQUALS("struct A { int x ; } ;", tokenizeAndStringify(code8, settings_win32a));

        const char code9[] = "struct A { __int64 x : 3; };";
        ASSERT_EQUALS("struct A { long long x ; } ;", tokenizeAndStringify(code9, settings_win32a));

        const char code10[] = "struct A { unsigned char x : 3; };";
        ASSERT_EQUALS("struct A { unsigned char x ; } ;", tokenizeAndStringify(code10));

        const char code11[] = "struct A { unsigned short x : 3; };";
        ASSERT_EQUALS("struct A { unsigned short x ; } ;", tokenizeAndStringify(code11));

        const char code12[] = "struct A { unsigned int x : 3; };";
        ASSERT_EQUALS("struct A { unsigned int x ; } ;", tokenizeAndStringify(code12));

        const char code13[] = "struct A { unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { unsigned long x ; } ;", tokenizeAndStringify(code13));

        const char code14[] = "struct A { unsigned __int8 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned char x ; } ;", tokenizeAndStringify(code14, settings_win32a));

        const char code15[] = "struct A { unsigned __int16 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned short x ; } ;", tokenizeAndStringify(code15, settings_win32a));

        const char code16[] = "struct A { unsigned __int32 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned int x ; } ;", tokenizeAndStringify(code16, settings_win32a));

        const char code17[] = "struct A { unsigned __int64 x : 3; };";
        ASSERT_EQUALS("struct A { unsigned long long x ; } ;", tokenizeAndStringify(code17, settings_win32a));

        const char code18[] = "struct A { signed char x : 3; };";
        ASSERT_EQUALS("struct A { signed char x ; } ;", tokenizeAndStringify(code18));

        const char code19[] = "struct A { signed short x : 3; };";
        ASSERT_EQUALS("struct A { signed short x ; } ;", tokenizeAndStringify(code19));

        const char code20[] = "struct A { signed int x : 3; };";
        ASSERT_EQUALS("struct A { signed int x ; } ;", tokenizeAndStringify(code20));

        const char code21[] = "struct A { signed long x : 3; };";
        ASSERT_EQUALS("struct A { signed long x ; } ;", tokenizeAndStringify(code21, settings_windows));

        const char code22[] = "struct A { signed __int8 x : 3; };";
        ASSERT_EQUALS("struct A { signed char x ; } ;", tokenizeAndStringify(code22, settings_win32a));

        const char code23[] = "struct A { signed __int16 x : 3; };";
        ASSERT_EQUALS("struct A { signed short x ; } ;", tokenizeAndStringify(code23, settings_win32a));

        const char code24[] = "struct A { signed __int32 x : 3; };";
        ASSERT_EQUALS("struct A { signed int x ; } ;", tokenizeAndStringify(code24, settings_win32a));

        const char code25[] = "struct A { signed __int64 x : 3; };";
        ASSERT_EQUALS("struct A { signed long long x ; } ;", tokenizeAndStringify(code25, settings_win32a));
    }

    void bitfields2() {
        const char code1[] = "struct A { public: int x : 3; };";
        ASSERT_EQUALS("struct A { public: int x ; } ;", tokenizeAndStringify(code1));

        const char code2[] = "struct A { public: unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { public: unsigned long x ; } ;", tokenizeAndStringify(code2));

        const char code3[] = "struct A { protected: int x : 3; };";
        ASSERT_EQUALS("struct A { protected: int x ; } ;", tokenizeAndStringify(code3));

        const char code4[] = "struct A { protected: unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { protected: unsigned long x ; } ;", tokenizeAndStringify(code4));

        const char code5[] = "struct A { private: int x : 3; };";
        ASSERT_EQUALS("struct A { private: int x ; } ;", tokenizeAndStringify(code5));

        const char code6[] = "struct A { private: unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { private: unsigned long x ; } ;", tokenizeAndStringify(code6));
    }

    void bitfields3() {
        const char code1[] = "struct A { const int x : 3; };";
        ASSERT_EQUALS("struct A { const int x ; } ;", tokenizeAndStringify(code1));

        const char code2[] = "struct A { const unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { const unsigned long x ; } ;", tokenizeAndStringify(code2));

        const char code3[] = "struct A { public: const int x : 3; };";
        ASSERT_EQUALS("struct A { public: const int x ; } ;", tokenizeAndStringify(code3));

        const char code4[] = "struct A { public: const unsigned long x : 3; };";
        ASSERT_EQUALS("struct A { public: const unsigned long x ; } ;", tokenizeAndStringify(code4));
    }

    void bitfields4() { // ticket #1956
        const char code1[] = "struct A { CHAR x : 3; };";
        ASSERT_EQUALS("struct A { CHAR x ; } ;", tokenizeAndStringify(code1));

        const char code2[] = "struct A { UCHAR x : 3; };";
        ASSERT_EQUALS("struct A { UCHAR x ; } ;", tokenizeAndStringify(code2));

        const char code3[] = "struct A { BYTE x : 3; };";
        ASSERT_EQUALS("struct A { BYTE x ; } ;", tokenizeAndStringify(code3));

        const char code4[] = "struct A { WORD x : 3; };";
        ASSERT_EQUALS("struct A { WORD x ; } ;", tokenizeAndStringify(code4));

        const char code5[] = "struct A { DWORD x : 3; };";
        ASSERT_EQUALS("struct A { DWORD x ; } ;", tokenizeAndStringify(code5));

        const char code6[] = "struct A { LONG x : 3; };";
        ASSERT_EQUALS("struct A { LONG x ; } ;", tokenizeAndStringify(code6));

        const char code7[] = "struct A { UINT8 x : 3; };";
        ASSERT_EQUALS("struct A { UINT8 x ; } ;", tokenizeAndStringify(code7));

        const char code8[] = "struct A { UINT16 x : 3; };";
        ASSERT_EQUALS("struct A { UINT16 x ; } ;", tokenizeAndStringify(code8));

        const char code9[] = "struct A { UINT32 x : 3; };";
        ASSERT_EQUALS("struct A { UINT32 x ; } ;", tokenizeAndStringify(code9));

        const char code10[] = "struct A { UINT64 x : 3; };";
        ASSERT_EQUALS("struct A { UINT64 x ; } ;", tokenizeAndStringify(code10));
    }

    void bitfields5() { // ticket #1956
        const char code1[] = "struct RGB { unsigned int r : 3, g : 3, b : 2; };";
        ASSERT_EQUALS("struct RGB { unsigned int r ; unsigned int g ; unsigned int b ; } ;", tokenizeAndStringify(code1));

        const char code2[] = "struct A { int a : 3; int : 3; int c : 3; };";
        ASSERT_EQUALS("struct A { int a ; int anonymous@0 ; int c ; } ;", tokenizeAndStringify(code2));

        const char code3[] = "struct A { virtual void f() {} int f1 : 1; };";
        ASSERT_EQUALS("struct A { virtual void f ( ) { } int f1 ; } ;", tokenizeAndStringify(code3));
    }

    void bitfields6() { // ticket #2595
        const char code1[] = "struct A { bool b : true; };";
        ASSERT_EQUALS("struct A { bool b ; } ;", tokenizeAndStringify(code1));

        const char code2[] = "struct A { bool b : true, c : true; };";
        ASSERT_EQUALS("struct A { bool b ; bool c ; } ;", tokenizeAndStringify(code2));

        const char code3[] = "struct A { bool : true; };";
        ASSERT_EQUALS("struct A { bool anonymous@0 ; } ;", tokenizeAndStringify(code3));
    }

    void bitfields7() { // ticket #1987
        const char code[] = "typedef struct Descriptor {"
                            "    unsigned element_size: 8* sizeof( unsigned );"
                            "} Descriptor;";
        const char expected[] = "struct Descriptor { "
                                "unsigned int element_size ; "
                                "} ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());
    }

    void bitfields8() {
        const char code[] = "struct A;"
                            "class B : virtual public C"
                            "{"
                            "    int f();"
                            "};";
        const char expected[] = "struct A ; "
                                "class B : virtual public C "
                                "{ "
                                "int f ( ) ; "
                                "} ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());
    }

    void bitfields9() { // ticket #2706
        const char code[] = "void f() {\n"
                            "    goto half;\n"
                            "half:\n"
                            "    {\n"
                            "        ;\n"
                            "    }\n"
                            "};";
        (void)tokenizeAndStringify(code);
        ASSERT_EQUALS("", errout_str());
    }

    void bitfields10() { // ticket #2737
        const char code[] = "{}"
                            "MACRO "
                            "default: { }"
                            ";";
        ASSERT_EQUALS("{ } MACRO default : { } ;", tokenizeAndStringify(code));
    }

    void bitfields12() { // ticket #3485 (segmentation fault)
        const char code[] = "{a:1;};\n";
        ASSERT_EQUALS("{ a anonymous@0 ; } ;", tokenizeAndStringify(code));
    }

    void bitfields13() { // ticket #3502 (segmentation fault)
        ASSERT_NO_THROW(tokenizeAndStringify("struct{x y:};\n"));
    }

    void bitfields15() { // #7747 - enum Foo {A,B}:4;
        ASSERT_EQUALS("struct AB {\n"
                      "enum Foo { A , B } ; enum Foo Anonymous ;\n"
                      "} ;",
                      tokenizeAndStringify("struct AB {\n"
                                           "  enum Foo {A,B} : 4;\n"
                                           "};"));
        ASSERT_EQUALS("struct AB {\n"
                      "enum Foo { A , B } ; enum Foo foo ;\n"
                      "} ;",
                      tokenizeAndStringify("struct AB {\n"
                                           "  enum Foo {A,B} foo : 4;\n"
                                           "};"));

        ASSERT_EQUALS("struct S {\n" // #14324
                      "enum E : int { E0 , E1 } ; enum E e ;\n"
                      "} ;",
                      tokenizeAndStringify("struct S {\n"
                                           "    enum E : int { E0, E1 } e : 2;\n"
                                           "};\n"));

        ASSERT_EQUALS("struct S {\n"
                      "enum class E : std :: uint8_t { E0 , E1 } ; enum E e ;\n"
                      "} ;",
                      tokenizeAndStringify("struct S {\n"
                                           "    enum class E : std::uint8_t { E0, E1 } e : 2;\n"
                                           "};\n"));
    }

    void bitfields16() {
        const char code[] = "struct A { unsigned int x : 1; };";

        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));
        const Token *x = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT_EQUALS(1, x->bits());
    }

    void bitfields17() {
        const char code[] = "struct S {\n" // #13722
                            "    volatile uint32_t a : 10;\n"
                            "    volatile uint32_t   : 6;\n"
                            "    volatile uint32_t b : 10;\n"
                            "    volatile uint32_t   : 6;\n"
                            "};\n";
        const char expected[] = "struct S {\n"
                                "volatile uint32_t a ;\n"
                                "volatile uint32_t anonymous@0 ;\n"
                                "volatile uint32_t b ;\n"
                                "volatile uint32_t anonymous@1 ;\n"
                                "} ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));

        const char code2[] = "struct S {\n"
                             "    const volatile uint32_t a : 10;\n"
                             "    const volatile uint32_t   : 6;\n"
                             "};\n";
        const char expected2[] = "struct S {\n"
                                 "const volatile uint32_t a ;\n"
                                 "const volatile uint32_t anonymous@0 ;\n"
                                 "} ;";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));
    }

    void bitfields18() {
        const char code[] = "struct S { unsigned int a : 100000; };";
        (void) tokenizeAndStringify(code);
        ASSERT_EQUALS("[test.cpp:1:29]: (warning) Bit-field size exceeds max number of bits 32767 [tooLargeBitField]\n", errout_str());
    }

    void bitfields19() {
        const char code[] = "struct S { volatile std::uint32_t a : 10; };";
        ASSERT_EQUALS("struct S { volatile std :: uint32_t a ; } ;", tokenizeAndStringify(code));
    }

    void bitfields20() {
        const char code[] = "struct S { volatile ::uint32_t a : 10; };";
        ASSERT_EQUALS("struct S { volatile :: uint32_t a ; } ;", tokenizeAndStringify(code));
    }

    void bitfields21() {
        const char code[] = "struct S { uint32_t a : 1, b : 1; };";
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));
        const Token *a = Token::findsimplematch(tokenizer.tokens(), "a");
        ASSERT_EQUALS(1, a->bits());
        const Token *b = Token::findsimplematch(tokenizer.tokens(), "b");
        ASSERT_EQUALS(1, b->bits());
    }

    void simplifyNamespaceStd() {
        const char *expected;

        {
            const char code[] = "map<foo, bar> m;"; // namespace std is not used
            ASSERT_EQUALS("map < foo , bar > m ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "map<foo, bar> m;";
            ASSERT_EQUALS("std :: map < foo , bar > m ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "string s;";
            ASSERT_EQUALS("std :: string s ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "void foo() {swap(a, b); }";
            ASSERT_EQUALS("void foo ( ) { std :: swap ( a , b ) ; }", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "void search() {}";
            ASSERT_EQUALS("void search ( ) { }", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "void search();\n"
                                "void dostuff() { search(); }";
            ASSERT_EQUALS("void search ( ) ;\nvoid dostuff ( ) { search ( ) ; }", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "void foo() {map(a, b); }"; // That's obviously not std::map<>
            ASSERT_EQUALS("void foo ( ) { map ( a , b ) ; }", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "string<wchar_t> s;"; // That's obviously not std::string
            TODO_ASSERT_EQUALS("string < wchar_t > s ;", "std :: string < wchar_t > s ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "swap s;"; // That's obviously not std::swap
            ASSERT_EQUALS("swap s ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "std::string s;";
            ASSERT_EQUALS("std :: string s ;", tokenizeAndStringify(code));
        }

        { // #4042 (Do not add 'std ::' to variables)
            const char code[] = "using namespace std;\n"
                                "const char * string = \"Hi\";";
            ASSERT_EQUALS("const char * string ; string = \"Hi\" ;", tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "string f(const char * string) {\n"
                                "    cout << string << endl;\n"
                                "    return string;\n"
                                "}";
            expected = "std :: string f ( const char * string ) {\n"
                       "std :: cout << string << std :: endl ;\n"
                       "return string ;\n"
                       "}";
            TODO_ASSERT_EQUALS(expected,
                               "std :: string f ( const char * string ) {\n"
                               "cout << string << endl ;\n"
                               "return string ;\n"
                               "}",
                               tokenizeAndStringify(code));
            ASSERT_EQUALS(
                "[test.cpp:3:5]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable cout [valueFlowBailoutIncompleteVar]\n",
                errout_str());
        }

        {
            const char code[] = "using namespace std;\n"
                                "void f() {\n"
                                "  try { }\n"
                                "  catch(std::exception &exception) { }\n"
                                "}";
            expected = "void f ( ) {\n"
                       "try { }\n"
                       "catch ( std :: exception & exception ) { }\n"
                       "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        { // #5773 (Don't prepend 'std ::' to function definitions)
            const char code[] = "using namespace std;\n"
                                "class C {\n"
                                "    void search() {}\n"
                                "    void search() const {}\n"
                                "    void search() THROW_MACRO {}\n"
                                "};";
            expected = "class C {\n"
                       "void search ( ) { }\n"
                       "void search ( ) const { }\n"
                       "void search ( ) { }\n"
                       "} ;";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        // Ticket #8091
        ASSERT_EQUALS("enum Anonymous0 { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum { string };"));
        ASSERT_EQUALS("enum Type { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum Type { string } ;"));
        ASSERT_EQUALS("enum class Type { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum class Type { string } ;"));
        ASSERT_EQUALS("enum struct Type { string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum struct Type { string } ;"));
        ASSERT_EQUALS("enum struct Type : int { f = 0 , string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "enum struct Type : int { f = 0 , string } ;"));
        ASSERT_EQUALS("enum Type { a , b } ; void foo ( enum Type , std :: string ) { }",
                      tokenizeAndStringify("using namespace std; "
                                           "enum Type { a , b } ; void foo ( enum Type , string) {}"));
        ASSERT_EQUALS("struct T { } ; enum struct Type : int { f = 0 , string } ;",
                      tokenizeAndStringify("using namespace std; "
                                           "struct T { typedef int type; } ; "
                                           "enum struct Type : T :: type { f = 0 , string } ;"));
        // Handle garbage enum code "well"
        ASSERT_EQUALS("enum E : int ; void foo ( ) { std :: string s ; }",
                      tokenizeAndStringify("using namespace std; enum E : int ; void foo ( ) { string s ; }"));

        ASSERT_NO_THROW(tokenizeAndStringify("NS_BEGIN(IMAGEIO_2D_DICOM) using namespace std; NS_END")); // #11045

        {
            const char code[] = "using namespace std;\n"
                                "void f(const unique_ptr<int>& p) {\n"
                                "    if (!p)\n"
                                "        throw runtime_error(\"abc\");\n"
                                "}";
            expected = "void f ( const std :: unique_ptr < int > & p ) {\n"
                       "if ( ! p ) {\n"
                       "throw std :: runtime_error ( \"abc\" ) ; }\n"
                       "}";
            TODO_ASSERT_EQUALS(expected,
                               "void f ( const std :: unique_ptr < int > & p ) {\n"
                               "if ( ! p ) {\n"
                               "throw runtime_error ( \"abc\" ) ; }\n"
                               "}",
                               tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n" // #8454
                                "void f() { string str = to_string(1); }\n";
            expected = "void f ( ) { std :: string str ; str = std :: to_string ( 1 ) ; }";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }

        {
            const char code[] = "using namespace std;\n"
                                "vector<int>&& f(vector<int>& v) {\n"
                                "    v.push_back(1);\n"
                                "    return move(v);\n"
                                "}\n";
            expected = "std :: vector < int > && f ( std :: vector < int > & v ) {\n"
                       "v . push_back ( 1 ) ;\n"
                       "return std :: move ( v ) ;\n"
                       "}";
            ASSERT_EQUALS(expected, tokenizeAndStringify(code));
        }
    }

    void microsoftMemory() {
        const char code1a[] = "void foo() { int a[10], b[10]; CopyMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcpy ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code1a,settings2_win32a));

        const char code1b[] = "void foo() { int a[10], b[10]; RtlCopyMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcpy ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code1b,settings2_win32a));

        const char code1c[] = "void foo() { int a[10], b[10]; RtlCopyBytes(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcpy ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code1c,settings2_win32a));

        const char code2a[] = "void foo() { int a[10]; FillMemory(a, sizeof(a), 255); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 255 , sizeof ( a ) ) ; }", tokenizeAndStringify(code2a,settings2_win32a));
        const char code2b[] = "void foo() { int a[10]; RtlFillMemory(a, sizeof(a), 255); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 255 , sizeof ( a ) ) ; }", tokenizeAndStringify(code2b,settings2_win32a));
        const char code2c[] = "void foo() { int a[10]; RtlFillBytes(a, sizeof(a), 255); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 255 , sizeof ( a ) ) ; }", tokenizeAndStringify(code2c,settings2_win32a));

        const char code3a[] = "void foo() { int a[10], b[10]; MoveMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memmove ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code3a,settings2_win32a));
        const char code3b[] = "void foo() { int a[10], b[10]; RtlMoveMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memmove ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code3b,settings2_win32a));

        const char code4a[] = "void foo() { int a[10]; ZeroMemory(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4a,settings2_win32a));
        const char code4b[] = "void foo() { int a[10]; RtlZeroMemory(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4b,settings2_win32a));
        const char code4c[] = "void foo() { int a[10]; RtlZeroBytes(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4c,settings2_win32a));
        const char code4d[] = "void foo() { int a[10]; RtlSecureZeroMemory(a, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; memset ( a , 0 , sizeof ( a ) ) ; }", tokenizeAndStringify(code4d,settings2_win32a));

        const char code5[] = "void foo() { int a[10], b[10]; RtlCompareMemory(a, b, sizeof(a)); }";
        ASSERT_EQUALS("void foo ( ) { int a [ 10 ] ; int b [ 10 ] ; memcmp ( a , b , sizeof ( a ) ) ; }", tokenizeAndStringify(code5,settings2_win32a));

        const char code6[] = "void foo() { ZeroMemory(f(1, g(a, b)), h(i, j(0, 1))); }";
        ASSERT_EQUALS("void foo ( ) { memset ( f ( 1 , g ( a , b ) ) , 0 , h ( i , j ( 0 , 1 ) ) ) ; }", tokenizeAndStringify(code6,settings2_win32a));

        const char code7[] = "void foo() { FillMemory(f(1, g(a, b)), h(i, j(0, 1)), 255); }";
        ASSERT_EQUALS("void foo ( ) { memset ( f ( 1 , g ( a , b ) ) , 255 , h ( i , j ( 0 , 1 ) ) ) ; }", tokenizeAndStringify(code7,settings2_win32a));
    }

    void microsoftString() {
        const char code1a[] = "void foo() { _tprintf (_T(\"test\") _T(\"1\")); }";
        ASSERT_EQUALS("void foo ( ) { printf ( \"test1\" ) ; }", tokenizeAndStringify(code1a, settings2_win32a));
        const char code1b[] = "void foo() { _tprintf (_TEXT(\"test\") _TEXT(\"2\")); }";
        ASSERT_EQUALS("void foo ( ) { printf ( \"test2\" ) ; }", tokenizeAndStringify(code1b, settings2_win32a));
        const char code1c[] = "void foo() { _tprintf (TEXT(\"test\") TEXT(\"3\")); }";
        ASSERT_EQUALS("void foo ( ) { printf ( \"test3\" ) ; }", tokenizeAndStringify(code1c, settings2_win32a));

        const char code2a[] = "void foo() { _tprintf (_T(\"test\") _T(\"1\")); }";
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test1\" ) ; }", tokenizeAndStringify(code2a, settings2_win32w));
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test1\" ) ; }", tokenizeAndStringify(code2a, settings2_win64));
        const char code2b[] = "void foo() { _tprintf (_TEXT(\"test\") _TEXT(\"2\")); }";
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test2\" ) ; }", tokenizeAndStringify(code2b, settings2_win32w));
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test2\" ) ; }", tokenizeAndStringify(code2b, settings2_win64));
        const char code2c[] = "void foo() { _tprintf (TEXT(\"test\") TEXT(\"3\")); }";
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test3\" ) ; }", tokenizeAndStringify(code2c, settings2_win32w));
        ASSERT_EQUALS("void foo ( ) { wprintf ( L\"test3\" ) ; }", tokenizeAndStringify(code2c, settings2_win64));
    }

    void borland() {
        // __closure
        ASSERT_EQUALS("int ( * a ) ( ) ;",  // TODO VarId
                      tokenizeAndStringify("int (__closure *a)();", settings2_win32a));

        // __property
        ASSERT_EQUALS("class Fred { ; __property ; } ;",
                      tokenizeAndStringify("class Fred { __property int x = { } };", settings2_win32a));
    }

    void simplifySQL() {
        // Oracle PRO*C extensions for inline SQL. Just replace the SQL with "asm()" to fix wrong error messages
        // ticket: #1959
        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL SELECT A FROM B\"\" ) ;", tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL SELECT A FROM B;"));
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL"), SYNTAX);

        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1 ( A ) ; END ; END - __CPPCHECK_EMBEDDED_SQL_EXEC__\"\" ) ; asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT\"\" ) ;",
                      tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1(A); END; END-__CPPCHECK_EMBEDDED_SQL_EXEC__; __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT;"));
        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL UPDATE A SET B = C\"\" ) ; asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT\"\" ) ;",
                      tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL UPDATE A SET B = C; __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT;"));
        ASSERT_EQUALS("asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT\"\" ) ; asm ( \"\"__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1 ( A ) ; END ; END - __CPPCHECK_EMBEDDED_SQL_EXEC__\"\" ) ;",
                      tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL COMMIT; __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL EXECUTE BEGIN Proc1(A); END; END-__CPPCHECK_EMBEDDED_SQL_EXEC__;"));

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("int f(){ __CPPCHECK_EMBEDDED_SQL_EXEC__ SQL } int a;"), SYNTAX);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL int f(){"), SYNTAX);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL END-__CPPCHECK_EMBEDDED_SQL_EXEC__ int a;"), SYNTAX);
        ASSERT_NO_THROW(tokenizeAndStringify("__CPPCHECK_EMBEDDED_SQL_EXEC__ SQL UPDATE A SET B = :&b->b1, C = :c::c1;"));
    }

    void simplifyCAlternativeTokens() {
        ASSERT_EQUALS("void or ( ) ;", tokenizeAndStringify("void or(void);", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( a && b ) { ; } }", tokenizeAndStringify("void f() { if (a and b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( a && b ) { ; } }", tokenizeAndStringify("void f() { if (a and b); }"));
        ASSERT_EQUALS("void f ( ) { if ( a || b ) { ; } }", tokenizeAndStringify("void f() { if (a or b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( a || b ) { ; } }", tokenizeAndStringify("void f() { if (a or b); }"));
        ASSERT_EQUALS("void f ( ) { if ( a & b ) { ; } }", tokenizeAndStringify("void f() { if (a bitand b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( a & b ) { ; } }", tokenizeAndStringify("void f() { if (a bitand b); }"));
        ASSERT_EQUALS("void f ( ) { if ( a | b ) { ; } }", tokenizeAndStringify("void f() { if (a bitor b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( a | b ) { ; } }", tokenizeAndStringify("void f() { if (a bitor b); }"));
        ASSERT_EQUALS("void f ( ) { if ( a ^ b ) { ; } }", tokenizeAndStringify("void f() { if (a xor b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( a ^ b ) { ; } }", tokenizeAndStringify("void f() { if (a xor b); }"));
        ASSERT_EQUALS("void f ( ) { if ( ~ b ) { ; } }", tokenizeAndStringify("void f() { if (compl b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( ~ b ) { ; } }", tokenizeAndStringify("void f() { if (compl b); }"));
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { ; } }", tokenizeAndStringify("void f() { if (not b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { ; } }", tokenizeAndStringify("void f() { if (not b); }"));
        ASSERT_EQUALS("void f ( ) const { if ( ! b ) { ; } }", tokenizeAndStringify("void f() const { if (not b); }"));
        ASSERT_EQUALS("void f ( ) { if ( a != b ) { ; } }", tokenizeAndStringify("void f() { if (a not_eq b); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( a != b ) { ; } }", tokenizeAndStringify("void f() { if (a not_eq b); }"));
        // #6201
        ASSERT_EQUALS("void f ( ) { if ( ! c || ! memcmp ( a , b , s ) ) { ; } }", tokenizeAndStringify("void f() { if (!c or !memcmp(a, b, s)); }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( ! c || ! memcmp ( a , b , s ) ) { ; } }", tokenizeAndStringify("void f() { if (!c or !memcmp(a, b, s)); }"));
        // #6029
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { } }", tokenizeAndStringify("void f() { if (not b){} }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( ! b ) { } }", tokenizeAndStringify("void f() { if (not b){} }"));
        // #6207
        ASSERT_EQUALS("void f ( ) { if ( not = x ) { } }", tokenizeAndStringify("void f() { if (not=x){} }", dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS("void f ( ) { if ( not = x ) { } }", tokenizeAndStringify("void f() { if (not=x){} }"));
        // #8029
        ASSERT_EQUALS("void f ( struct S * s ) { x = s . and + 1 ; }", tokenizeAndStringify("void f(struct S *s) { x = s->and + 1; }", dinit(TokenizeOptions, $.cpp = false)));
        // #8745
        ASSERT_EQUALS("void f ( ) { if ( x ) { or = 0 ; } }", tokenizeAndStringify("void f() { if (x) or = 0; }"));
        // #9324
        ASSERT_EQUALS("void f ( const char * str ) { while ( * str == '!' || * str == '[' ) { } }",
                      tokenizeAndStringify("void f(const char *str) { while (*str=='!' or *str=='['){} }"));
        // #9920
        ASSERT_EQUALS("result = ch != s . end ( ) && * ch == ':' ;", tokenizeAndStringify("result = ch != s.end() and *ch == ':';", dinit(TokenizeOptions, $.cpp = false)));

        // #8975
        ASSERT_EQUALS("void foo ( ) {\n"
                      "char * or ;\n"
                      "while ( ( * or != 0 ) && ( * or != '|' ) ) { or ++ ; }\n"
                      "}",
                      tokenizeAndStringify(
                          "void foo() {\n"
                          "  char *or;\n"
                          "  while ((*or != 0) && (*or != '|')) or++;\n"
                          "}", dinit(TokenizeOptions, $.cpp = false)));
        // #10013
        ASSERT_EQUALS("void f ( ) { x = ! 123 ; }", tokenizeAndStringify("void f() { x = not 123; }"));

        { // #12476
            const char code[] = "struct S { int a, b; };"
                                "void f(struct S* compl) {"
                                "    compl->a = compl->b;"
                                "}";
            const char exp[] = "struct S { int a ; int b ; } ; void f ( struct S * compl ) { compl . a = compl . b ; }";
            ASSERT_EQUALS(exp, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        }

        //ASSERT_EQUALS("", filter_valueflow(errout_str()));
        ignore_errout();
    }

    void simplifyCompoundStatements() {
        ASSERT_EQUALS("; x = 123 ;", tokenizeAndStringify(";x=({123;});"));
        ASSERT_EQUALS("; x = y ;", tokenizeAndStringify(";x=({y;});"));
        // #13419: Do not simplify compound statements in for loop
        ASSERT_EQUALS("void foo ( int x ) { for ( ; ( { { } ; x < 1 ; } ) ; ) { ; } }",
                      tokenizeAndStringify("void foo(int x) { for (;({ {}; x<1; });); }"));
    }

    void simplifyOperatorName1() {
        // make sure C code doesn't get changed
        const char code[] = "void operator () {}"
                            "int main()"
                            "{"
                            "    operator();"
                            "}";

        const char result[] = "void operator ( ) { } "
                              "int main ( ) "
                              "{ "
                              "operator ( ) ; "
                              "}";

        ASSERT_EQUALS(result, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
    }

    void simplifyOperatorName2() {
        const char code[] = "class Fred"
                            "{"
                            "    Fred(const Fred & f) { operator = (f); }"
                            "    operator = ();"
                            "}";

        const char result[] = "class Fred "
                              "{ "
                              "Fred ( const Fred & f ) { operator= ( f ) ; } "
                              "operator= ( ) ; "
                              "}";

        ASSERT_EQUALS(result, tokenizeAndStringify(code));
    }

    void simplifyOperatorName3() {
        // #2615
        const char code[] = "void f() {"
                            "static_cast<ScToken*>(xResult.operator->())->GetMatrix();"
                            "}";
        const char result[] = "void f ( ) { static_cast < ScToken * > ( xResult . operator-> ( ) ) . GetMatrix ( ) ; }";
        ASSERT_EQUALS(result, tokenizeAndStringify(code));
    }

    void simplifyOperatorName4() {
        const char code[] = "void operator==() { }";
        const char result[] = "void operator== ( ) { }";
        ASSERT_EQUALS(result, tokenizeAndStringify(code));
    }

    void simplifyOperatorName5() {
        const char code1[] = "std::istream & operator >> (std::istream & s, Fred &f);";
        const char result1[] = "std :: istream & operator>> ( std :: istream & s , Fred & f ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1));

        const char code2[] = "std::ostream & operator << (std::ostream & s, const Fred &f);";
        const char result2[] = "std :: ostream & operator<< ( std :: ostream & s , const Fred & f ) ;";
        ASSERT_EQUALS(result2, tokenizeAndStringify(code2));
    }

    void simplifyOperatorName6() { // ticket #3195
        const char code1[] = "value_type * operator ++ (int);";
        const char result1[] = "value_type * operator++ ( int ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1));

        const char code2[] = "value_type * operator -- (int);";
        const char result2[] = "value_type * operator-- ( int ) ;";
        ASSERT_EQUALS(result2, tokenizeAndStringify(code2));
    }

    void simplifyOperatorName7() { // ticket #4619
        const char code1[] = "value_type * operator += (int);";
        const char result1[] = "value_type * operator+= ( int ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1));
    }

    void simplifyOperatorName8() { // ticket #5706
        const char code1[] = "value_type * operator += (int) noexcept ;";
        const char result1[] = "value_type * operator+= ( int ) noexcept ( true ) ;";
        ASSERT_EQUALS(result1, tokenizeAndStringify(code1));

        const char code2[] = "value_type * operator += (int) noexcept ( true ) ;";
        const char result2[] = "value_type * operator+= ( int ) noexcept ( true ) ;";
        ASSERT_EQUALS(result2, tokenizeAndStringify(code2));

        const char code3[] = "value_type * operator += (int) throw ( ) ;";
        const char result3[] = "value_type * operator+= ( int ) throw ( ) ;";
        ASSERT_EQUALS(result3, tokenizeAndStringify(code3));

        const char code4[] = "value_type * operator += (int) const noexcept ;";
        const char result4[] = "value_type * operator+= ( int ) const noexcept ( true ) ;";
        ASSERT_EQUALS(result4, tokenizeAndStringify(code4));

        const char code5[] = "value_type * operator += (int) const noexcept ( true ) ;";
        const char result5[] = "value_type * operator+= ( int ) const noexcept ( true ) ;";
        ASSERT_EQUALS(result5, tokenizeAndStringify(code5));

        const char code6[] = "value_type * operator += (int) const throw ( ) ;";
        const char result6[] = "value_type * operator+= ( int ) const throw ( ) ;";
        ASSERT_EQUALS(result6, tokenizeAndStringify(code6));

        const char code7[] = "value_type * operator += (int) const noexcept ( false ) ;";
        const char result7[] = "value_type * operator+= ( int ) const noexcept ( false ) ;";
        ASSERT_EQUALS(result7, tokenizeAndStringify(code7));

    }

    void simplifyOperatorName9() { // Ticket #5709
        const char code[] = "struct R { R operator, ( R b ) ; } ;";
        ASSERT_EQUALS(code, tokenizeAndStringify(code));
    }

    void simplifyOperatorName10() { // #8746
        const char code1[] = "using a::operator=;";
        ASSERT_EQUALS("using a :: operator= ;", tokenizeAndStringify(code1));

        const char code2[] = "{ return &Fred::operator!=; }";
        ASSERT_EQUALS("{ return & Fred :: operator!= ; }", tokenizeAndStringify(code2));
    }

    void simplifyOperatorName11() { // #8889
        const char code[] = "auto operator = (const Fred & other) -> Fred & ;";
        ASSERT_EQUALS("auto operator= ( const Fred & other ) . Fred & ;", tokenizeAndStringify(code));
        ASSERT_EQUALS("[test.cpp:1:1]: (debug) auto token with no type. [autoNoType]\n", errout_str());

        const char code1[] = "auto operator = (const Fred & other) -> Fred & { }";
        ASSERT_EQUALS("auto operator= ( const Fred & other ) . Fred & { }", tokenizeAndStringify(code1));
        ASSERT_EQUALS("[test.cpp:1:1]: (debug) auto token with no type. [autoNoType]\n", errout_str());

        const char code2[] = "template <typename T> void g(S<&T::operator+ >) {}";
        ASSERT_EQUALS("template < typename T > void g ( S < & T :: operator+ > ) { }", tokenizeAndStringify(code2));
        ASSERT_EQUALS("", errout_str());

        const char code3[] = "template <typename T> void g(S<&T::operator int>) {}";
        ASSERT_EQUALS("template < typename T > void g ( S < & T :: operatorint > ) { }", tokenizeAndStringify(code3));
        ASSERT_EQUALS("", errout_str());

        const char code4[] = "template <typename T> void g(S<&T::template operator- <double> >) {}";
        ASSERT_EQUALS("template < typename T > void g ( S < & T :: operator- < double > > ) { }", tokenizeAndStringify(code4));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyOperatorName12() { // #9110
        const char code[] = "namespace a {"
                            "template <typename b> void operator+(b);"
                            "}"
                            "using a::operator+;";
        ASSERT_EQUALS("namespace a { "
                      "template < typename b > void operator+ ( b ) ; "
                      "} "
                      "using a :: operator+ ;",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName13() { // user defined literal
        const char code[] = "unsigned long operator\"\"_numch(const char *ch, unsigned long size);";
        ASSERT_EQUALS("unsigned long operator\"\"_numch ( const char * ch , unsigned long size ) ;",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName14() { // std::complex operator "" if
        {
            const char code[] = "constexpr std::complex<float> operator\"\"if(long double __num);";
            ASSERT_EQUALS("constexpr std :: complex < float > operator\"\"if ( long double __num ) ;",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "constexpr std::complex<float> operator\"\"if(long double __num) { }";
            ASSERT_EQUALS("constexpr std :: complex < float > operator\"\"if ( long double __num ) { }",
                          tokenizeAndStringify(code));
        }
    }

    void simplifyOperatorName15() { // ticket #9468 syntaxError
        const char code[] = "template <typename> struct a;"
                            "template <typename> struct b {"
                            "  typedef char c;"
                            "  operator c();"
                            "};"
                            "template <> struct a<char> : b<char> { using b::operator char; };";
        ASSERT_EQUALS("struct a<char> ; template < typename > struct a ; "
                      "struct b<char> ; "
                      "struct a<char> : b<char> { using b :: operatorchar ; } ; struct b<char> { "
                      "operatorchar ( ) ; "
                      "} ;",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName16() { // ticket #9472
        {
            const char code[] = "class I : public A { iterator& operator++() override; };";
            ASSERT_EQUALS("class I : public A { iterator & operator++ ( ) override ; } ;",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "class I : public A { iterator& operator++() override { } };";
            ASSERT_EQUALS("class I : public A { iterator & operator++ ( ) override { } } ;",
                          tokenizeAndStringify(code));
        }
    }

    void simplifyOperatorName17() {
        {
            const char code[] = "template <class a> void b(a c, a d) { c.operator>() == d; }";
            ASSERT_EQUALS("template < class a > void b ( a c , a d ) { c . operator> ( ) == d ; }",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "template <class a> void b(a c, a d) { c.operator>() == (d + 1); }";
            ASSERT_EQUALS("template < class a > void b ( a c , a d ) { c . operator> ( ) == ( d + 1 ) ; }",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "template <class a> void b(a c, a d) { c.operator<() == d; }";
            ASSERT_EQUALS("template < class a > void b ( a c , a d ) { c . operator< ( ) == d ; }",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "template <class a> void b(a c, a d) { c.operator>() == (d + 1); }";
            ASSERT_EQUALS("template < class a > void b ( a c , a d ) { c . operator> ( ) == ( d + 1 ) ; }",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "template <class a> void b(a c, a d) { c.operator++() == d; }";
            ASSERT_EQUALS("template < class a > void b ( a c , a d ) { c . operator++ ( ) == d ; }",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "template <class a> void b(a c, a d) { c.operator++() == (d + 1); }";
            ASSERT_EQUALS("template < class a > void b ( a c , a d ) { c . operator++ ( ) == ( d + 1 ) ; }",
                          tokenizeAndStringify(code));
        }
    }

    void simplifyOperatorName18() { // global namespace
        {
            const char code[] = "struct Fred { operator std::string() const { return std::string(\"Fred\"); } };";
            ASSERT_EQUALS("struct Fred { operatorstd::string ( ) const { return std :: string ( \"Fred\" ) ; } } ;",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "struct Fred { operator ::std::string() const { return ::std::string(\"Fred\"); } };";
            ASSERT_EQUALS("struct Fred { operator::std::string ( ) const { return :: std :: string ( \"Fred\" ) ; } } ;",
                          tokenizeAndStringify(code));
        }
    }

    void simplifyOperatorName19() {
        const char code[] = "struct v {};"
                            "enum E { e };"
                            "struct s {"
                            "  operator struct v() { return v(); };"
                            "  operator enum E() { return e; }"
                            "};"
                            "void f() {"
                            "  (void)&s::operator struct v;"
                            "  (void)&s::operator enum E;"
                            "}";
        ASSERT_EQUALS("struct v { } ; "
                      "enum E { e } ; "
                      "struct s { "
                      "operatorstructv ( ) { return v ( ) ; } ; "
                      "operatorenumE ( ) { return e ; } "
                      "} ; "
                      "void f ( ) { "
                      "( void ) & s :: operatorstructv ; "
                      "( void ) & s :: operatorenumE ; "
                      "}",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName20() {
        const char code[] = "void operator \"\" _a(const char *);"
                            "namespace N {"
                            "  using ::operator \"\" _a;"
                            "  void operator \"\" _b(const char *);"
                            "}";
        ASSERT_EQUALS("void operator\"\"_a ( const char * ) ; "
                      "namespace N { "
                      "using :: operator\"\"_a ; "
                      "void operator\"\"_b ( const char * ) ; "
                      "}",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName21() {
        const char code[] = "template<char...> void operator \"\" _h() {}"
                            "template<> void operator \"\" _h<'a', 'b', 'c'>() {}"
                            "template void operator \"\" _h<'a', 'b', 'c', 'd'>();";
        ASSERT_EQUALS("void operator\"\"_h<'a','b','c'> ( ) ; "
                      "void operator\"\"_h<'a','b','c','d'> ( ) ; "
                      "void operator\"\"_h<'a','b','c'> ( ) { } "
                      "void operator\"\"_h<'a','b','c','d'> ( ) { }",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName22() {
        const char code[] = "static RSLRelOp convertOperator(const Software::ComparisonOperator& op) {"
                            "  if (op == &Software::operator==) return RSLEqual;"
                            "return RSLNotEqual;"
                            "}";
        ASSERT_EQUALS("static RSLRelOp convertOperator ( const Software :: ComparisonOperator & op ) { "
                      "if ( op == & Software :: operator== ) { return RSLEqual ; } "
                      "return RSLNotEqual ; "
                      "}",
                      tokenizeAndStringify(code));
        ASSERT_EQUALS(
            "[test.cpp:1:116]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable RSLEqual [valueFlowBailoutIncompleteVar]\n",
            filter_valueflow(errout_str()));
    }

    void simplifyOperatorName23() {
        {
            const char code[] = "double *vtkMatrix3x3::operator[](const unsigned int i) {"
                                "  VTK_LEGACY_BODY(vtkMatrix3x3::operator[], \"VTK 7.0\");"
                                "  return &(this->Element[i][0]);"
                                "}";
            ASSERT_EQUALS("double * vtkMatrix3x3 :: operator[] ( const unsigned int i ) { "
                          "VTK_LEGACY_BODY ( vtkMatrix3x3 :: operator[] , \"VTK 7.0\" ) ; "
                          "return & ( this . Element [ i ] [ 0 ] ) ; "
                          "}",
                          tokenizeAndStringify(code));
            ignore_errout();
        }
        {
            const char code[] = "double *vtkMatrix3x3::operator,(const unsigned int i) {"
                                "  VTK_LEGACY_BODY(vtkMatrix3x3::operator,, \"VTK 7.0\");"
                                "  return &(this->Element[i][0]);"
                                "}";
            ASSERT_EQUALS("double * vtkMatrix3x3 :: operator, ( const unsigned int i ) { "
                          "VTK_LEGACY_BODY ( vtkMatrix3x3 :: operator, , \"VTK 7.0\" ) ; "
                          "return & ( this . Element [ i ] [ 0 ] ) ; "
                          "}",
                          tokenizeAndStringify(code));
            ignore_errout();
        }
    }

    void simplifyOperatorName24() {
        {
            const char code[] = "void foo() { int i = a.operator++() ? a.operator--() : 0; }";
            ASSERT_EQUALS("void foo ( ) { int i ; i = a . operator++ ( ) ? a . operator-- ( ) : 0 ; }",
                          tokenizeAndStringify(code));
        }
        {
            const char code[] = "void foo() { int i = a.operator++(0) ? a.operator--(0) : 0; }";
            ASSERT_EQUALS("void foo ( ) { int i ; i = a . operator++ ( 0 ) ? a . operator-- ( 0 ) : 0 ; }",
                          tokenizeAndStringify(code));
        }
    }

    void simplifyOperatorName25() {
        const char code[] = "bool negative(const Number &num) { return num.operator std::string()[0] == '-'; }";
        ASSERT_EQUALS("bool negative ( const Number & num ) { return num . operatorstd::string ( ) [ 0 ] == '-' ; }",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName26() {
        const char code[] = "void foo() {"
                            "    x = y.operator *().z[123];"
                            "}";
        ASSERT_EQUALS("void foo ( ) { x = y . operator* ( ) . z [ 123 ] ; }",
                      tokenizeAndStringify(code));
        ignore_errout();
    }

    void simplifyOperatorName27() {
        const char code[] = "int operator \"\" i (const char *, int);\n"
                            "x = \"abc\"i;";
        ASSERT_EQUALS("int operator\"\"i ( const char * , int ) ;\n"
                      "x = operator\"\"i ( \"abc\" , 3 ) ;",
                      tokenizeAndStringify(code));
    }

    void simplifyOperatorName28() {
        const char code[] = "template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };\n"
                            "int main() { }";
        ASSERT_EQUALS("template < class ... Ts > struct overloaded : Ts ... { using Ts :: operator ( ) ... ; } ;\n"
                      "int main ( ) { }",
                      tokenizeAndStringify(code));
        ASSERT_EQUALS("[test.cpp:1:61]: (debug) simplifyOperatorName: found unsimplified operator name [debug]\n", errout_str());
    }

    void simplifyOperatorName29() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP20).build();
        ASSERT_EQUALS("auto operator<=> ( ) ;", tokenizeAndStringify("auto operator<=>();", settings));
    }

    void simplifyOperatorName30() { // #14151
        const char code[] = "struct S { int operator()() const { return 42; } };\n"
                            "int f() {\n"
                            "    S s;\n"
                            "    return int{ s.operator()() };\n"
                            "}\n";
        ASSERT_EQUALS("struct S { int operator() ( ) const { return 42 ; } } ;\n"
                      "int f ( ) {\n"
                      "S s ;\n"
                      "return int { s . operator() ( ) } ;\n"
                      "}", tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyOperatorName31() { // #6342
        const char code[] = "template <typename T>\n"
                            "struct B {\n"
                            "    typedef T A[3];\n"
                            "    operator A& () { return x_; }\n"
                            "    A x_;\n"
                            "};";
        ASSERT_EQUALS("template < typename T >\nstruct B {\n\nT ( & operatorT ( ) ) [ 3 ] { return x_ ; }\nT x_ [ 3 ] ;\n} ;", tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyOperatorName32() { // #10256
        const char code[] = "void f(int* = nullptr) {}\n";
        ASSERT_EQUALS("void f ( int * = nullptr ) { }", tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyOperatorName33() { // #10138
        const char code[] = "int (operator\"\" _ii)(unsigned long long v) { return v; }\n";
        ASSERT_EQUALS("int operator\"\"_ii ( unsigned long long v ) { return v ; }", tokenizeAndStringify(code));
        ASSERT_EQUALS("", errout_str());
    }

    void simplifyOverloadedOperators1() {
        const char code[] = "struct S { void operator()(int); };\n"
                            "\n"
                            "void foo(S x) {\n"
                            "    x(123);\n"
                            "}";
        ASSERT_EQUALS("struct S { void operator() ( int ) ; } ;\n"
                      "\n"
                      "void foo ( S x ) {\n"
                      "x . operator() ( 123 ) ;\n"
                      "}",
                      tokenizeAndStringify(code));
    }

    void simplifyOverloadedOperators2() { // #9879 - (*this)(123);
        const char code[] = "struct S {\n"
                            "  void operator()(int);\n"
                            "  void foo() { (*this)(123); }\n"
                            "};\n";
        ASSERT_EQUALS("struct S {\n"
                      "void operator() ( int ) ;\n"
                      "void foo ( ) { ( * this ) . operator() ( 123 ) ; }\n"
                      "} ;",
                      tokenizeAndStringify(code));
    }

    void simplifyOverloadedOperators3() { // #9881
        const char code[] = "struct Func { double operator()(double x) const; };\n"
                            "void foo(double, double);\n"
                            "void test() {\n"
                            "    Func max;\n"
                            "    double y = 0;\n"
                            "    foo(0, max(y));\n"
                            "}";
        ASSERT_EQUALS("struct Func { double operator() ( double x ) const ; } ;\n"
                      "void foo ( double , double ) ;\n"
                      "void test ( ) {\n"
                      "Func max ;\n"
                      "double y ; y = 0 ;\n"
                      "foo ( 0 , max . operator() ( y ) ) ;\n"
                      "}",
                      tokenizeAndStringify(code));
    }

    void simplifyNullArray() {
        ASSERT_EQUALS("* ( foo . bar [ 5 ] ) = x ;", tokenizeAndStringify("0[foo.bar[5]] = x;"));
    }

    void removeMacrosInGlobalScope() {
        // remove some unhandled macros in the global scope.
        ASSERT_EQUALS("void f ( ) { }", tokenizeAndStringify("void f() NOTHROW { }"));
        ASSERT_EQUALS("struct Foo { } ;", tokenizeAndStringify("struct __declspec(dllexport) Foo {};"));
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("ABA() namespace { int a ; }"), UNKNOWN_MACRO);

        // #3750
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("; AB(foo*) foo::foo() { }"), UNKNOWN_MACRO);

        // #4834 - syntax error
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("A(B) foo() {}"), UNKNOWN_MACRO);

        // #3855
        ASSERT_EQUALS("; class foo { }",
                      tokenizeAndStringify("; AB class foo { }"));
        ASSERT_EQUALS("; CONST struct ABC abc ;",
                      tokenizeAndStringify("; CONST struct ABC abc ;"));

        ASSERT_NO_THROW(tokenizeAndStringify("class A {\n"
                                             "  UNKNOWN_MACRO(A)\n" // <- this macro is ignored
                                             "private:\n"
                                             "  int x;\n"
                                             "};"));

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("MACRO(test) void test() { }"), UNKNOWN_MACRO); // #7931

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("BEGIN_MESSAGE_MAP(CSetProgsAdvDlg, CResizableStandAloneDialog)\n"
                                                   "    ON_BN_CLICKED(IDC_ADDTOOL, OnBnClickedAddtool)\n"
                                                   "END_MESSAGE_MAP()\n"
                                                   "\n"
                                                   "BOOL CSetProgsAdvDlg::OnInitDialog() {}"),
                              UNKNOWN_MACRO);

        ASSERT_EQUALS("struct S {\n"
                      "S ( ) : p { new ( malloc ( 4 ) ) int { } } { }\n"
                      "int * p ;\n"
                      "} ;",
                      tokenizeAndStringify("struct S {\n"
                                           "    S() : p{new (malloc(4)) int{}} {}\n"
                                           "    int* p;\n"
                                           "};\n"));
    }

    void addSemicolonAfterUnknownMacro() {
        // #6975
        ASSERT_EQUALS("void f ( ) { MACRO ( ) ; try { } }", tokenizeAndStringify("void f() { MACRO() try {} }"));
        // #9376
        ASSERT_EQUALS("MACRO ( ) ; using namespace foo ;", tokenizeAndStringify("MACRO() using namespace foo;"));
    }

    void multipleAssignment() {
        ASSERT_EQUALS("a = b = 0 ;", tokenizeAndStringify("a=b=0;"));
    }

    void platformWin() {
        const char code[] = "BOOL f;"
                            "BOOLEAN g;"
                            "BYTE h;"
                            "CHAR i;"
                            "DWORD j;"
                            "FLOAT k;"
                            "INT l;"
                            "INT32 m;"
                            "INT64 n;"
                            "LONG o;"
                            "SHORT p;"
                            "UCHAR q;"
                            "UINT r;"
                            "ULONG s;"
                            "USHORT t;"
                            "WORD u;"
                            "VOID *v;"
                            "LPBOOL w;"
                            "PBOOL x;"
                            "LPBYTE y;"
                            "PBOOLEAN z;"
                            "PBYTE A;"
                            "LPCSTR B;"
                            "PCSTR C;"
                            "LPCVOID D;"
                            "LPDWORD E;"
                            "LPINT F;"
                            "PINT G;"
                            "LPLONG H;"
                            "PLONG I;"
                            "LPSTR J;"
                            "PSTR K;"
                            "PCHAR L;"
                            "LPVOID M;"
                            "PVOID N;"
                            "BOOL _bool;"
                            "HFILE hfile;"
                            "LONG32 long32;"
                            "LCID lcid;"
                            "LCTYPE lctype;"
                            "LGRPID lgrpid;"
                            "LONG64 long64;"
                            "PUCHAR puchar;"
                            "LPCOLORREF lpcolorref;"
                            "PDWORD pdword;"
                            "PULONG pulong;"
                            "SERVICE_STATUS_HANDLE service_status_hanlde;"
                            "SC_LOCK sc_lock;"
                            "SC_HANDLE sc_handle;"
                            "HACCEL haccel;"
                            "HCONV hconv;"
                            "HCONVLIST hconvlist;"
                            "HDDEDATA hddedata;"
                            "HDESK hdesk;"
                            "HDROP hdrop;"
                            "HDWP hdwp;"
                            "HENHMETAFILE henhmetafile;"
                            "HHOOK hhook;"
                            "HKL hkl;"
                            "HMONITOR hmonitor;"
                            "HSZ hsz;"
                            "HWINSTA hwinsta;"
                            "PWCHAR pwchar;"
                            "PUSHORT pushort;"
                            "LANGID langid;"
                            "DWORD64 dword64;"
                            "ULONG64 ulong64;"
                            "LPWSTR lpcwstr;"
                            "LPCWSTR lpcwstr;"
                            "LPHANDLE lpHandle;"
                            "PCWSTR pcwStr;"
                            "PDWORDLONG pdWordLong;"
                            "PDWORD_PTR pdWordPtr;"
                            "PDWORD32 pdWord32;"
                            "PDWORD64 pdWord64;"
                            "LONGLONG ll;"
                            "USN usn;"
                            "PULONG64 puLong64;"
                            "PULONG32 puLong32;"
                            "PFLOAT ptrToFloat;";

        const char expected[] = "int f ; "
                                "unsigned char g ; "
                                "unsigned char h ; "
                                "char i ; "
                                "unsigned long j ; "
                                "float k ; "
                                "int l ; "
                                "int m ; "
                                "long long n ; "
                                "long o ; "
                                "short p ; "
                                "unsigned char q ; "
                                "unsigned int r ; "
                                "unsigned long s ; "
                                "unsigned short t ; "
                                "unsigned short u ; "
                                "void * v ; "
                                "int * w ; "
                                "int * x ; "
                                "unsigned char * y ; "
                                "unsigned char * z ; "
                                "unsigned char * A ; "
                                "const char * B ; "
                                "const char * C ; "
                                "const void * D ; "
                                "unsigned long * E ; "
                                "int * F ; "
                                "int * G ; "
                                "long * H ; "
                                "long * I ; "
                                "char * J ; "
                                "char * K ; "
                                "char * L ; "
                                "void * M ; "
                                "void * N ; "
                                "int _bool ; "
                                "int hfile ; "
                                "int long32 ; "
                                "unsigned long lcid ; "
                                "unsigned long lctype ; "
                                "unsigned long lgrpid ; "
                                "long long long64 ; "
                                "unsigned char * puchar ; "
                                "unsigned long * lpcolorref ; "
                                "unsigned long * pdword ; "
                                "unsigned long * pulong ; "
                                "void * service_status_hanlde ; "
                                "void * sc_lock ; "
                                "void * sc_handle ; "
                                "void * haccel ; "
                                "void * hconv ; "
                                "void * hconvlist ; "
                                "void * hddedata ; "
                                "void * hdesk ; "
                                "void * hdrop ; "
                                "void * hdwp ; "
                                "void * henhmetafile ; "
                                "void * hhook ; "
                                "void * hkl ; "
                                "void * hmonitor ; "
                                "void * hsz ; "
                                "void * hwinsta ; "
                                "wchar_t * pwchar ; "
                                "unsigned short * pushort ; "
                                "unsigned short langid ; "
                                "unsigned long long dword64 ; "
                                "unsigned long long ulong64 ; "
                                "wchar_t * lpcwstr ; "
                                "const wchar_t * lpcwstr ; "
                                "void * lpHandle ; "
                                "const wchar_t * pcwStr ; "
                                "long * pdWordLong ; "
                                "long * pdWordPtr ; "
                                "unsigned int * pdWord32 ; "
                                "unsigned long * pdWord64 ; "
                                "long long ll ; "
                                "long long usn ; "
                                "unsigned long long * puLong64 ; "
                                "unsigned int * puLong32 ; "
                                "float * ptrToFloat ;";

        // These types should be defined the same on all Windows platforms
        const std::string win32A = tokenizeAndStringify(code, settings_win32a);
        ASSERT_EQUALS(expected, win32A);
        ASSERT_EQUALS(win32A, tokenizeAndStringify(code, settings_win32w));
        ASSERT_EQUALS(win32A, tokenizeAndStringify(code, settings_win32a));
    }

    void platformWin32A() {
        const char code[] = "wchar_t wc;"
                            "TCHAR c;"
                            "PTSTR ptstr;"
                            "LPTSTR lptstr;"
                            "PCTSTR pctstr;"
                            "LPCTSTR lpctstr;"
                            "void foo() {"
                            "    TCHAR tc = _T(\'c\'); "
                            "    TCHAR src[10] = _T(\"123456789\");"
                            "    TCHAR dst[10];"
                            "    _tcscpy(dst, src);"
                            "    dst[0] = 0;"
                            "    _tcscat(dst, src);"
                            "    LPTSTR d = _tcsdup(src);"
                            "    _tprintf(_T(\"Hello world!\"));"
                            "    _stprintf(dst, _T(\"Hello!\"));"
                            "    _sntprintf(dst, sizeof(dst) / sizeof(TCHAR), _T(\"Hello world!\"));"
                            "    _tscanf(_T(\"%s\"), dst);"
                            "    _stscanf(dst, _T(\"%s\"), dst);"
                            "}"
                            "TBYTE tbyte;";
        const char expected[] = "wchar_t wc ; "
                                "char c ; "
                                "char * ptstr ; "
                                "char * lptstr ; "
                                "const char * pctstr ; "
                                "const char * lpctstr ; "
                                "void foo ( ) { "
                                "char tc ; tc = \'c\' ; "
                                "char src [ 10 ] = \"123456789\" ; "
                                "char dst [ 10 ] ; "
                                "strcpy ( dst , src ) ; "
                                "dst [ 0 ] = 0 ; "
                                "strcat ( dst , src ) ; "
                                "char * d ; d = strdup ( src ) ; "
                                "printf ( \"Hello world!\" ) ; "
                                "sprintf ( dst , \"Hello!\" ) ; "
                                "_snprintf ( dst , sizeof ( dst ) / sizeof ( char ) , \"Hello world!\" ) ; "
                                "scanf ( \"%s\" , dst ) ; "
                                "sscanf ( dst , \"%s\" , dst ) ; "
                                "} "
                                "unsigned char tbyte ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, settings_win32a));

        const char code2[] = "LPCTSTR f(void* p) { return LPCTSTR(p); }\n" // #11430
                             "LPCTSTR g() { return LPCTSTR{}; }";
        const char expected2[] = "const char * f ( void * p ) { return ( const char * ) ( p ) ; }\n"
                                 "const char * g ( ) { return ( const char * ) ( 0 ) ; }";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2, settings_win32a));
    }

    void platformWin32W() {
        const char code[] = "wchar_t wc;"
                            "TCHAR c;"
                            "PTSTR ptstr;"
                            "LPTSTR lptstr;"
                            "PCTSTR pctstr;"
                            "LPCTSTR lpctstr;"
                            "TBYTE tbyte;"
                            "void foo() {"
                            "    TCHAR tc = _T(\'c\');"
                            "    TCHAR src[10] = _T(\"123456789\");"
                            "    TCHAR dst[10];"
                            "    _tcscpy(dst, src);"
                            "    dst[0] = 0;"
                            "    _tcscat(dst, src);"
                            "    LPTSTR d = _tcsdup(src);"
                            "    _tprintf(_T(\"Hello world!\"));"
                            "    _stprintf(dst, _T(\"Hello!\"));"
                            "    _sntprintf(dst, sizeof(dst) / sizeof(TCHAR), _T(\"Hello world!\"));"
                            "    _tscanf(_T(\"%s\"), dst);"
                            "    _stscanf(dst, _T(\"%s\"), dst);"
                            "}";
        const char expected[] = "wchar_t wc ; "
                                "wchar_t c ; "
                                "wchar_t * ptstr ; "
                                "wchar_t * lptstr ; "
                                "const wchar_t * pctstr ; "
                                "const wchar_t * lpctstr ; "
                                "unsigned wchar_t tbyte ; "
                                "void foo ( ) { "
                                "wchar_t tc ; tc = L\'c\' ; "
                                "wchar_t src [ 10 ] = L\"123456789\" ; "
                                "wchar_t dst [ 10 ] ; "
                                "wcscpy ( dst , src ) ; "
                                "dst [ 0 ] = 0 ; "
                                "wcscat ( dst , src ) ; "
                                "wchar_t * d ; d = wcsdup ( src ) ; "
                                "wprintf ( L\"Hello world!\" ) ; "
                                "swprintf ( dst , L\"Hello!\" ) ; "
                                "_snwprintf ( dst , sizeof ( dst ) / sizeof ( wchar_t ) , L\"Hello world!\" ) ; "
                                "wscanf ( L\"%s\" , dst ) ; "
                                "swscanf ( dst , L\"%s\" , dst ) ; "
                                "}";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, settings_win32w));
    }

    void platformWin32AStringCat() { //#5150
        const char code[] = "TCHAR text[] = _T(\"123\") _T(\"456\") _T(\"789\");";
        const char expected[] = "char text [ 10 ] = \"123456789\" ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, settings_win32a));
    }

    void platformWin32WStringCat() { //#5150
        const char code[] = "TCHAR text[] = _T(\"123\") _T(\"456\") _T(\"789\");";
        const char expected[] = "wchar_t text [ 10 ] = L\"123456789\" ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, settings_win32w));
    }

    void platformWinWithNamespace() {
        const char code1[] = "UINT32 a; ::UINT32 b; foo::UINT32 c;";
        const char expected1[] = "unsigned int a ; unsigned int b ; foo :: UINT32 c ;";
        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1, settings_win32a));

        const char code2[] = "LPCVOID a; ::LPCVOID b; foo::LPCVOID c;";
        const char expected2[] = "const void * a ; const void * b ; foo :: LPCVOID c ;";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2, settings_win32w));
    }

    void isOneNumber() const {
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("1.0"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1.0"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("1.0e+0"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1L"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("1"));
        ASSERT_EQUALS(true, Tokenizer::isOneNumber("+1E+0"));

        ASSERT_EQUALS(false, Tokenizer::isOneNumber("0.0"));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber("+0.0"));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber("-0"));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber(""));
        ASSERT_EQUALS(false, Tokenizer::isOneNumber("garbage"));
    }

    void simplifyStaticConst() {
        const char code1[]     = "class foo { public: bool const static c ; }";
        const char expected1[] = "class foo { public: static const bool c ; }";
        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1));

        const char code2[] =
            "int long long f()\n"
            "{\n"
            "static const long long signed int i1;\n"
            "static const long long int signed i2;\n"
            "static const signed long long int i3;\n"
            "static const signed int long long i4;\n"
            "static const int signed long long i5;\n"
            "static const int long long signed i6;\n"
            "long long static const signed int i7;\n"
            "long long static const int signed i8;\n"
            "signed static const long long int i9;\n"
            "signed static const int long long i10;\n"
            "int static const signed long long i11;\n"
            "int static const long long signed i12;\n"
            "long long signed int static const i13;\n"
            "long long int signed static const i14;\n"
            "signed long long int static const i15;\n"
            "signed int long long static const i16;\n"
            "int signed long long static const i17;\n"
            "int long long signed static const i18;\n"
            "return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11 + i12\n"
            "+ i13 + i14 + i15 + i16 + i17 + i18;\n"
            "}";
        const char expected2[] =
            "long long f ( )\n"
            "{\n"
            "static const signed long long i1 ;\n"
            "static const signed long long i2 ;\n"
            "static const signed long long i3 ;\n"
            "static const signed long long i4 ;\n"
            "static const signed long long i5 ;\n"
            "static const signed long long i6 ;\n"
            "static const signed long long i7 ;\n"
            "static const signed long long i8 ;\n"
            "static const signed long long i9 ;\n"
            "static const signed long long i10 ;\n"
            "static const signed long long i11 ;\n"
            "static const signed long long i12 ;\n"
            "static const signed long long i13 ;\n"
            "static const signed long long i14 ;\n"
            "static const signed long long i15 ;\n"
            "static const signed long long i16 ;\n"
            "static const signed long long i17 ;\n"
            "static const signed long long i18 ;\n"
            "return i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11 + i12\n"
            "+ i13 + i14 + i15 + i16 + i17 + i18 ;\n"
            "}";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));

        const char code3[] = "const unsigned long extern int i;";
        const char expected3[] = "extern const unsigned long i ;";
        ASSERT_EQUALS(expected3, tokenizeAndStringify(code3));
    }

    void simplifyCPPAttribute() {
        ASSERT_EQUALS("int f ( ) ;",
                      tokenizeAndStringify("[[deprecated]] int f();"));

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("[[deprecated]] int f();", dinit(TokenizeOptions, $.cpp = false)), SYNTAX);

        ASSERT_EQUALS("template < class T > int f ( ) { }",
                      tokenizeAndStringify("template <class T> [[noreturn]] int f(){}"));

        ASSERT_EQUALS("int f ( int i ) ;",
                      tokenizeAndStringify("[[maybe_unused]] int f([[maybe_unused]] int i);"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[,]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[deprecated,]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[,,]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[deprecated,,]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[deprecated,maybe_unused,]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[,,,]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct alignas(int) a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct alignas ( alignof ( float ) ) a;"));

        ASSERT_EQUALS("char a [ 256 ] ;",
                      tokenizeAndStringify("alignas(256) char a[256];"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct alignas(float) [[deprecated(reason)]] a;"));

        ASSERT_EQUALS("struct a ;",
                      tokenizeAndStringify("struct [[deprecated,maybe_unused]] alignas(double) [[trivial_abi]] a;"));

        ASSERT_EQUALS("void func5 ( const char * , ... ) ;",
                      tokenizeAndStringify("[[noreturn]] void func5(const char*, ...);"));

        ASSERT_EQUALS("void func5 ( const char * , ... ) ;",
                      tokenizeAndStringify("[[noreturn]] [[gnu::format(printf, 1, 2)]] void func5(const char*, ...);"));

        ASSERT_EQUALS("void func5 ( const char * , ... ) ;",
                      tokenizeAndStringify("[[gnu::format(printf, 1, 2)]] [[noreturn]] void func5(const char*, ...);"));

        ASSERT_EQUALS("int func1 ( ) ;",
                      tokenizeAndStringify("[[nodiscard]] int func1();"));

        ASSERT_EQUALS("int func1 ( ) ;",
                      tokenizeAndStringify("[[nodiscard]] [[clang::optnone]] int func1();"));

        ASSERT_EQUALS("int func1 ( ) ;",
                      tokenizeAndStringify("[[clang::optnone]] [[nodiscard]] int func1();"));

        const Settings s = settingsBuilder(settings1).c(Standards::C23).build();
        ASSERT_EQUALS("void f ( int i ) { exit ( i ) ; }",
                      tokenizeAndStringify("[[noreturn]] void f(int i) { exit(i); }", s, false));
    }

    void simplifyCaseRange() {
        ASSERT_EQUALS("void f ( int x ) { switch ( x ) { case 1 : case 2 : case 3 : case 4 : ; } }", tokenizeAndStringify("void f(int x) { switch(x) { case 1 ... 4: } }"));
        ASSERT_EQUALS("void f ( int x ) { switch ( x ) { case 4 ... 1 : ; } }", tokenizeAndStringify("void f(int x) { switch(x) { case 4 ... 1: } }"));
        (void)tokenizeAndStringify("void f(int x) { switch(x) { case 1 ... 1000000: } }"); // Do not run out of memory

        ASSERT_EQUALS("void f ( int x ) { switch ( x ) { case 'a' : case 98 : case 'c' : ; } }", tokenizeAndStringify("void f(int x) { switch(x) { case 'a' ... 'c': } }"));
        ASSERT_EQUALS("void f ( int x ) { switch ( x ) { case 'c' ... 'a' : ; } }", tokenizeAndStringify("void f(int x) { switch(x) { case 'c' ... 'a': } }"));

        ASSERT_EQUALS("void f ( int x ) { switch ( x ) { case '[' : case 92 : case ']' : ; } }", tokenizeAndStringify("void f(int x) { switch(x) { case '[' ... ']': } }"));

        ASSERT_EQUALS("void f ( int x ) { switch ( x ) { case '&' : case 39 : case '(' : ; } }", tokenizeAndStringify("void f(int x) { switch(x) { case '&' ... '(': } }"));
        ASSERT_EQUALS("void f ( int x ) { switch ( x ) { case '\\x61' : case 98 : case '\\x63' : ; } }", tokenizeAndStringify("void f(int x) { switch(x) { case '\\x61' ... '\\x63': } }"));
    }

    void simplifyEmptyNamespaces() {
        ASSERT_EQUALS(";", tokenizeAndStringify("namespace { }"));
        ASSERT_EQUALS(";", tokenizeAndStringify("namespace foo { }"));
        ASSERT_EQUALS(";", tokenizeAndStringify("namespace foo { namespace { } }"));
        ASSERT_EQUALS(";", tokenizeAndStringify("namespace { namespace { } }")); // Ticket #9512
        ASSERT_EQUALS(";", tokenizeAndStringify("namespace foo { namespace bar { } }"));
    }

    void prepareTernaryOpForAST() {
        ASSERT_EQUALS("a ? b : c ;", tokenizeAndStringify("a ? b : c;"));

        ASSERT_EQUALS("a ? ( b , c ) : d ;", tokenizeAndStringify("a ? b , c : d;"));
        ASSERT_EQUALS("a ? ( b , c ) : d ;", tokenizeAndStringify("a ? (b , c) : d;"));

        ASSERT_EQUALS("a ? ( 1 ? ( a , b ) : 3 ) : d ;", tokenizeAndStringify("a ? 1 ? a, b : 3 : d;"));

        ASSERT_EQUALS("a ? ( std :: map < int , int > ( ) ) : 0 ;", tokenizeAndStringify("typedef std::map<int,int> mymap; a ? mymap() : 0;"));

        ASSERT_EQUALS("a ? ( b < c ) : d > e", tokenizeAndStringify("a ? b < c : d > e"));
    }

    enum class AstStyle : std :: uint8_t {
        Simple,
        Z3
    };

    enum class ListSimplification : std::uint8_t {
        Partial,
        Full
    };

    template<size_t size>
    std::string testAst(const char (&data)[size], AstStyle style = AstStyle::Simple, ListSimplification ls = ListSimplification::Partial) {
        // tokenize given code..
        TokenList tokenlist{settingsDefault, Standards::Language::CPP};
        tokenlist.appendFileIfNew("test.cpp");
        if (!tokenlist.createTokensFromString(data))
            return "ERROR";

        TokenizerTest tokenizer(std::move(tokenlist), *this);
        if (ls == ListSimplification::Partial) {
            tokenizer.combineStringAndCharLiterals();
            tokenizer.combineOperators();
            tokenizer.simplifySpaceshipOperator();
            tokenizer.createLinks();
            tokenizer.createLinks2();
            tokenizer.simplifyCAlternativeTokens();
            tokenizer.list.front()->assignIndexes();
        } else { // Full
            tokenizer.simplifyTokens1("");
        }

        // set varid..
        for (Token *tok = tokenizer.list.front(); tok; tok = tok->next()) {
            if (tok->str() == "var")
                tok->varId(1);
        }

        // Create AST..
        tokenizer.prepareTernaryOpForAST();
        tokenizer.list.createAst();

        tokenizer.list.validateAst(false);

        // Basic AST validation
        for (const Token *tok = tokenizer.list.front(); tok; tok = tok->next()) {
            if (tok->astOperand2() && !tok->astOperand1() && tok->str() != ";" && tok->str() != ":")
                return "Op2 but no Op1 for token: " + tok->str();
        }

        // Return stringified AST
        if (style == AstStyle::Z3)
            return tokenizer.list.front()->astTop()->astStringZ3();

        std::string ret;
        std::set<const Token *> astTop;
        for (const Token *tok = tokenizer.list.front(); tok; tok = tok->next()) {
            if (tok->astOperand1() && astTop.find(tok->astTop()) == astTop.end()) {
                astTop.insert(tok->astTop());
                if (!ret.empty())
                    ret += " ";
                ret += tok->astTop()->astString();
            }
        }
        return ret;
    }

    void astexpr() { // simple expressions with arithmetical ops
        ASSERT_EQUALS("12+3+", testAst("1+2+3"));
        ASSERT_EQUALS("12*3+", testAst("1*2+3"));
        ASSERT_EQUALS("123*+", testAst("1+2*3"));
        ASSERT_EQUALS("12*34*+", testAst("1*2+3*4"));
        ASSERT_EQUALS("12*34*5*+", testAst("1*2+3*4*5"));
        ASSERT_EQUALS("0(r.&", testAst("(&((typeof(x))0).r);"));
        ASSERT_EQUALS("0(r.&", testAst("&((typeof(x))0).r;"));
        ASSERT_EQUALS("0f1(||", testAst("; 0 || f(1);"));

        // Various tests of precedence
        ASSERT_EQUALS("ab::c+", testAst("a::b+c"));
        ASSERT_EQUALS("abc+=", testAst("a=b+c"));
        ASSERT_EQUALS("abc=,", testAst("a,b=c"));
        ASSERT_EQUALS("a-1+", testAst("-a+1"));
        ASSERT_EQUALS("ab++-c-", testAst("a-b++-c"));
        ASSERT_EQUALS("ab<=>", testAst("a<=>b"));

        // sizeof
        ASSERT_EQUALS("ab.sizeof", testAst("sizeof a.b"));

        // assignment operators
        ASSERT_EQUALS("ab>>=", testAst("a>>=b;"));
        ASSERT_EQUALS("ab<<=", testAst("a<<=b;"));
        ASSERT_EQUALS("ab+=",  testAst("a+=b;"));
        ASSERT_EQUALS("ab-=",  testAst("a-=b;"));
        ASSERT_EQUALS("ab*=",  testAst("a*=b;"));
        ASSERT_EQUALS("ab/=",  testAst("a/=b;"));
        ASSERT_EQUALS("ab%=",  testAst("a%=b;"));
        ASSERT_EQUALS("ab&=",  testAst("a&=b;"));
        ASSERT_EQUALS("ab|=",  testAst("a|=b;"));
        ASSERT_EQUALS("ab^=",  testAst("a^=b;"));

        ASSERT_EQUALS("ab*c*.(+return", testAst("return a + ((*b).*c)();"));

        // assignments are executed from right to left
        ASSERT_EQUALS("abc==", testAst("a=b=c;"));

        // ternary operator
        ASSERT_EQUALS("ab0=c1=:?", testAst("a?b=0:c=1;"));
        ASSERT_EQUALS("fabc,d:?=e,", testAst("f = a ? b, c : d, e;"));
        ASSERT_EQUALS("fabc,de,:?=", testAst("f = (a ? (b, c) : (d, e));"));
        ASSERT_EQUALS("fabc,de,:?=", testAst("f = (a ? b, c : (d, e));"));
        ASSERT_EQUALS("ab35,4:?foo(:?return", testAst("return (a ? b ? (3,5) : 4 : foo());"));
        ASSERT_EQUALS("check(result_type00,{invalid:?return", testAst("return check() ? result_type {0, 0} : invalid;"));
        ASSERT_EQUALS("x01:?return", testAst("return x ? 0 : 1;"));
        ASSERT_EQUALS("x00throw:?return", testAst("return x ? 0 : throw 0;")); // #9768
        ASSERT_EQUALS("val0<1throwval:?return", testAst("return val < 0 ? throw 1 : val;")); // #8526
        ASSERT_EQUALS("ix0<00throw:?=", testAst("int i = x < 0 ? 0 : throw 0;"));
        ASSERT_EQUALS("pa[pb[<1-pa[pb[>:?return", testAst("return p[a] < p[b] ? -1 : p[a] > p[b];"));

        ASSERT_EQUALS("a\"\"=", testAst("a=\"\""));
        ASSERT_EQUALS("a\'\'=", testAst("a=\'\'"));
        ASSERT_EQUALS("'X''a'>", testAst("('X' > 'a')"));
        ASSERT_EQUALS("L'X'L'a'>", testAst("(L'X' > L'a')"));
        ASSERT_EQUALS("u'X'u'a'>", testAst("(u'X' > u'a')"));
        ASSERT_EQUALS("U'X'U'a'>", testAst("(U'X' > U'a')"));
        ASSERT_EQUALS("u8'X'u8'a'>", testAst("(u8'X' > u8'a')"));

        ASSERT_EQUALS("a0>bc/d:?", testAst("(a>0) ? (b/(c)) : d;"));
        ASSERT_EQUALS("abc/+d+", testAst("a + (b/(c)) + d;"));
        ASSERT_EQUALS("x1024x/0:?", testAst("void f() { x ? 1024 / x : 0; }"));

        ASSERT_EQUALS("absizeofd(ef.+(=", testAst("a = b(sizeof(c d) + e.f)"));

        ASSERT_EQUALS("a*b***", testAst("*a * **b;")); // Correctly distinguish between unary and binary operator*

        // strings
        ASSERT_EQUALS("f\"A\"1,(",testAst("f(\"A\" B, 1);"));
        ASSERT_EQUALS("fA1,(",testAst("f(A \"B\", 1);"));

        // C++ : type()
        ASSERT_EQUALS("fint(0,(", testAst("f(int(),0);"));
        ASSERT_EQUALS("f(0,(", testAst("f(int *(),0);"));  // typedef int* X; f(X(),0);
        ASSERT_EQUALS("f((0,(", testAst("f((intp)int *(),0);"));
        ASSERT_EQUALS("zx1(&y2(&|=", testAst("z = (x & (unsigned)1) | (y & (unsigned)2);")); // not type()

        // for
        ASSERT_EQUALS("for;;(", testAst("for(;;)"));
        ASSERT_EQUALS("fora0=a8<a++;;(", testAst("for(a=0;a<8;a++)"));
        ASSERT_EQUALS("fori1=current0=,iNUM<=i++;;(", testAst("for(i = (1), current = 0; i <= (NUM); ++i)"));
        ASSERT_EQUALS("foreachxy,((", testAst("for(each(x,y)){}"));  // it's not well-defined what this ast should be
        ASSERT_EQUALS("forvar1(;;(", testAst("for(int var(1);;)"));
        ASSERT_EQUALS("forab:(", testAst("for (int a : b);"));
        ASSERT_EQUALS("forvarb:(", testAst("for (int *var : b);"));
        ASSERT_EQUALS("forvard:(", testAst("for (a<b> var : d);"));
        ASSERT_EQUALS("forvare:(", testAst("for (a::b<c> var : e);"));
        ASSERT_EQUALS("forx*0=yz;;(", testAst("for(*x=0;y;z)"));
        ASSERT_EQUALS("forx0=y(8<z;;(", testAst("for (x=0;(int)y<8;z);"));
        ASSERT_EQUALS("forab,c:(", testAst("for (auto [a,b]: c);"));
        ASSERT_EQUALS("fora*++;;(", testAst("for (++(*a);;);"));
        ASSERT_EQUALS("foryz:(", testAst("for (decltype(x) *y : z);"));
        ASSERT_EQUALS("for(tmpNULL!=tmptmpnext.=;;( tmpa=", testAst("for ( ({ tmp = a; }) ; tmp != NULL; tmp = tmp->next ) {}"));
        ASSERT_EQUALS("forx0=x;;(", testAst("for (int x=0; x;);"));
        ASSERT_EQUALS("forae*bc.({:(", testAst("for (a *e : {b->c()});"));
        ASSERT_EQUALS("fori0=iasize.(<i++;;( asize.(", testAst("for (decltype(a.size()) i = 0; i < a.size(); ++i);"));
        ASSERT_EQUALS("foria:( asize.(", testAst("for(decltype(a.size()) i:a);"));
        ASSERT_EQUALS("forec0{([,(:( fb.return", testAst("for (auto e : c(0, [](auto f) { return f->b; }));")); // #10802
        ASSERT_EQUALS("forvar1{;;(", testAst("for(int var{1};;)")); // #12867

        // for with initializer (c++20)
        ASSERT_EQUALS("forab=ca:;(", testAst("for(a=b;int c:a)"));

        // problems with multiple expressions
        ASSERT_EQUALS("ax( whilex(", testAst("a(x) while (x)"));
        ASSERT_EQUALS("ifx( i0= whilei(", testAst("if (x) { ({ int i = 0; while(i); }) };"));
        ASSERT_EQUALS("ifx( BUG_ON{!( i0= whilei(", testAst("if (x) { BUG_ON(!({int i=0; while(i);})); }"));
        ASSERT_EQUALS("v0= while{0!=( v0= while{0!=( v0=", testAst("({ v = 0; }); while (({ v = 0; }) != 0); while (({ v = 0; }) != 0);"));


        ASSERT_EQUALS("abc.1:?1+bd.1:?+=", testAst("a =(b.c ? : 1) + 1 + (b.d ? : 1);"));

        ASSERT_EQUALS("catch...(", testAst("try {} catch (...) {}"));

        ASSERT_EQUALS("", testAst("void Foo(Bar&);"));
        ASSERT_EQUALS("", testAst("void Foo(Bar&&);"));

        ASSERT_EQUALS("Barb&", testAst("void Foo(Bar& b);"));
        ASSERT_EQUALS("Barb&&", testAst("void Foo(Bar&& b);"));

        ASSERT_EQUALS("DerivedDerived::(", testAst("Derived::~Derived() {}"));

        ASSERT_EQUALS("ifCA_FarReadfilenew(,sizeofobjtype(,(!(", testAst("if (!CA_FarRead(file, (void far *)new, sizeof(objtype)))")); // #5910 - don't hang if C code is parsed as C++

        // C++17: if (expr1; expr2)
        ASSERT_EQUALS("ifx3=y;(", testAst("if (int x=3; y)"));

        ASSERT_EQUALS("xstdstring::decltypes(a::{=", testAst("auto x = std::string{ decltype(s)::a };"));
        ASSERT_EQUALS("if0decltypest.(X::>(", testAst("if (0 > decltype(s.t)::X) {}"));
    }

    void astexpr2() { // limit for large expressions
        // #7724 - wrong AST causes hang
        // Ideally a proper AST is created for this code.
        const char code1[] = "const char * a(int type) {\n"
                             "  return (\n"
                             "   (type == 1) ? \"\"\n"
                             " : (type == 2) ? \"\"\n"
                             " : (type == 3) ? \"\"\n"
                             " : (type == 4) ? \"\"\n"
                             " : (type == 5) ? \"\"\n"
                             " : (type == 6) ? \"\"\n"
                             " : (type == 7) ? \"\"\n"
                             " : (type == 8) ? \"\"\n"
                             " : (type == 9) ? \"\"\n"
                             " : (type == 10) ? \"\"\n"
                             " : (type == 11) ? \"\"\n"
                             " : (type == 12) ? \"\"\n"
                             " : (type == 13) ? \"\"\n"
                             " : (type == 14) ? \"\"\n"
                             " : (type == 15) ? \"\"\n"
                             " : (type == 16) ? \"\"\n"
                             " : (type == 17) ? \"\"\n"
                             " : (type == 18) ? \"\"\n"
                             " : (type == 19) ? \"\"\n"
                             " : (type == 20) ? \"\"\n"
                             " : (type == 21) ? \"\"\n"
                             " : (type == 22) ? \"\"\n"
                             " : (type == 23) ? \"\"\n"
                             " : (type == 24) ? \"\"\n"
                             " : (type == 25) ? \"\"\n"
                             " : (type == 26) ? \"\"\n"
                             " : (type == 27) ? \"\"\n"
                             " : (type == 28) ? \"\"\n"
                             " : (type == 29) ? \"\"\n"
                             " : (type == 30) ? \"\"\n"
                             " : (type == 31) ? \"\"\n"
                             " : (type == 32) ? \"\"\n"
                             " : (type == 33) ? \"\"\n"
                             " : (type == 34) ? \"\"\n"
                             " : (type == 35) ? \"\"\n"
                             " : (type == 36) ? \"\"\n"
                             " : (type == 37) ? \"\"\n"
                             " : (type == 38) ? \"\"\n"
                             " : (type == 39) ? \"\"\n"
                             " : (type == 40) ? \"\"\n"
                             " : (type == 41) ? \"\"\n"
                             " : (type == 42) ? \"\"\n"
                             " : (type == 43) ? \"\"\n"
                             " : (type == 44) ? \"\"\n"
                             " : (type == 45) ? \"\"\n"
                             " : (type == 46) ? \"\"\n"
                             " : (type == 47) ? \"\"\n"
                             " : (type == 48) ? \"\"\n"
                             " : (type == 49) ? \"\"\n"
                             " : (type == 50) ? \"\"\n"
                             " : (type == 51) ? \"\"\n"
                             " : \"\");\n"
                             "}\n";
        // Ensure that the AST is validated for the simplified token list
        TODO_ASSERT_THROW(tokenizeAndStringify(code1), InternalError); // this should not crash/hang

        const char code2[] = "template<uint64_t kInput>\n" // #11515
                             "struct ConstCTZ {\n"
                             "    static constexpr uint32_t value =\n"
                             "        (kInput & (uint64_t(1) << 0)) ? 0 :\n"
                             "        (kInput & (uint64_t(1) << 1)) ? 1 :\n"
                             "        (kInput & (uint64_t(1) << 2)) ? 2 :\n"
                             "        (kInput & (uint64_t(1) << 3)) ? 3 :\n"
                             "        (kInput & (uint64_t(1) << 4)) ? 4 :\n"
                             "        (kInput & (uint64_t(1) << 5)) ? 5 :\n"
                             "        (kInput & (uint64_t(1) << 6)) ? 6 :\n"
                             "        (kInput & (uint64_t(1) << 7)) ? 7 :\n"
                             "        (kInput & (uint64_t(1) << 8)) ? 8 :\n"
                             "        (kInput & (uint64_t(1) << 9)) ? 9 :\n"
                             "        (kInput & (uint64_t(1) << 10)) ? 10 :\n"
                             "        (kInput & (uint64_t(1) << 11)) ? 11 :\n"
                             "        (kInput & (uint64_t(1) << 12)) ? 12 :\n"
                             "        (kInput & (uint64_t(1) << 13)) ? 13 :\n"
                             "        (kInput & (uint64_t(1) << 14)) ? 14 :\n"
                             "        (kInput & (uint64_t(1) << 15)) ? 15 :\n"
                             "        (kInput & (uint64_t(1) << 16)) ? 16 :\n"
                             "        (kInput & (uint64_t(1) << 17)) ? 17 :\n"
                             "        (kInput & (uint64_t(1) << 18)) ? 18 :\n"
                             "        (kInput & (uint64_t(1) << 19)) ? 19 :\n"
                             "        (kInput & (uint64_t(1) << 20)) ? 20 :\n"
                             "        (kInput & (uint64_t(1) << 21)) ? 21 :\n"
                             "        (kInput & (uint64_t(1) << 22)) ? 22 :\n"
                             "        (kInput & (uint64_t(1) << 23)) ? 23 :\n"
                             "        (kInput & (uint64_t(1) << 24)) ? 24 :\n"
                             "        (kInput & (uint64_t(1) << 25)) ? 25 :\n"
                             "        (kInput & (uint64_t(1) << 26)) ? 26 :\n"
                             "        (kInput & (uint64_t(1) << 27)) ? 27 :\n"
                             "        (kInput & (uint64_t(1) << 28)) ? 28 :\n"
                             "        (kInput & (uint64_t(1) << 29)) ? 29 :\n"
                             "        (kInput & (uint64_t(1) << 30)) ? 30 :\n"
                             "        (kInput & (uint64_t(1) << 31)) ? 31 :\n"
                             "        (kInput & (uint64_t(1) << 32)) ? 32 :\n"
                             "        (kInput & (uint64_t(1) << 33)) ? 33 :\n"
                             "        (kInput & (uint64_t(1) << 34)) ? 34 :\n"
                             "        (kInput & (uint64_t(1) << 35)) ? 35 :\n"
                             "        (kInput & (uint64_t(1) << 36)) ? 36 :\n"
                             "        (kInput & (uint64_t(1) << 37)) ? 37 :\n"
                             "        (kInput & (uint64_t(1) << 38)) ? 38 :\n"
                             "        (kInput & (uint64_t(1) << 39)) ? 39 :\n"
                             "        (kInput & (uint64_t(1) << 40)) ? 40 :\n"
                             "        (kInput & (uint64_t(1) << 41)) ? 41 :\n"
                             "        (kInput & (uint64_t(1) << 42)) ? 42 :\n"
                             "        (kInput & (uint64_t(1) << 43)) ? 43 :\n"
                             "        (kInput & (uint64_t(1) << 44)) ? 44 :\n"
                             "        (kInput & (uint64_t(1) << 45)) ? 45 :\n"
                             "        (kInput & (uint64_t(1) << 46)) ? 46 :\n"
                             "        (kInput & (uint64_t(1) << 47)) ? 47 :\n"
                             "        (kInput & (uint64_t(1) << 48)) ? 48 :\n"
                             "        (kInput & (uint64_t(1) << 49)) ? 49 :\n"
                             "        (kInput & (uint64_t(1) << 50)) ? 50 :\n"
                             "        (kInput & (uint64_t(1) << 51)) ? 51 :\n"
                             "        (kInput & (uint64_t(1) << 52)) ? 52 :\n"
                             "        (kInput & (uint64_t(1) << 53)) ? 53 :\n"
                             "        (kInput & (uint64_t(1) << 54)) ? 54 :\n"
                             "        (kInput & (uint64_t(1) << 55)) ? 55 :\n"
                             "        (kInput & (uint64_t(1) << 56)) ? 56 :\n"
                             "        (kInput & (uint64_t(1) << 57)) ? 57 :\n"
                             "        (kInput & (uint64_t(1) << 58)) ? 58 :\n"
                             "        (kInput & (uint64_t(1) << 59)) ? 59 :\n"
                             "        (kInput & (uint64_t(1) << 60)) ? 60 :\n"
                             "        (kInput & (uint64_t(1) << 61)) ? 61 :\n"
                             "        (kInput & (uint64_t(1) << 62)) ? 62 :\n"
                             "        (kInput & (uint64_t(1) << 63)) ? 63 : 64;\n"
                             "};\n";
        ASSERT_NO_THROW(tokenizeAndStringify(code2));

        const char code3[] = "void f(const std::vector<int>& v) {\n" // #12569
                             "    ::std::for_each(v.begin(), v.end(), [](int i) {\n"
                             "        int j(i ? i : 5);\n"
                             "    });\n"
                             "}\n";
        ASSERT_NO_THROW(tokenizeAndStringify(code3));
    }

    void astnewdelete() {
        ASSERT_EQUALS("aintnew=", testAst("a = new int;"));
        ASSERT_EQUALS("aint4[new=", testAst("a = new int[4];"));
        ASSERT_EQUALS("aFoobar(new=", testAst("a = new Foo(bar);"));
        ASSERT_EQUALS("aFoobar(new=", testAst("a = new Foo(bar);"));
        ASSERT_EQUALS("aFoo(new=", testAst("a = new Foo<bar>();"));
        ASSERT_EQUALS("aXnew(", testAst("a (new (X));"));
        ASSERT_EQUALS("aXnew5,(", testAst("a (new (X), 5);"));
        ASSERT_EQUALS("adelete", testAst("delete a;"));
        ASSERT_EQUALS("adelete", testAst("delete (a);"));
        ASSERT_EQUALS("adelete", testAst("delete[] a;"));
        ASSERT_EQUALS("ab.3c-(delete", testAst("delete[] a.b(3 - c);"));
        ASSERT_EQUALS("aA1(new(bB2(new(,", testAst("a(new A(1)), b(new B(2))"));
        ASSERT_EQUALS("Fred10[new", testAst(";new Fred[10];"));
        ASSERT_EQUALS("adelete", testAst("void f() { delete a; }"));
        ASSERT_EQUALS("Aa*A{new=", testAst("A* a = new A{};"));
        ASSERT_EQUALS("Aa*A12,{new=", testAst("A* a = new A{ 1, 2 };"));
        ASSERT_EQUALS("Sv0[(new", testAst("new S(v[0]);")); // #10929
        ASSERT_EQUALS("SS::x(px0>intx[{newint1[{new:?(:", testAst("S::S(int x) : p(x > 0 ? new int[x]{} : new int[1]{}) {}")); // #10793
        ASSERT_EQUALS("a0[T{new=", testAst("a[0] = new T{};"));
        ASSERT_EQUALS("a0[T::{new=", testAst("a[0] = new ::T{};"));
        ASSERT_EQUALS("a0[ST::{new=", testAst("a[0] = new S::T{};"));
        ASSERT_EQUALS("intnewdelete", testAst("delete new int;")); // #11039
        ASSERT_EQUALS("intnewdelete", testAst("void f() { delete new int; }"));
        ASSERT_EQUALS("pint3[new1+=", testAst("p = (new int[3]) + 1;")); // #11327
        ASSERT_EQUALS("aType2[T1T2,{new=", testAst("a = new Type *[2] {T1, T2};")); // #11745
        ASSERT_EQUALS("pSthis(new=", testAst("p = new S*(this);")); // #10809
        ASSERT_EQUALS("pint0{new=", testAst("p = new int*{ 0 };"));
        ASSERT_EQUALS("pint5[{new=", testAst("p = new int* [5]{};"));
        ASSERT_EQUALS("pint5[0{new=", testAst("p = new int* [5]{ 0 };"));
        ASSERT_EQUALS("sSint(new::(new=", testAst("s = new S(::new int());")); // #12502
        ASSERT_EQUALS("sS(new::=", testAst("s = ::new (ptr) S();")); // #12552
        ASSERT_EQUALS("pdelete::return", testAst("return ::delete p;"));
        ASSERT_EQUALS("gn--(delete", testAst("delete g(--n);"));
        ASSERT_EQUALS("bdeletep1*p0*,deletep0*p1*,:?*return", testAst("return *(b ? (delete *p1, *p0) : (delete *p0, *p1));")); // #10662

        // placement new
        ASSERT_EQUALS("X12,3,(new ab,c,", testAst("new (a,b,c) X(1,2,3);"));
        ASSERT_EQUALS("aX::new=", testAst("a = new (b) ::X;"));
        ASSERT_EQUALS("cCnew= abc:?", testAst("c = new(a ? b : c) C;"));

        // invalid code (libreoffice), don't hang
        // #define SlideSorterViewShell
        // SfxViewFrame* pFrame;
        // new SlideSorterViewShell(pFrame,rViewShellBase,pParentWindow,pFrameViewArgument);
        ASSERT_EQUALS("fxnewy,z,(", testAst("f(new (x,y,z));"));

        // clang testsuite..
        ASSERT_EQUALS("const0(new", testAst("new const auto (0);"));
        ASSERT_EQUALS("autonew", testAst("new (auto) (0.0);"));
        ASSERT_EQUALS("int3[4[5[new", testAst("new (int S::*[3][4][5]) ();"));
        ASSERT_EQUALS("pSnew=", testAst("p=new (x)(S)(1,2);"));
        ASSERT_EQUALS("inti[new(", testAst("(void)new (int[i]);"));
        ASSERT_EQUALS("intp* pnew malloc4(", testAst("int*p; new (p) (malloc(4));"));
        ASSERT_EQUALS("intnew", testAst("new (&w.x)(int*)(0);"));
        ASSERT_EQUALS("&new", testAst("new (&w.x)(0);")); // <- the "(int*)" has been simplified

        // gcc testsuite..
        ASSERT_EQUALS("char10[new(", testAst("(void)new(char*)[10];"));
    }

    void astpar() { // parentheses
        ASSERT_EQUALS("12+3*", testAst("(1+2)*3"));
        ASSERT_EQUALS("123+*", testAst("1*(2+3)"));
        ASSERT_EQUALS("123+*4*", testAst("1*(2+3)*4"));
        ASSERT_EQUALS("ifab.c&d==(", testAst("if((a.b&c)==d){}"));

        ASSERT_EQUALS("pf.pf.12,(&&", testAst("((p.f) && (p.f)(1,2))"));

        ASSERT_EQUALS("forresdirGetFirst.file&_T(,(=;;(", testAst("for ((res = dir.GetFirst(&file, _T(" ")));;) {}"));

        // problems with: if (x[y]==z)
        ASSERT_EQUALS("ifa(0[1==(", testAst("if(a()[0]==1){}"));
        ASSERT_EQUALS("ifbuff0[&(*1==(", testAst("if (*((DWORD*)&buff[0])==1){}"));
        ASSERT_EQUALS("ifp*0[1==(", testAst("if((*p)[0]==1)"));
        ASSERT_EQUALS("ifab.cd.[e==(", testAst("if(a.b[c.d]==e){}"));

        ASSERT_EQUALS("iftpnote.i1-[note.0==tpnote.i1-[type.4>||(", testAst("if ((tp.note[i - 1].note == 0) || (tp.note[i - 1].type > 4)) {}"));
        ASSERT_EQUALS("ab.i[j1+[", testAst("a.b[i][j+1]"));

        // problems with: x=expr
        ASSERT_EQUALS("(= x (( (. ([ a i) f)))",
                      testAst("x = ((a[i]).f)();", AstStyle::Z3));
        ASSERT_EQUALS("abc.de.++[=", testAst("a = b.c[++(d.e)];"));
        ASSERT_EQUALS("abc(1+=", testAst("a = b(c**)+1;"));
        ASSERT_EQUALS("abc.=", testAst("a = (b).c;"));

        // casts
        ASSERT_EQUALS("a1(2(+=",testAst("a=(t)1+(t)2;"));
        ASSERT_EQUALS("a1(2+=",testAst("a=(t)1+2;"));
        ASSERT_EQUALS("a1(2+=",testAst("a=(t*)1+2;"));
        ASSERT_EQUALS("a1(2+=",testAst("a=(t&)1+2;"));
        ASSERT_EQUALS("a1(2+=",testAst("a=(t&&)1+2;"));
        ASSERT_EQUALS("ab::r&c(=", testAst("a::b& r = (a::b&)c;")); // #5261
        ASSERT_EQUALS("ab10:?=", testAst("a=(b)?1:0;"));
        ASSERT_EQUALS("ac5[new(=", testAst("a = (b*)(new c[5]);")); // #8786
        ASSERT_EQUALS("a(4+", testAst("(int)(a) + 4;"));

        // (cast){data}[index]
        ASSERT_EQUALS("a&{(0[1[5[0=", testAst("(int (**)[i]){&a}[0][1][5] = 0;"));
        ASSERT_EQUALS("ab12,{(0[,(", testAst("a(b, (int []){1,2}[0]);"));
        ASSERT_EQUALS("TrivialDefCtora2[2[{1[1[n.0=", testAst("TrivialDefCtor{a[2][2]}[1][1].n = 0;"));
        ASSERT_EQUALS("aT12,3,{1[=", testAst("a = T{1, 2, 3}[1];"));

        // Type{data}()
        ASSERT_EQUALS("ab{(=", testAst("a=b{}();"));
        ASSERT_EQUALS("abc{((=", testAst("a=b(c{}());"));
        ASSERT_EQUALS("xNULL!=0(x(:?", testAst("void f() { {} ((x != NULL) ? (void)0 : x()); }"));

        // ({..})
        ASSERT_EQUALS("a{+d+ bc+", testAst("a+({b+c;})+d"));
        ASSERT_EQUALS("a{d*+ bc+", testAst("a+({b+c;})*d"));
        ASSERT_EQUALS("xa{((= bc( yd{((= ef(",
                      testAst("x=(int)(a({b(c);}));" // don't hang
                              "y=(int)(d({e(f);}));"));
        ASSERT_EQUALS("A{{,( x0= Bx1={x2={,(",  // TODO: This is not perfect!!
                      testAst("A({},{x=0;});" // don't hang
                              "B({x=1},{x=2});"));
        ASSERT_EQUALS("xMACROtype.T=value.1=,{({=",
                      testAst("x = { MACRO( { .type=T, .value=1 } ) }")); // don't hang: MACRO({..})
        ASSERT_EQUALS("fori10=i{;;( i--", testAst("for (i=10;i;({i--;}) ) {}"));
        ASSERT_EQUALS("c{1{,{2.3f{,(",
                      testAst("c({{}, {1}}, {2.3f});"));
        ASSERT_EQUALS("x{{= e0= assert0(", testAst("x = {({ int e = 0; assert(0); e; })};"));

        // function pointer
        TODO_ASSERT_EQUALS("todo", "va_argapvoid((,(*0=", testAst("*va_arg(ap, void(**) ()) = 0;"));

        // struct/array initialization
        ASSERT_EQUALS("name_bytes[bits~unusedBits>>unusedBits<<{=", testAst("const uint8_t name_bytes[] = { (~bits >> unusedBits) << unusedBits };"));
        ASSERT_EQUALS("abuf.0{={=", testAst("a = { .buf = { 0 } };"));
        ASSERT_EQUALS("ab2[a.0=b.0=,{a.0=b.0=,{,{=", testAst("struct AB ab[2] = { { .a=0, .b=0 }, { .a=0, .b=0 } };"));
        ASSERT_EQUALS("tset{=", testAst("struct cgroup_taskset tset = {};"));
        ASSERT_EQUALS("s1a&,{2b&,{,{=", testAst("s = { {1, &a}, {2, &b} };"));
        ASSERT_EQUALS("s0[L.2[x={=", testAst("s = { [0].L[2] = x};"));
        ASSERT_EQUALS("ac.0={(=", testAst("a = (b){.c=0,};")); // <- useless comma
        ASSERT_EQUALS("xB[1y.z.1={(&=,{={=", testAst("x = { [B] = {1, .y = &(struct s) { .z=1 } } };"));
        ASSERT_EQUALS("xab,c,{=", testAst("x={a,b,(c)};"));
        ASSERT_EQUALS("x0fSa.1=b.2=,c.\"\"=,{(||=", testAst("x = 0 || f(S{.a = 1, .b = 2, .c = \"\" });"));
        ASSERT_EQUALS("x0fSa.1{=b.2{,c.\"\"=,{(||=", testAst("x = 0 || f(S{.a = { 1 }, .b { 2 }, .c = \"\" });"));
        ASSERT_EQUALS("a0\"\"abc12:?,{{,(", testAst("a(0, {{\"\", (abc) ? 1 : 2}});"));
        ASSERT_EQUALS("a0\'\'abc12:?,{{,(", testAst("a(0, {{\'\', (abc) ? 1 : 2}});"));
        ASSERT_EQUALS("x12,{34,{,{56,{78,{,{,{=", testAst("x = { { {1,2}, {3,4} }, { {5,6}, {7,8} } };"));
        ASSERT_EQUALS("Sa.stdmove::s(=b.1=,{(", testAst("S({.a = std::move(s), .b = 1})"));

        // struct initialization hang
        ASSERT_EQUALS("sbar.1{,{(={= forfieldfield++;;(",
                      testAst("struct S s = {.bar = (struct foo) { 1, { } } };\n"
                              "void f(struct cmd *) { for (; field; field++) {} }"));

        // template parentheses: <>
        ASSERT_EQUALS("ab::c(de::(<=return", testAst("return a::b(c) <= d<double>::e();")); // #6195

        // C++ initializer
        ASSERT_EQUALS("Class{", testAst("Class{};"));
        ASSERT_EQUALS("Class12,{", testAst("Class{1,2};"));
        ASSERT_EQUALS("Class12,{", testAst("Class<X>{1,2};"));
        ASSERT_EQUALS("abc{d:?=", testAst("a=b?c{}:d;"));
        ASSERT_EQUALS("abc12,{d:?=", testAst("a=b?c{1,2}:d;"));
        ASSERT_EQUALS("abc{d:?=", testAst("a=b?c<X>{}:d;"));
        ASSERT_EQUALS("abc12,{d:?=", testAst("a=b?c<X>{1,2}:d;"));
        ASSERT_EQUALS("a::12,{", testAst("::a{1,2};")); // operator precedence
        ASSERT_EQUALS("Abc({newreturn", testAst("return new A {b(c)};"));
        ASSERT_EQUALS("a{{return", testAst("return{{a}};"));
        ASSERT_EQUALS("a{b{,{return", testAst("return{{a},{b}};"));
        ASSERT_EQUALS("stdvector::{{,{return", testAst("return std::vector<std::vector<int> >{{},{}};"));
        ASSERT_EQUALS("stdvector::{2{,{return", testAst("return std::vector<std::vector<int> >{{}, {2}};"));
        ASSERT_EQUALS("forbstdvector::{{,{:(", testAst("for (auto b : std::vector<std::vector<int> >{{},{}});"));
        ASSERT_EQUALS("forbstdvector::{2{,{:(", testAst("for (auto b : std::vector<std::vector<int> >{{}, {2}});"));
        ASSERT_EQUALS("abR{{,P(,((", testAst("a(b(R{},{},P()));"));
        ASSERT_EQUALS("f1{2{,3{,{x,(", testAst("f({{1},{2},{3}},x);"));
        ASSERT_EQUALS("a1{ b2{", testAst("auto a{1}; auto b{2};"));
        ASSERT_EQUALS("var1ab::23,{,{4ab::56,{,{,{", testAst("auto var{{1,a::b{2,3}}, {4,a::b{5,6}}};"));
        ASSERT_EQUALS("var{{,{{,{", testAst("auto var{ {{},{}}, {} };"));
        ASSERT_EQUALS("fXYabcfalse==CD:?,{,{(", testAst("f({X, {Y, abc == false ? C : D}});"));
        ASSERT_EQUALS("stdvector::p0[{(return", testAst("return std::vector<int>({ p[0] });"));
        ASSERT_EQUALS("vstdvector::{=", testAst("auto v = std::vector<int>{ };"));

        // Initialization with decltype(expr) instead of a type
        ASSERT_EQUALS("decltypex((", testAst("decltype(x)();"));
        ASSERT_EQUALS("decltypex({", testAst("decltype(x){};"));
        ASSERT_EQUALS("decltypexy+(yx+(", testAst("decltype(x+y)(y+x);"));
        ASSERT_EQUALS("decltypexy+(yx+{", testAst("decltype(x+y){y+x};"));
        ASSERT_EQUALS("adecltypeac::(,decltypead::(,",
                      testAst("template <typename a> void b(a &, decltype(a::c), decltype(a::d));"));

        ASSERT_NO_THROW(tokenizeAndStringify("struct A;\n" // #10839
                                             "struct B { A* hash; };\n"
                                             "auto g(A* a) { return [=](void*) { return a; }; }\n"
                                             "void f(void* p, B* b) {\n"
                                             "    b->hash = (g(b->hash))(p);\n"
                                             "}\n"));
        ignore_errout();
        ASSERT_NO_THROW(tokenizeAndStringify("struct A;\n"
                                             "struct B { A* hash; };\n"
                                             "A* h(void* p);\n"
                                             "typedef A* (*X)(void*);\n"
                                             "X g(A*) { return h; }\n"
                                             "void f(void* p, B * b) {\n"
                                             "b->hash = (g(b->hash))(p);\n"
                                             "}\n"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_NO_THROW(tokenizeAndStringify("struct A;\n"
                                             "struct B { A* hash; };\n"
                                             "void f(void* p, B* b) {\n"
                                             "    b->hash = (decltype(b->hash))(p);\n"
                                             "}\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify("void a(int);\n" // #10801
                                             "    struct b {\n"
                                             "    static int c();\n"
                                             "} d;\n"
                                             "void f() {\n"
                                             "    (decltype (&a)(d.c))(0);\n"
                                             "}\n"));
        ASSERT_EQUALS("", errout_str());

        // #10334: Do not hang!
        (void)tokenizeAndStringify("void foo(const std::vector<std::string>& locations = {\"\"}) {\n"
                                   "    for (int i = 0; i <= 123; ++i)\n"
                                   "        x->emplace_back(y);\n"
                                   "}");
        ignore_errout();

        ASSERT_NO_THROW(tokenizeAndStringify("void f() {\n" // #10831
                                             "    auto g = [](std::function<void()> h = []() {}) { };\n"
                                             "}"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify("void f() {\n" // #11379
                                             "    auto l = [x = 3](std::string&& v) { };\n"
                                             "}\n"));
        ASSERT_EQUALS(
            "[test.cpp:2:15]: (debug) valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x [valueFlowBailoutIncompleteVar]\n",
            errout_str());

        ASSERT_EQUALS("forinti(0=i5<=i++;;(", testAst("for (int (i) = 0; (i) <= 5; (i)++) {}")); // #13225
    }

    void astbrackets() { // []
        ASSERT_EQUALS("a23+[4+", testAst("a[2+3]+4"));
        ASSERT_EQUALS("a1[0[", testAst("a[1][0]"));
        ASSERT_EQUALS("ab0[=", testAst("a=(b)[0];"));
        ASSERT_EQUALS("abc.0[=", testAst("a=b.c[0];"));
        ASSERT_EQUALS("ab0[1[=", testAst("a=b[0][1];"));
    }

    void astvardecl() {
        // Variable declaration
        ASSERT_EQUALS("a1[\"\"=", testAst("char a[1]=\"\";"));
        ASSERT_EQUALS("charp*(3[char5[3[new=", testAst("char (*p)[3] = new char[5][3];"));
        ASSERT_EQUALS("varp=", testAst("const int *var = p;"));
        ASSERT_EQUALS("intrp0[*(&", testAst("int& r(*p[0]);"));

        // #9127
        const char code1[] = "using uno::Ref;\n"
                             "Ref<X> r;\n"
                             "int var(0);";
        ASSERT_EQUALS("unoRef:: var0(", testAst(code1));

        ASSERT_EQUALS("vary=", testAst("std::string var = y;"));
        ASSERT_EQUALS("vary=", testAst("std::unique_ptr<int> var = y;")); // #14019

        ASSERT_EQUALS("", testAst("void *(*var)(int);"));
        ASSERT_EQUALS("", testAst("void *(*var[2])(int);"));


        // create ast for decltype
        ASSERT_EQUALS("decltypex( var1=", testAst("decltype(x) var = 1;"));
        ASSERT_EQUALS("a1bdecltypet((>2,(", testAst("a(1 > b(decltype(t)), 2);")); // #10271
        ASSERT_EQUALS("decltypex({01:?", testAst("decltype(x){} ? 0 : 1;"));

        ASSERT_EQUALS("Tp* Tt* forctp.=;;( tp.", testAst("struct T { T* p; };\n" // #10874
                                                         "void f(T * t) {\n"
                                                         "    for (decltype(t->p) (c) = t->p; ;) {}\n"
                                                         "}\n"));
        ASSERT_EQUALS("x0=a, stdtie::a(x=", testAst("int x = 0, a; std::tie(a) = x;\n"));
        ASSERT_EQUALS("tmpa*=a*b*=,b*tmp=,", testAst("{ ((tmp) = (*a)), ((*a) = (*b)), ((*b) = (tmp)); }"));
        ASSERT_EQUALS("a(*v=", testAst("(*(volatile unsigned int *)(a) = (v));"));
        ASSERT_EQUALS("i(j=", testAst("(int&)(i) = j;"));

        ASSERT_EQUALS("", testAst("void f(enum E* var){}"));
        ASSERT_EQUALS("", testAst("void f(enum E*& var){}"));
        ASSERT_EQUALS("", testAst("void f(bool& var){}"));
    }

    void astunaryop() { // unary operators
        ASSERT_EQUALS("1a--+", testAst("1 + --a"));
        ASSERT_EQUALS("1a--+", testAst("1 + a--"));
        ASSERT_EQUALS("ab+!", testAst("!(a+b)"));
        ASSERT_EQUALS("ab.++", testAst("++a.b;"));
        ASSERT_EQUALS("ab.++", testAst("a.b++;"));
        ASSERT_EQUALS("ab::++", testAst("a::b++;"));
        ASSERT_EQUALS("c5[--*", testAst("*c[5]--;"));
        ASSERT_EQUALS("xreturn", testAst("return x;"));
        ASSERT_EQUALS("x(throw", testAst(";throw x();"));
        ASSERT_EQUALS("a*bc:?return", testAst("return *a ? b : c;"));
        ASSERT_EQUALS("xy*--=", testAst("x = -- * y;"));
        ASSERT_EQUALS("x(throw", testAst(";throw (foo) x;")); // #9955

        // Unary :: operator
        ASSERT_EQUALS("abcd::12,(e/:?=", testAst("a = b ? c : ::d(1,2) / e;"));

        // how is "--" handled here:
        ASSERT_EQUALS("ab4<<c--+1:?", testAst("a ? (b << 4) + --c : 1"));
        ASSERT_EQUALS("ab4<<c--+1:?", testAst("a ? (b << 4) + c-- : 1"));
        ASSERT_EQUALS("ai[i= i--", testAst("a[i]=i; --i;"));

        ASSERT_EQUALS("fint0{1&(", testAst("f(int{ 0 } & 1);")); // #11572
        ASSERT_EQUALS("int0{1&return", testAst("int g() { return int{ 0 } & 1; }"));
    }

    void astfunction() { // function calls
        ASSERT_EQUALS("1f(+2+", testAst("1+f()+2"));
        ASSERT_EQUALS("1f2(+3+", testAst("1+f(2)+3"));
        ASSERT_EQUALS("1f23,(+4+", testAst("1+f(2,3)+4"));
        ASSERT_EQUALS("1f2a&,(+", testAst("1+f(2,&a)"));
        ASSERT_EQUALS("argv[", testAst("int f(char argv[]);"));
        ASSERT_EQUALS("", testAst("void f();"));
        ASSERT_EQUALS("", testAst("void f() {}"));
        ASSERT_EQUALS("", testAst("int f() = delete;"));
        ASSERT_EQUALS("", testAst("a::b f();"));
        ASSERT_EQUALS("", testAst("a::b f() {}"));
        ASSERT_EQUALS("", testAst("a::b f() = delete;"));
        ASSERT_EQUALS("", testAst("int f() const = delete;"));
        ASSERT_EQUALS("", testAst("extern unsigned f(const char *);"));
        ASSERT_EQUALS("charformat*...,", testAst("extern void f(const char *format, ...);"));
        ASSERT_EQUALS("int(int(void,", testAst("extern int for_each_commit_graft(int (*)(int*), void *);"));
        ASSERT_EQUALS("for;;(", testAst("for (;;) {}"));
        ASSERT_EQUALS("xsizeofvoid(=", testAst("x=sizeof(void*)"));
        ASSERT_EQUALS("abc{d{,{(=", testAst("a = b({ c{}, d{} });"));
        ASSERT_EQUALS("abc;(", testAst("a(b;c)"));
        ASSERT_EQUALS("x{( forbc;;(", testAst("x({ for(a;b;c){} });"));
        ASSERT_EQUALS("PT.(", testAst("P->~T();"));  // <- The "T" token::function() will be a destructor
        ASSERT_EQUALS("double&(4[", testAst("void f(double(&)[4]) {}"));
        ASSERT_EQUALS("voidu*", testAst("int* g ( void* (f) (void*), void* u);")); // #12475
        ASSERT_EQUALS("f::(", testAst("::f();")); // #12544
        ASSERT_EQUALS("(( f (, c ({ (= (. x) 0))))", testAst("f(c, { .x = 0 });", AstStyle::Z3)); // #12806
        ASSERT_EQUALS("(= it (( (. s insert) (, it ({ (, (, (= (. a) i) (= (. b) 2)) (= (. c) 3))))))",
                      testAst("it = s.insert(it, { .a = i, .b = 2, .c = 3 });", AstStyle::Z3)); // #12815
    }

    void asttemplate() { // uninstantiated templates will have <,>,etc..
        ASSERT_EQUALS("a(3==", testAst("a<int>()==3"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("ab(== f(", testAst("a == b<c>(); f();"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("static_casta(i[", testAst("; static_cast<char*>(a)[i];")); // #6203
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("reinterpret_castreinterpret_castptr(123&(",
                      testAst(";reinterpret_cast<void*>(reinterpret_cast<unsigned>(ptr) & 123);")); // #7253
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("bcd.(=", testAst(";a<int> && b = c->d();"));
        ASSERT_EQUALS("", errout_str());

        // This two unit tests were added to avoid a crash. The actual correct AST result for non-executable code has not been determined so far.
        ASSERT_NO_THROW(testAst("class C : public ::a::b<bool> { };"));
        ASSERT_EQUALS("", errout_str());
        ASSERT_EQUALS("AB: abc+=", testAst("struct A : public B<C*> { void f() { a=b+c; } };"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS("xfts(=", testAst("; auto x = f(ts...);"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS("dae(new= ifd(", testAst("template <typename a, typename... b>\n" // #10199
                                               "void c(b... e) {\n"
                                               "    a d = new a((e)...);\n"
                                               "    if (d) {}\n"
                                               "}\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS("ad*astdforward::e((new= ifd(", testAst("struct a {};\n" // #11103
                                                              "template <class... b> void c(b... e) {\n"
                                                              "    a* d = new a(std::forward<b>(e)...);\n"
                                                              "    if (d) {}\n"
                                                              "}\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_EQUALS("stddir::Args...&&, dir\"abc\"+= dirconcatstdforward::args((+return",
                      testAst("template <typename ...Args> std::string concat(std::string dir, Args&& ...args) {\n" // #10492
                              "    dir += \"abc\";\n"
                              "    return dir + concat(std::forward<Args>(args)...);\n"
                              "}\n"));
        ASSERT_EQUALS("", errout_str());

        // #11369
        ASSERT_NO_THROW(tokenizeAndStringify("int a;\n"
                                             "template <class> auto b() -> decltype(a) {\n"
                                             "    if (a) {}\n"
                                             "}\n"));
        ignore_errout();
    }

    void astrequires()
    {
        ASSERT_EQUALS("brequires{ac::||= ac::", testAst("template <class a> concept b = requires { a::c; } || a::c;"));
        ASSERT_EQUALS("crequires{ac::||= a{b{||",
                      testAst("template <class a, class b> concept c = requires { a{} || b{}; } || a::c;"));
        ASSERT_EQUALS("ifrequires{(", testAst("if (requires { true; }) {}")); // #13308
        ASSERT_EQUALS("Crequires({requires({||= sizeofT(4== sizeofT(8==", testAst("concept C = requires() { sizeof(T) == 4; } || requires() { sizeof(T) == 8; };"));
    }

    void astcast() {
        ASSERT_EQUALS("ac&(=", testAst("a = (long)&c;"));
        ASSERT_EQUALS("ac*(=", testAst("a = (Foo*)*c;"));
        ASSERT_EQUALS("ac-(=", testAst("a = (long)-c;"));
        ASSERT_EQUALS("ac~(=", testAst("a = (b)~c;"));
        ASSERT_EQUALS("ac(=", testAst("a = (some<strange, type>)c;"));
        ASSERT_EQUALS("afoveon_avgimage((foveon_avgimage((+=", testAst("a = foveon_avg(((short(*)[4]) image)) + foveon_avg(((short(*)[4]) image));"));
        ASSERT_EQUALS("c(40<<return", testAst("return (long long)c << 40;"));
        ASSERT_EQUALS("ab-(=", testAst("a = ((int)-b)")); // Multiple subsequent unary operators (cast and -)
        ASSERT_EQUALS("xdouble123(i*(=", testAst("x = (int)(double(123)*i);"));
        ASSERT_EQUALS("ac(=", testAst("a = (::b)c;"));
        ASSERT_EQUALS("abcd,({(=", testAst("a = (s){b(c, d)};"));
        ASSERT_EQUALS("xatoistr({(=", testAst("x = (struct X){atoi(str)};"));
        ASSERT_EQUALS("xa.0=b.0=,c.0=,{(=", testAst("x = (struct abc) { .a=0, .b=0, .c=0 };"));

        ASSERT_EQUALS("yz.(return", testAst("return (x)(y).z;"));

        ASSERT_EQUALS("fon!(restoring01:?,(", testAst("f((long) !on, restoring ? 0 : 1);"));

        ASSERT_EQUALS("esi.!(=", testAst("E e = (E)!s->i;")); // #10882

        ASSERT_EQUALS("xp(= 12>34:?", testAst("x = ( const char ( * ) [ 1 > 2 ? 3 : 4 ] ) p ;"));

        ASSERT_EQUALS("f{(si.,(", testAst("f((struct S){ }, s->i);")); // #11606

        // not cast
        ASSERT_EQUALS("AB||", testAst("(A)||(B)"));
        ASSERT_EQUALS("abc[1&=", testAst("a = (b[c]) & 1;"));
        ASSERT_EQUALS("abc::(=", testAst("a = (b::c)();"));

        ASSERT_EQUALS("pcharnew(=", testAst("p = (void *)(new char);"));
    }

    void astlambda() {
        // a lambda expression '[x](y){}' is compiled as:
        // [
        // `-(
        //   `-{

        ASSERT_EQUALS("x{(a&[( ai=", testAst("x([&a](int i){a=i;});"));
        ASSERT_EQUALS("{([(return 0return", testAst("return [](){ return 0; }();"));

        // noexcept (which if simplified to always have a condition by the time AST is created)
        ASSERT_EQUALS("x{([( ai=", testAst("x([](int i) noexcept(true) { a=i; });"));
        ASSERT_EQUALS("x{([( ai=", testAst("x([](int i) mutable noexcept(true) { a=i; });"));
        ASSERT_EQUALS("x{([( ai=", testAst("x([](int i) const noexcept(true) { a=i; });"));

        // both mutable and constexpr (which is simplified to 'const' by the time AST is created)
        ASSERT_EQUALS("x{([( ai=", testAst("x([](int i) const mutable { a=i; });"));
        ASSERT_EQUALS("x{([( ai=", testAst("x([](int i) mutable const { a=i; });"));
        ASSERT_EQUALS("x{([( ai=", testAst("x([](int i) const mutable noexcept(true) { a=i; });"));
        ASSERT_EQUALS("x{([( ai=", testAst("x([](int i) mutable const noexcept(true) { a=i; });"));

        // ->
        ASSERT_EQUALS("{([(return 0return", testAst("return []() -> int { return 0; }();"));
        ASSERT_EQUALS("{(something[(return 0return", testAst("return [something]() -> int { return 0; }();"));
        ASSERT_EQUALS("{([cd,(return 0return", testAst("return [](int a, int b) -> int { return 0; }(c, d);"));
        ASSERT_EQUALS("{([return", testAst("return []() -> decltype(0) {};"));
        ASSERT_EQUALS("x{(&[=", testAst("x = [&]()->std::string const & {};"));
        ASSERT_EQUALS("f{([=", testAst("f = []() -> foo* {};"));
        ASSERT_EQUALS("f{([=", testAst("f = []() -> foo&& {};"));
        ASSERT_EQUALS("f{([=", testAst("f = [](void) mutable -> foo* {};"));
        ASSERT_EQUALS("f{([=", testAst("f = []() mutable {};"));

        ASSERT_EQUALS("x{([= 0return", testAst("x = [](){return 0; };"));

        ASSERT_EQUALS("ab{&[(= cd=", testAst("a = b([&]{c=d;});"));

        // 8628
        ASSERT_EQUALS("f{([( switchx( 1case y++", testAst("f([](){switch(x){case 1:{++y;}}});"));

        ASSERT_EQUALS("{(=[{return ab=",
                      testAst("return {\n"
                              "  [=]() {\n"
                              "    a = b;\n"
                              "  }\n"
                              "};\n"));
        ASSERT_EQUALS("{=[{return ab=",
                      testAst("return {\n"
                              "  [=] {\n"
                              "    a = b;\n"
                              "  }\n"
                              "};\n"));
        ASSERT_EQUALS("{(=[{return ab=",
                      testAst("return {\n"
                              "  [=]() -> int {\n"
                              "    a=b;\n"
                              "  }\n"
                              "}"));
        ASSERT_EQUALS("{(=[{return ab=",
                      testAst("return {\n"
                              "  [=]() mutable consteval -> int {\n"
                              "    a=b;\n"
                              "  }\n"
                              "}"));

        // daca@home hang
        ASSERT_EQUALS("a{(&[= 0return b{(=[= fori0=i10!=i++;;(",
                      testAst("a = [&]() -> std::pair<int, int> { return 0; };\n"
                              "b = [=]() { for (i = 0; i != 10; ++i); };"));

        // #9662
        ASSERT_EQUALS("b{[{ stdunique_ptr::0nullptrnullptr:?{", testAst("auto b{[] { std::unique_ptr<void *>{0 ? nullptr : nullptr}; }};"));
        ASSERT_EQUALS("{b{[=[", testAst("void a() { [b = [] { ; }] {}; }"));

        // Lambda capture expression (C++14)
        ASSERT_EQUALS("a{b1=[= c2=", testAst("a = [b=1]{c=2;};"));

        // #9729
        ASSERT_NO_THROW(tokenizeAndStringify("void foo() { bar([]() noexcept { if (0) {} }); }"));
        ASSERT_EQUALS("", errout_str());

        // #11128
        const Settings s = settingsBuilder(settings1).cpp(Standards::CPP17).build();
        ASSERT_NO_THROW(tokenizeAndStringify("template <typename T>\n"
                                             "struct S;\n"
                                             "struct R;\n"
                                             "S<R> y, z;\n"
                                             "auto f(int x) -> S<R> {\n"
                                             "    if (const auto i = x; i != 0)\n"
                                             "        return y;\n"
                                             "    else\n"
                                             "        return z;\n"
                                             "}\n", s));
        ignore_errout();

        // #10079 - createInnerAST bug..
        ASSERT_EQUALS("x{([= yz= switchy(",
                      testAst("x = []() -> std::vector<uint8_t> {\n"
                              "    const auto y = z;\n"
                              "    switch (y) {}\n"
                              "};"));

        // #11357
        ASSERT_NO_THROW(tokenizeAndStringify("void f(std::vector<int>& v, bool c) {\n"
                                             "    std::sort(v.begin(), v.end(), [&c](const auto a, const auto b) {\n"
                                             "        switch (c) {\n"
                                             "        case false: {\n"
                                             "            if (a < b) {}\n"
                                             "        }\n"
                                             "        }\n"
                                             "        return a < b;\n"
                                             "    });\n"
                                             "}\n"));
        ignore_errout();

        ASSERT_NO_THROW(tokenizeAndStringify("namespace N {\n"
                                             "    enum E : bool { F };\n"
                                             "}\n"
                                             "void f(std::vector<int>& v, bool c) {\n"
                                             "    std::sort(v.begin(), v.end(), [&c](const auto a, const auto b) {\n"
                                             "        switch (c) {\n"
                                             "        case N::E::F: {\n"
                                             "            if (a < b) {}\n"
                                             "        }\n"
                                             "        }\n"
                                             "        return a < b;\n"
                                             "    });\n"
                                             "}\n"));
        ignore_errout();

        ASSERT_NO_THROW(tokenizeAndStringify("void f(const std::vector<char>& v) {\n"
                                             "    std::for_each(v.begin(), v.end(), [&](char c) {\n"
                                             "        switch (c) {\n"
                                             "            case 'r': {\n"
                                             "                if (c) {}\n"
                                             "            }\n"
                                             "            break;\n"
                                             "        }\n"
                                             "    });\n"
                                             "}\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify("struct A { A(int) {} };\n"
                                             "void g(void (*)(int));\n"
                                             "void f() {\n"
                                             "    g([](int i) {\n"
                                             "        switch (i) {\n"
                                             "        case static_cast<int>(1): {\n"
                                             "            A a(i);\n"
                                             "            if (1) {}\n"
                                             "        }\n"
                                             "        }\n"
                                             "    });\n"
                                             "}\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify("int foo() {\n"
                                             "    connect([]( const int& f ) {\n"
                                             "                switch( f )\n"
                                             "                {\n"
                                             "                    case (int)1:\n"
                                             "                    {\n"
                                             "                        A r(f);\n"
                                             "                        if (1) {}\n"
                                             "                    }\n"
                                             "                    break;\n"
                                             "                }\n"
                                             "                return 0;\n"
                                             "             });\n"
                                             "    return 0;\n"
                                             "}\n"));
        ASSERT_EQUALS("", errout_str());

        // #11378
        ASSERT_EQUALS("gT{(&[{= 0return", testAst("auto g = T{ [&]() noexcept -> int { return 0; } };"));

        ASSERT_EQUALS("sf.{(i[{={", testAst("void g(int i) { S s{ .f = { [i]() {} } }; }"));

        ASSERT_EQUALS("{([", testAst("void f() { []() {}; }")); // #13471
    }

    void astcase() {
        ASSERT_EQUALS("0case", testAst("case 0:"));
        ASSERT_EQUALS("12+case", testAst("case 1+2:"));
        ASSERT_EQUALS("xyz:?case", testAst("case (x?y:z):"));
        ASSERT_EQUALS("switchx( 1case y++ 2case", testAst("switch(x){case 1:{++y;break;case 2:break;}}"));
        ASSERT_EQUALS("switchi( 12<<~case 0return", testAst("switch (i) { case ~(1 << 2) : return 0; }")); // #13197
    }

    void astrefqualifier() {
        ASSERT_EQUALS("", testAst("class a { auto b() -> int&; };"));
        ASSERT_EQUALS("", testAst("class a { auto b() -> int&&; };"));
        ASSERT_EQUALS("", testAst("class a { void b() &&; };"));
        ASSERT_EQUALS("", testAst("class a { void b() &; };"));
        ASSERT_EQUALS("", testAst("class a { void b() && = delete; };"));
        ASSERT_EQUALS("", testAst("class a { void b() & = delete; };"));
        ASSERT_EQUALS("", testAst("class a { void b() && {} };"));
        ASSERT_EQUALS("", testAst("class a { void b() & {} };"));
    }

    void astthrowdelete() {
        ASSERT_EQUALS("a(", testAst("class a { virtual ~a() throw() = delete; };"));
    }

    void asttrailingdecltype() {
        ASSERT_EQUALS("Cc& csize.(", testAst("template<class C> constexpr auto s(const C &c) noexcept -> decltype(c.size()) {}"));
        ASSERT_EQUALS("Cc& MakeSpancdata.(csize.(,(",
                      testAst("template <typename C> constexpr auto MakeSpan(C &c) -> decltype(MakeSpan(c.data(), c.size())) {}"));
        ASSERT_EQUALS("Eqeq&key_typek&, eqkk,(", testAst("auto KeyTypeCanBeEq(const Eq& eq, const key_type& k) -> decltype(eq(k, k));"));
        ASSERT_EQUALS("h{([= si. {return", testAst("auto h = []() -> decltype(s.i) { return {}; };"));
    }
    void astnoexcept() {
        ASSERT_EQUALS("noexceptaswap.b((", testAst("void f() noexcept(noexcept(a.swap(b))) {}"));
        ASSERT_EQUALS("{([ noexceptaswap.b((", testAst("[]() noexcept(noexcept(a.swap(b))) {}"));
    }

    //Verify that returning a newly constructed object generates the correct AST even when the class name is scoped
    //Addresses https://trac.cppcheck.net/ticket/9700
    void astnewscoped() {
        ASSERT_EQUALS("(return (new A))", testAst("return new A;", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( A)))", testAst("return new A();", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( A true)))", testAst("return new A(true);", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (:: A B)))", testAst("return new A::B;", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( (:: A B))))", testAst("return new A::B();", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( (:: A B) true)))", testAst("return new A::B(true);", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (:: (:: A B) C)))", testAst("return new A::B::C;", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( (:: (:: A B) C))))", testAst("return new A::B::C();", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( (:: (:: A B) C) true)))", testAst("return new A::B::C(true);", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (:: (:: (:: A B) C) D)))", testAst("return new A::B::C::D;", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( (:: (:: (:: A B) C) D))))", testAst("return new A::B::C::D();", AstStyle::Z3));
        ASSERT_EQUALS("(return (new (( (:: (:: (:: A B) C) D) true)))", testAst("return new A::B::C::D(true);", AstStyle::Z3));
    }

    void astdecltypescope() {
        ASSERT_EQUALS("sizedecltypethism_P.(XSize::::{", testAst("size { decltype(this->m_P)::X::Size };"));
    }

    void astdesignatedinit() {
        ASSERT_EQUALS("(( f ({ (= (. x) 1)))", testAst("f({ .x = 1 });", AstStyle::Z3));
    }

    void astrvaluedecl() {
        ASSERT_EQUALS("varstdmove::var(=", testAst("std::string&& var = std::move(var);"));
    }

    void astorkeyword() {
        ASSERT_EQUALS("ifsp.\"\"==sp.0==||(", testAst("void f() { if (s.p == \"\" or s.p == 0) {} }"));
    }

    void astenumdecl() {
        ASSERT_EQUALS("A0U=", testAst("enum class myclass : unsigned char { A = 0U, };"));
        ASSERT_EQUALS("A0U=", testAst("enum myclass : unsigned char { A = 0U, };"));
    }

    void astcompound() {
        ASSERT_EQUALS("sn0=={(tmp:?=", testAst("Str s = n == 0 ? (Str) { 0 } : tmp;")); // #14548
        ASSERT_EQUALS("s(sstrlens(0:?,{(return", testAst("return (struct Str) { (unsigned char*)s, s ? strlen(s) : 0 };"));
    }

    void astfuncdecl() {
        ASSERT_EQUALS("", testAst("bool operator==(const S& a, const S& b);", AstStyle::Simple, ListSimplification::Full));
        ASSERT_EQUALS("", testAst("::int32_t f();"));
    }

#define isStartOfExecutableScope(offset, code) isStartOfExecutableScope_(offset, code, __FILE__, __LINE__)
    template<size_t size>
    bool isStartOfExecutableScope_(int offset, const char (&code)[size], const char* file, int line) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        return Tokenizer::startOfExecutableScope(tokenizer.tokens()->tokAt(offset)) != nullptr;
    }

    void startOfExecutableScope() {
        ASSERT(isStartOfExecutableScope(3, "void foo() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() volatile { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() override { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() noexcept { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() NOEXCEPT { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() CONST NOEXCEPT { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const noexcept { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() noexcept(true) { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const noexcept(true) { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() throw() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() THROW() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() CONST THROW() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const throw() { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() throw(int) { }"));
        ASSERT(isStartOfExecutableScope(3, "void foo() const throw(int) { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a(1) { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a(1), b(2) { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a{1} { }"));
        ASSERT(isStartOfExecutableScope(2, "foo() : a{1}, b{2} { }"));
    }

    void removeMacroInClassDef() { // #6058
        ASSERT_EQUALS("class Fred { } ;", tokenizeAndStringify("class DLLEXPORT Fred { } ;"));
        ASSERT_EQUALS("class Fred : Base { } ;", tokenizeAndStringify("class Fred FINAL : Base { } ;"));
        ASSERT_EQUALS("class Fred : Base { } ;", tokenizeAndStringify("class DLLEXPORT Fred final : Base { } ;")); // #11422
        // Regression for C code:
        ASSERT_EQUALS("struct Fred { } ;", tokenizeAndStringify("struct DLLEXPORT Fred { } ;", dinit(TokenizeOptions, $.cpp = false)));
    }

    void sizeofAddParentheses() {
        ASSERT_EQUALS("sizeof ( sizeof ( 1 ) ) ;", tokenizeAndStringify("sizeof sizeof 1;"));
        ASSERT_EQUALS("sizeof ( a . b ) + 3 ;", tokenizeAndStringify("sizeof a.b+3;"));
        ASSERT_EQUALS("sizeof ( a [ 2 ] . b ) + 3 ;", tokenizeAndStringify("sizeof a[2].b+3;"));
        ASSERT_EQUALS("f ( 0 , sizeof ( ptr . bar ) ) ;", tokenizeAndStringify("f(0, sizeof ptr->bar );"));
        ASSERT_EQUALS("sizeof ( a ) > sizeof ( & main ) ;", tokenizeAndStringify("sizeof a > sizeof &main;"));
    }

    void reportUnknownMacros() {
        const char code1[] = "MY_UNKNOWN_IMP1(IInStream)\n"
                             "STDMETHOD(Read)(void *data, UInt32 size, UInt32 *processedSize) { if (ptr); }";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code1), UNKNOWN_MACRO);

        const char code2[] = "void foo() { dostuff(x 0); }";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code2), UNKNOWN_MACRO);

        const char code3[] = "f(\"1\" __stringify(48) \"1\");";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code3), UNKNOWN_MACRO);

        const char code4[] = "struct Foo {\n"
                             "  virtual MACRO(int) f1() {}\n"
                             "  virtual MACRO(int) f2() {}\n"
                             "};";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code4), UNKNOWN_MACRO);

        const char code5[] = "void foo() {\n"
                             "  EVALUATE(123, int x=a; int y=b+c;);\n"
                             "}";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code5), UNKNOWN_MACRO);

        const char code6[] = "void foo() { dostuff(a, .x=0); }";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code6), UNKNOWN_MACRO);

        const char code7[] = "void foo() { dostuff(ZEND_NUM_ARGS() TSRMLS_CC, x, y); }"; // #9476
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code7), UNKNOWN_MACRO);

        const char code8[] = "void foo() { a = [](int x, decltype(vec) y){}; }";
        ASSERT_NO_THROW(tokenizeAndStringify(code8));

        const char code9[] = "void f(std::exception c) { b(M() c.what()); }";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code9), UNKNOWN_MACRO);

        const char code10[] = "void f(std::exception c) { b(M() M() + N(c.what())); }";
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code10), UNKNOWN_MACRO);

        const char code11[] = "struct B { B(B&&) noexcept {} ~B() noexcept {} };";
        ASSERT_NO_THROW(tokenizeAndStringify(code11));

        ASSERT_NO_THROW(tokenizeAndStringify("alignas(8) alignas(16) int x;")); // alignas is not unknown macro

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void foo() { if(x) SYSTEM_ERROR }"), UNKNOWN_MACRO);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void foo() { dostuff(); SYSTEM_ERROR }"), UNKNOWN_MACRO);

        ASSERT_NO_THROW(tokenizeAndStringify("void f(void* q) {\n"
                                             "    g(&(S) { .p = (int*)q });\n"
                                             "}\n", dinit(TokenizeOptions, $.cpp = false)));

        ASSERT_NO_THROW(tokenizeAndStringify("typedef struct { int i; } S;\n"
                                             "void f(float a) {\n"
                                             "S s = (S){ .i = (int)a };\n"
                                             "}\n", dinit(TokenizeOptions, $.cpp = false)));

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("std::string g();\n"
                                                   "std::string f() {\n"
                                                   "    return std::string{ g() + \"abc\" MACRO \"def\" };\n"
                                                   "}\n"), UNKNOWN_MACRO);

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("static void handle_toggle(void (*proc) PROTO_XT_CALLBACK_ARGS, int var) {}\n"), // #13198
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If PROTO_XT_CALLBACK_ARGS is a macro then please configure it.");

        ignore_errout();
    }

    void findGarbageCode() { // Test Tokenizer::findGarbageCode()
        // C++ try/catch in global scope
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("try { }"), SYNTAX, "syntax error: keyword 'try' is not allowed in global scope");
        ASSERT_NO_THROW(tokenizeAndStringify("void f() try { } catch (int) { }"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct S {\n" // #9716
                                             "    S();\n"
                                             "    int x, y;\n"
                                             "};\n"
                                             "S::S()\n"
                                             "    try : x(1), y{ 2 } { f(); }\n"
                                             "    catch (const std::exception& e) { g(); }\n"
                                             "    catch (...) { g(); }\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("void f()\n"
                                             "    try { g(); }\n"
                                             "    catch (const std::exception& e) { h(); }\n"
                                             "    catch (...) { h(); }\n"));

        // before if|for|while|switch
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { do switch (a) {} while (1); }"));
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { label: switch (a) {} }"));
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { UNKNOWN_MACRO if (a) {} }"));
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { []() -> int * {}; }"));
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { const char* var = \"1\" \"2\"; }"));

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void f() { MACRO(switch); }"), UNKNOWN_MACRO);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void f() { MACRO(x,switch); }"), UNKNOWN_MACRO);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void foo() { for_chain( if (!done) done = 1); }"), UNKNOWN_MACRO);
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void foo() { for_chain( a, b, if (!done) done = 1); }"), UNKNOWN_MACRO);

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { if (retval==){} }"), SYNTAX, "syntax error: ==)");

        // after (expr)
        ASSERT_NO_THROW(tokenizeAndStringify("void f() { switch (a) int b; }"));

        ASSERT_NO_THROW(tokenizeAndStringify("S s = { .x=2, .y[0]=3 };"));
        ASSERT_NO_THROW(tokenizeAndStringify("S s = { .ab.a=2, .ab.b=3 };"));

        ASSERT_NO_THROW(tokenizeAndStringify("extern \"C\" typedef void FUNC();"));

        // Ticket #9572
        ASSERT_NO_THROW(tokenizeAndStringify("struct poc { "
                                             "  struct { int d; } port[1]; "
                                             "}; "
                                             "struct poc p = { .port[0] = {.d = 3} };"));

        // Ticket #9664
        ASSERT_NO_THROW(tokenizeAndStringify("S s = { .x { 2 }, .y[0] { 3 } };"));

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("f(0, .x());"), SYNTAX); // #12823

        // Ticket #11134
        ASSERT_NO_THROW(tokenizeAndStringify("struct my_struct { int x; }; "
                                             "std::string s; "
                                             "func(my_struct{ .x=42 }, s.size());"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct my_struct { int x; int y; }; "
                                             "std::string s; "
                                             "func(my_struct{ .x{42}, .y=3 }, s.size());"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct my_struct { int x; int y; }; "
                                             "std::string s; "
                                             "func(my_struct{ .x=42, .y{3} }, s.size());"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct my_struct { int x; }; "
                                             "void h() { "
                                             "  for (my_struct ms : { my_struct{ .x=5 } }) {} "
                                             "}"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct my_struct { int x; int y; }; "
                                             "void h() { "
                                             "  for (my_struct ms : { my_struct{ .x=5, .y{42} } }) {} "
                                             "}"));

        ASSERT_NO_THROW(tokenizeAndStringify("template <typename T> void foo() {} "
                                             "void h() { "
                                             "  [func=foo<int>]{func();}(); "
                                             "}"));
        ASSERT_NO_THROW(tokenizeAndStringify("template <class T> constexpr int n = 1;\n"
                                             "template <class T> T a[n<T>];\n"));

        ASSERT_EQUALS("std :: vector < int > x ;", // #11785
                      tokenizeAndStringify("std::vector<int> typedef v; v x;\n"));


        // op op
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { dostuff (x==>y); }"), SYNTAX, "syntax error: == >");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { assert(a==()); }"), SYNTAX, "syntax error: ==()");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { assert(a+()); }"), SYNTAX, "syntax error: +()");

        // #9445 - typeof is not a keyword in C
        ASSERT_NO_THROW(tokenizeAndStringify("void foo() { char *typeof, *value; }", dinit(TokenizeOptions, $.expand = false, $.cpp = false)));

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("enum : { };"), SYNTAX, "syntax error: Unexpected token '{'");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("enum : 3 { };"), SYNTAX, "syntax error: Unexpected token '3'");
        ASSERT_NO_THROW(tokenizeAndStringify("enum { E = int{} };"));

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("int a() { b((c)return 0) }"), SYNTAX, "syntax error");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("int f() { MACRO(x) return 0; }"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If MACRO is a macro then please configure it.");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("bool f() { unknown return 0; }"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If unknown is a macro then please configure it.");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f(int i) {\n" // #11770
                                                          "    if (i == 0) {}\n"
                                                          "    else if (i == 1) {}\n"
                                                          "    else\n"
                                                          "        MACRO(i)\n"
                                                          "}\n"
                                                          "void g() {}\n"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If MACRO is a macro then please configure it.");
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int i) {\n"
                                             "    if (i == 0) {}\n"
                                             "    else if (i == 1) {}\n"
                                             "    else\n"
                                             "        MACRO(i);\n"
                                             "}\n"
                                             "void g() {}\n"));

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("class C : public QObject {\n" // #11770
                                                          "    struct S { static void g() {} };\n"
                                                          "private Q_SLOTS:\n"
                                                          "    void f() { S::g(); }\n"
                                                          "};\n"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If Q_SLOTS is a macro then please configure it.");
        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("class C : public QObject {\n"
                                                          "    struct S { static void g() {} };\n"
                                                          "private slots:\n"
                                                          "    void f() { S::g(); }\n"
                                                          "};\n"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If slots is a macro then please configure it.");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("namespace U_ICU_ENTRY_POINT_RENAME(icu) { }\n"
                                                          "namespace icu = U_ICU_ENTRY_POINT_RENAME(icu);\n"
                                                          "namespace U_ICU_ENTRY_POINT_RENAME(icu) {\n"
                                                          "    class BreakIterator;\n"
                                                          "}\n"
                                                          "typedef int UStringCaseMapper(icu::BreakIterator* iter);\n"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If U_ICU_ENTRY_POINT_RENAME is a macro then please configure it.");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("void f() { MACRO(x(), y(), \"abc\", z(); ok = true); }\n"), // #12006
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If MACRO is a macro then please configure it.");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("int (*f) MACRO((void *));\n"), // #12010
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If MACRO is a macro then please configure it.");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("struct S { int a[2] PACKED; };\n"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If PACKED is a macro then please configure it.");

        ASSERT_THROW_INTERNAL_EQUALS(tokenizeAndStringify("MACRO(a, b,,)\n"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If MACRO is a macro then please configure it.");

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("{ for (()()) }"), SYNTAX); // #11643

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void f(const std::vector<std::string>& v) {\n" // #14326
                                                   "    for (const std::string&s : v)\n"
                                                   "}"),
                              SYNTAX);

        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void f(T* p) {\n"
                                                   "    g(p->x TSRMLS_CC);"
                                                   "}"),
                              UNKNOWN_MACRO);

        ASSERT_NO_THROW(tokenizeAndStringify("S* g = ::new(ptr) S();")); // #12552
        ASSERT_NO_THROW(tokenizeAndStringify("void f(int* p) { return ::delete p; }"));

        ASSERT_NO_THROW(tokenizeAndStringify("template <typename T, int N>\n" // #12659
                                             "constexpr void f(T(&&a)[N]) {}"));

        ASSERT_NO_THROW(tokenizeAndStringify("typedef struct { typedef int T; } S;")); // #12700

        ASSERT_NO_THROW(tokenizeAndStringify("class A { bool restrict() const; };\n"
                                             "bool A::restrict() const { return true; }")); // #12718

        ASSERT_NO_THROW(tokenizeAndStringify("enum { E = sizeof(struct { int i; }) };")); // #13249

        ASSERT_NO_THROW(tokenizeAndStringify("struct S { unsigned u:2, :30; };")); // #14393

        ASSERT_NO_THROW(tokenizeAndStringify("struct S {};\n" // #14400
                                             "auto f(bool b) -> struct S {\n"
                                             "    if (b) {}\n"
                                             "    return {};\n"
                                             "};\n"));

        ASSERT_NO_THROW(tokenizeAndStringify("void f() {\n" // #14395
                                             "    for (int i : [](int a, int b) { ++a; ++b; return std::vector<int>{a, b}; }(1, 2)) {}\n"
                                             "}\n"));

        ignore_errout();
    }


    void checkEnableIf() {
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template<\n"
                            "    typename U,\n"
                            "    typename std::enable_if<\n"
                            "        std::is_convertible<U, T>{}>::type* = nullptr>\n"
                            "void foo(U x);\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template<class t>\n"
                            "T f(const T a, const T b) {\n"
                            "    return a < b ? b : a;\n"
                            "}\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template<class T>\n"
                            "struct A {\n"
                            "    T f(const T a, const T b) {\n"
                            "        return a < b ? b : a;\n"
                            "    }\n"
                            "};\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "const int a = 1;\n"
                            "const int b = 2;\n"
                            "template<class T>\n"
                            "struct A {\n"
                            "    int x = a < b ? b : a;"
                            "};\n"));
        ASSERT_EQUALS("", errout_str());

        // #10139
        ASSERT_NO_THROW(tokenizeAndStringify("template<typename F>\n"
                                             "void foo(std::enable_if_t<value<F>>* = 0) {}\n"));
        ASSERT_EQUALS("", errout_str());

        // #10001
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  int c;\n"
                                             "  template <class b> void d(b e) const { c < e ? c : e; }\n"
                                             "};\n"));
        ASSERT_EQUALS("", errout_str());

        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  int c;\n"
                                             "  template <class b> void d(b e) const { c > e ? c : e; }\n"
                                             "};\n"));
        ASSERT_EQUALS("", errout_str());
    }

    void checkTemplates() {
        // #9109
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "namespace {\n"
                            "template <typename> struct a;\n"
                            "template <typename> struct b {};\n"
                            "}\n"
                            "namespace {\n"
                            "template <typename> struct c;\n"
                            "template <typename d> struct e {\n"
                            "  using f = a< b<typename c<d>::g> >;\n"
                            "  bool h = f::h;\n"
                            "};\n"
                            "template <typename i> using j = typename e<i>::g;\n"
                            "}\n"));

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template <typename = void> struct a {\n"
                            "  void c();\n"
                            "};\n"
                            "void f() {\n"
                            "  a<> b;\n"
                            "  b.a<>::c();\n"
                            "}\n"));

        // #9138
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template <typename> struct a;\n"
                            "template <bool> using c = int;\n"
                            "template <bool b> c<b> d;\n"
                            "template <> struct a<int> {\n"
                            "template <typename e> constexpr auto g() { d<0 || e::f>; return 0; }\n"
                            "};\n"));

        // #9144
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "namespace a {\n"
                            "template <typename b, bool = __is_empty(b) && __is_final(b)> struct c;\n"
                            "}\n"
                            "namespace boost {\n"
                            "using a::c;\n"
                            "}\n"
                            "namespace d = boost;\n"
                            "using d::c;\n"
                            "template <typename...> struct e {};\n"
                            "static_assert(sizeof(e<>) == sizeof(e<c<int>, c<int>, int>), \"\");\n"));

        // #9146
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template <int> struct a;\n"
                            "template <class, class b> using c = typename a<int{b::d}>::e;\n"
                            "template <class> struct f;\n"
                            "template <class b> using g = typename f<c<int, b>>::e;\n"));

        // #9153
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "namespace {\n"
                            "template <class> struct a;\n"
                            "}\n"
                            "namespace {\n"
                            "namespace b {\n"
                            "template <int c> struct B { using B<c / 2>::d; };\n"
                            "}\n"
                            "template <class, class> using e = typename b::B<int{}>;\n"
                            "namespace b {\n"
                            "template <class> struct f;\n"
                            "}\n"
                            "template <class c> using g = b::f<e<int, c>>;\n"
                            "}\n"));

        // #9154
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template <bool> using a = int;\n"
                            "template <class b> using aa = a<b::c>;\n"
                            "template <class...> struct A;\n"
                            "template <class> struct d;\n"
                            "template <class... f> using e = typename d<f...>::g;\n"
                            "template <class> struct h;\n"
                            "template <class, class... b> using i = typename h<b...>::g;\n"
                            "template <class f, template <class> class j> using k = typename f::g;\n"
                            "template <class... b> using l = a<k<A<b...>, aa>::c>;\n"
                            "template <int> struct m;\n"
                            "template <class, class n> using o = typename m<int{n::c}>::g;\n"
                            "template <class> struct p;\n"
                            "template <class, class n> using q = typename p<o<A<>, n>>::g;\n"
                            "template <class f, class r, class... b> using c = e<i<q<f, r>, b...>>;\n"
                            "template <class, class> struct s;\n"
                            "template <template <class> class t, class... w, template <class> class x,\n"
                            "          class... u>\n"
                            "struct s<t<w...>, x<u...>>;\n"));

        // #9156
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template <typename> struct a;\n"
                            "template <bool> struct b;\n"
                            "template <class k, class> using d = typename b<k::c>::e;\n"
                            "template <class> struct f;\n"
                            "template <template <class> class, class... g> using i = typename f<g...>::e;\n"
                            "template <template <class> class h, class... g> using ab = d<i<h, g...>, int>;\n"
                            "template <template <class> class h, class... g> struct j {\n"
                            "  template <class... ag> using ah = typename ab<h, ag..., g...>::e;\n"
                            "};\n"
                            "template <class> struct F;\n"
                            "int main() { using T = void (*)(a<j<F, char[]>>); }\n"));

        // #9245
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  typedef int b;\n"
                                             "  operator b();\n"
                                             "};\n"
                                             "template <int> using c = a;\n"
                                             "template <int d> c<d> e;\n"
                                             "auto f = ((e<4> | 0));\n"));

        // #9340
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "struct a {\n"
                            "  template <class... b> void c(b... p1) {\n"
                            "    using d = a;\n"
                            "    d e = {(p1)...};\n"
                            "  }\n"
                            "};\n"));

        // #9444
        ASSERT_NO_THROW(tokenizeAndStringify("template <int> struct a;\n"
                                             "template <long b> using c = a<b>;\n"
                                             "template <long b> c<b> d;\n"
                                             "template <typename> struct e {\n"
                                             "  template <typename... f> void g() const { d<e<f &&...>::h>; }\n"
                                             "};\n"));

        // #9858
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "struct a {\n"
                            "  struct b {};\n"
                            "};\n"
                            "void c(a::b, a::b);\n"
                            "void g(a::b f) { c(f, {a::b{}}); }\n"
                            "template <class> void h() {\n"
                            "  int e;\n"
                            "  for (int d = 0; d < e; d++)\n"
                            "    ;\n"
                            "}\n"));

        // #10015
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "void func() {\n"
                            "    if (std::is_same_v<int, int> || 1)\n"
                            "        ;\n"
                            "}\n"));

        // #10309
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "using a = void *;\n"
                            "void b() {\n"
                            "  std::unique_ptr<a, void (*)(a *)>(new a(0), [](a *c) {\n"
                            "    if (c)\n"
                            "      ;\n"
                            "  });\n"
                            "}\n"));

        ASSERT_NO_THROW(tokenizeAndStringify("a<b?0:1>()==3;"));

        // #10336
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  template <class b> a(b);\n"
                                             "};\n"
                                             "struct c;\n"
                                             "void fn1(int, a);\n"
                                             "void f() { fn1(0, {a{0}}); }\n"
                                             "template <class> std::vector<c> g() {\n"
                                             "  int d;\n"
                                             "  for (size_t e = 0; e < d; e++)\n"
                                             "    ;\n"
                                             "}\n"));

        // #9523
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template <int> struct a;\n"
                            "template <typename, typename> struct b;\n"
                            "template <typename c> struct b<c, typename a<c{} && 0>::d> {\n"
                            "  void e() {\n"
                            "    if (0) {}\n"
                            "  }\n"
                            "};\n"));

        ASSERT_NO_THROW(tokenizeAndStringify(
                            "template <std::size_t First, std::size_t... Indices, typename Functor>\n"
                            "constexpr void constexpr_for_fold_impl([[maybe_unused]] Functor&& f, std::index_sequence<Indices...>) noexcept {\n"
                            "    (std::forward<Functor>(f).template operator() < First + Indices > (), ...);\n"
                            "}\n"));

        // #9301
        ASSERT_NO_THROW(tokenizeAndStringify("template <typename> constexpr char x[] = \"\";\n"
                                             "template <> constexpr char x<int>[] = \"\";\n"));

        // #10951
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  template <class> static void b() {}\n"
                                             "  ~a();\n"
                                             "};\n"
                                             "void d() { a::b<int>(); }\n"));

        // #11090
        ASSERT_NO_THROW(tokenizeAndStringify("using a = char;\n"
                                             "using c = int;\n"
                                             "template <typename = void> struct d {};\n"
                                             "using b = c;\n"
                                             "template <> struct d<b> : d<a> {};\n"
                                             "template <> struct d<> : d<a> {};\n"));

        ignore_errout();
    }

    void checkNamespaces() {
        ASSERT_NO_THROW(tokenizeAndStringify("namespace x { namespace y { namespace z {}}}"));
    }

    void checkLambdas() {
        ASSERT_NO_THROW(tokenizeAndStringify("auto f(int& i) { return [=, &i] {}; }"));
        ASSERT_NO_THROW(tokenizeAndStringify("auto f(int& i) { return [&, i] {}; }"));
        ASSERT_NO_THROW(tokenizeAndStringify("auto f(int& i) { return [&, i = std::move(i)] {}; }"));
        ASSERT_NO_THROW(tokenizeAndStringify("auto f(int& i) { return [=, i = std::move(i)] {}; }"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct c {\n"
                                             "  void d() {\n"
                                             "    int a;\n"
                                             "    auto b = [this, a] {};\n"
                                             "  }\n"
                                             "};\n"));

        // #9525
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  template <class b> a(b) {}\n"
                                             "};\n"
                                             "auto c() -> a {\n"
                                             "  return {[] {\n"
                                             "    if (0) {}\n"
                                             "  }};\n"
                                             "}\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  template <class b> a(b) {}\n"
                                             "};\n"
                                             "auto c() -> a {\n"
                                             "  return {[]() -> int {\n"
                                             "    if (0) {}\n"
                                             "    return 0;\n"
                                             "  }};\n"
                                             "}\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  template <class b> a(b) {}\n"
                                             "};\n"
                                             "auto c() -> a {\n"
                                             "  return {[]() mutable -> int {\n"
                                             "    if (0) {}\n"
                                             "    return 0;\n"
                                             "  }};\n"
                                             "}\n"));
        // #0535
        ASSERT_NO_THROW(tokenizeAndStringify("template <typename, typename> struct a;\n"
                                             "template <typename, typename b> void c() {\n"
                                             "  ([]() -> decltype(0) {\n"
                                             "    if (a<b, decltype(0)>::d) {}\n"
                                             "  });\n"
                                             "}\n"));

        // #9563
        ASSERT_NO_THROW(tokenizeAndStringify("template <typename> struct a;\n"
                                             "template <typename b, typename... c> struct a<b(c...)> {\n"
                                             "  template <typename d> a(d);\n"
                                             "};\n"
                                             "void e(\n"
                                             "    int, a<void()> f = [] {});\n"));

        // #9644
        ASSERT_NO_THROW(tokenizeAndStringify("void a() {\n"
                                             "  char b[]{};\n"
                                             "  auto c = [](int d) {\n"
                                             "    for (char e = 0; d;) {}\n"
                                             "  };\n"
                                             "}\n"));
        // #9537
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  template <typename b> a(b) {}\n"
                                             "};\n"
                                             "a c{[] {\n"
                                             "  if (0) {}\n"
                                             "}};\n"));
        // #9185
        ASSERT_NO_THROW(tokenizeAndStringify("void a() {\n"
                                             "  [b = [] { ; }] {};\n"
                                             "}\n"));

        // #10739
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  std::vector<int> b;\n"
                                             "};\n"
                                             "void c() {\n"
                                             "  a bar;\n"
                                             "  (decltype(bar.b)::value_type){};\n"
                                             "}\n"));

        ASSERT_NO_THROW(tokenizeAndStringify("struct S { char c{}; };\n" // #11400
                                             "void takesFunc(auto f) {}\n"
                                             "int main() { \n"
                                             "    takesFunc([func = [](S s) { return s.c; }] {});\n"
                                             "}\n"));

        ASSERT_NO_THROW(tokenizeAndStringify("void f() {\n" // #14256
                                             "    int i = 0;\n"
                                             "    auto x = [i] mutable {};\n"
                                             "}\n"));

        ignore_errout();
    }
    void checkIfCppCast() {
        ASSERT_NO_THROW(tokenizeAndStringify("struct a {\n"
                                             "  int b();\n"
                                             "};\n"
                                             "struct c {\n"
                                             "  bool d() const;\n"
                                             "  a e;\n"
                                             "};\n"
                                             "bool c::d() const {\n"
                                             "  int f = 0;\n"
                                             "  if (!const_cast<a *>(&e)->b()) {}\n"
                                             "  return f;\n"
                                             "}\n"));
    }

    void checkRefQualifiers() {
        // #9511
        ASSERT_NO_THROW(tokenizeAndStringify("class a {\n"
                                             "  void b() && {\n"
                                             "    if (this) {}\n"
                                             "  }\n"
                                             "};\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("class a {\n"
                                             "  void b() & {\n"
                                             "    if (this) {}\n"
                                             "  }\n"
                                             "};\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("class a {\n"
                                             "  auto b() && -> void {\n"
                                             "    if (this) {}\n"
                                             "  }\n"
                                             "};\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("class a {\n"
                                             "  auto b() & -> void {\n"
                                             "    if (this) {}\n"
                                             "  }\n"
                                             "};\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("class a {\n"
                                             "  auto b(int& x) -> int& {\n"
                                             "    if (this) {}\n"
                                             "    return x;\n"
                                             "  }\n"
                                             "};\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("class a {\n"
                                             "  auto b(int& x) -> int&& {\n"
                                             "    if (this) {}\n"
                                             "    return x;\n"
                                             "  }\n"
                                             "};\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("class a {\n"
                                             "  auto b(int& x) && -> int& {\n"
                                             "    if (this) {}\n"
                                             "    return x;\n"
                                             "  }\n"
                                             "};\n"));
        // #9524
        ASSERT_NO_THROW(tokenizeAndStringify("auto f() -> int* {\n"
                                             "  if (0) {}\n"
                                             "  return 0;\n"
                                             "};\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("auto f() -> int** {\n"
                                             "  if (0) {}\n"
                                             "  return 0;\n"
                                             "};\n"));
        ignore_errout();
    }

    void checkConditionBlock() {
        ASSERT_NO_THROW(tokenizeAndStringify("void a() {\n"
                                             "  for (auto b : std::vector<std::vector<int>>{{}, {}}) {}\n"
                                             "}\n"));
    }

    void checkUnknownCircularVar()
    {
        ASSERT_NO_THROW(tokenizeAndStringify("void execute() {\n"
                                             "    const auto &bias = GEMM_CTX_ARG_STORAGE(bias);\n"
                                             "    auto &c = GEMM_CTX_ARG_STORAGE(c);\n"
                                             "}\n"));
        ignore_errout(); // we do not care about the output
    }

    void checkRequires()
    {
        ASSERT_NO_THROW(tokenizeAndStringify("template<class T, class U>\n"
                                             "struct X { X(U) requires true {} };\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("template<class T, class U>\n"
                                             "struct X { X(U) requires bool{std::is_integral<T>{}} {} };\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("template <typename T>\n"
                                             "struct test { operator int() requires true { return 0; } };\n"));
        ASSERT_NO_THROW(tokenizeAndStringify("template <typename T>\n"
                                             "struct test { operator int() requires bool{std::is_integral<T>{}} { return 0; } };\n"));
    }

    void noCrash1() {
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "struct A {\n"
                            "  A( const std::string &name = \" \" );\n"
                            "};\n"
                            "A::A( const std::string &name ) { return; }\n"));
    }

    // #9007
    void noCrash2() {
        ASSERT_NO_THROW(tokenizeAndStringify(
                            "class a {\n"
                            "public:\n"
                            "  enum b {};\n"
                            "};\n"
                            "struct c;\n"
                            "template <class> class d {\n"
                            "  d(const int &, a::b, double, double);\n"
                            "  d(const d &);\n"
                            "};\n"
                            "template <> d<int>::d(const int &, a::b, double, double);\n"
                            "template <> d<int>::d(const d &) {}\n"
                            "template <> d<c>::d(const d &) {}\n"));
        ignore_errout(); // we do not care about the output
    }

    void noCrash3() {
        ASSERT_NO_THROW(tokenizeAndStringify("void a(X<int> x, typename Y1::Y2<int, A::B::C, 2> y, Z z = []{});"));
    }

    void noCrash4() {
        ASSERT_NO_THROW(tokenizeAndStringify("static int foo() {\n"
                                             "    zval ref ;\n"
                                             "    p = &(ref).value;\n"
                                             "    return result ;\n"
                                             "}\n"));
        ignore_errout();
    }

    void noCrash5() { // #10603
        ASSERT_NO_THROW(tokenizeAndStringify("class B { using shared_ptr = std::shared_ptr<Foo>; };\n"
                                             "class D : public B { void f(const std::shared_ptr<int>& ptr) {} };\n"));
    }

    void noCrash6() { // #10212
        ASSERT_NO_THROW(tokenizeAndStringify("template <long, long a = 0> struct b;\n"
                                             "template <class, bool> struct c;\n"
                                             "template <template <class, class> class a, class e, class... d>\n"
                                             "struct c<a<e, d...>, true> {};\n"));
    }

    void noCrash7() {
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("void g() {\n"// TODO: don't throw
                                                   "    for (using T = int; (T)false;) {}\n" // C++23 P2360R0: Extend init-statement to allow alias-declaration
                                                   "}\n"), SYNTAX);
    }

    template<size_t size>
    void checkConfig(const char (&code)[size]) {
        const Settings s = settingsBuilder().checkConfiguration().build();

        // tokenize..
        SimpleTokenizer tokenizer(s, *this);
        ASSERT(tokenizer.tokenize(code));
    }

    void checkConfiguration() {
        ASSERT_THROW_INTERNAL_EQUALS(checkConfig("void f() { DEBUG(x();y()); }"),
                                     UNKNOWN_MACRO,
                                     "There is an unknown macro here somewhere. Configuration is required. If DEBUG is a macro then please configure it.");
    }

    void unknownMacroBeforeReturn() {
        ASSERT_THROW_INTERNAL(tokenizeAndStringify("int f() { X return 0; }"), UNKNOWN_MACRO);
    }

    void cppKeywordInCSource() {
        ASSERT_NO_THROW(tokenizeAndStringify("int throw() {}", dinit(TokenizeOptions, $.cpp = false)));

        const char code[] = "void requires(const char*);\n" // #14613
                            "void f() { requires(\"abc\"); }\n";
        ASSERT_NO_THROW(tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        const Settings s_cpp17 = settingsBuilder().cpp(Standards::CPP17).build();
        ASSERT_NO_THROW(tokenizeAndStringify(code, s_cpp17, true));
        const Settings s_cpp20 = settingsBuilder().cpp(Standards::CPP20).build();
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code, s_cpp20, true), AST);
    }

    void cppcast() {
        const char code[] = "a = const_cast<int>(x);\n"
                            "a = dynamic_cast<int>(x);\n"
                            "a = reinterpret_cast<int>(x);\n"
                            "a = static_cast<int>(x);\n";

        SimpleTokenizer tokenizer(settingsDefault, *this);
        ASSERT(tokenizer.tokenize(code));

        for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next()) {
            ASSERT_EQUALS(tok->str() == "(", tok->isCast());
        }
    }

#define checkHdrs(...) checkHdrs_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    std::string checkHdrs_(const char* file, int line, const char (&code)[size], bool checkHeadersFlag) {
        const Settings settings = settingsBuilder().checkHeaders(checkHeadersFlag).build();

        SimpleTokenizer2 tokenizer(settings, *this, code, "test.cpp");

        // Tokenizer..
        ASSERT_LOC(tokenizer.simplifyTokens1(""), file, line);

        return tokenizer.tokens()->stringifyList();
    }

    void checkHeader1() {
        // #9977
        const char code[] = "# 1 \"test.h\"\n"
                            "struct A {\n"
                            "    int a = 1;\n"
                            "    void f() { g(1); }\n"
                            "    template <typename T> void g(T x) { a = 2; }\n" // <- template is used and should be kept
                            "};";

        ASSERT_EQUALS("\n\n##file 1\n"
                      "1: struct A {\n"
                      "2: int a ; a = 1 ;\n"
                      "3: void f ( ) { g<int> ( 1 ) ; }\n"
                      "4: void g<int> ( int x ) ;\n"
                      "5: } ;\n"
                      "4: void A :: g<int> ( int x ) { a = 2 ; }\n",
                      checkHdrs(code, true));

        ASSERT_EQUALS("\n\n##file 1\n\n"
                      "1:\n"
                      "|\n"
                      "4:\n"
                      "5: ;\n",
                      checkHdrs(code, false));
    }

    void removeExtraTemplateKeywords() {
        const char code1[] = "typename GridView::template Codim<0>::Iterator iterator;";
        const char expected1[] = "GridView :: Codim < 0 > :: Iterator iterator ;";
        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1));

        const char code2[] = "typename GridView::template Codim<0>::Iterator it = gv.template begin<0>();";
        const char expected2[] = "GridView :: Codim < 0 > :: Iterator it ; it = gv . begin < 0 > ( ) ;";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2));
    }

    void removeAlignas1() {
        const char code[] = "alignas(float) unsigned char c[sizeof(float)];";
        const char expected[] = "unsigned char c [ sizeof ( float ) ] ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void removeAlignas2() { // Do not remove alignas and alignof in the same way
        const char code[] = "static_assert( alignof( VertexC ) == 4 );";
        const char expected[] = "static_assert ( alignof ( VertexC ) == 4 ) ;";
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void removeAlignas3() {
        const char code[] = "alignas(16) int x;";
        const char expected[] = "int x ;";
        // According to cppreference alignas() is a C23 macro; but it is often available when compiling C11.
        // Misra C has C11 examples with alignas.
        // Microsoft provides alignas in C11.
        ASSERT_EQUALS(expected, tokenizeAndStringify(code, dinit(TokenizeOptions, $.cpp = false)));
        ASSERT_EQUALS(expected, tokenizeAndStringify(code));
    }

    void dumpAlignas() {
        SimpleTokenizer tokenizer(settingsDefault, *this, false);
        ASSERT(tokenizer.tokenize("int alignas(8) alignas(16) x;"));
        ASSERT(Token::simpleMatch(tokenizer.tokens(), "int x ;"));
        std::ostringstream ostr;
        tokenizer.dump(ostr);
        const std::string dump = ostr.str();
        ASSERT(dump.find(" alignas=\"8\" alignas2=\"16\"") != std::string::npos);
    }

    void simplifyCoroutines() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP20).build();

        const char code1[] = "generator<int> f() { co_yield start++; }";
        const char expected1[] = "generator < int > f ( ) { co_yield ( start ++ ) ; }";
        ASSERT_EQUALS(expected1, tokenizeAndStringify(code1, settings));

        const char code2[] = "task<> f() { co_await foo(); }";
        const char expected2[] = "task < > f ( ) { co_await ( foo ( ) ) ; }";
        ASSERT_EQUALS(expected2, tokenizeAndStringify(code2, settings));

        const char code3[] = "generator<int> f() { co_return 7; }";
        const char expected3[] = "generator < int > f ( ) { co_return ( 7 ) ; }";
        ASSERT_EQUALS(expected3, tokenizeAndStringify(code3, settings));
    }

    void simplifySpaceshipOperator() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP20).build();

        ASSERT_EQUALS("; x <=> y ;", tokenizeAndStringify(";x<=>y;", settings));
    }

    void simplifyIfSwitchForInit1() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP17).build();
        const char code[] = "void f() { if (a;b) {} }";
        ASSERT_EQUALS("void f ( ) { { a ; if ( b ) { } } }", tokenizeAndStringify(code, settings));
    }

    void simplifyIfSwitchForInit2() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP20).build();
        const char code[] = "void f() { if (a;b) {} else {} }";
        ASSERT_EQUALS("void f ( ) { { a ; if ( b ) { } else { } } }", tokenizeAndStringify(code, settings));
    }

    void simplifyIfSwitchForInit3() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP20).build();
        const char code[] = "void f() { switch (a;b) {} }";
        ASSERT_EQUALS("void f ( ) { { a ; switch ( b ) { } } }", tokenizeAndStringify(code, settings));
    }

    void simplifyIfSwitchForInit4() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP20).build();
        const char code[] = "void f() { for (a;b:c) {} }";
        ASSERT_EQUALS("void f ( ) { { a ; for ( b : c ) { } } }", tokenizeAndStringify(code, settings));
    }

    void simplifyIfSwitchForInit5() {
        const Settings settings = settingsBuilder().cpp(Standards::CPP20).build();
        const char code[] = "void f() { if ([] { ; }) {} }";
        ASSERT_EQUALS("void f ( ) { if ( [ ] { ; } ) { } }", tokenizeAndStringify(code, settings));
    }

    void newPlacementArgsCppInit() { // #13775
        const char code[] = "::new(nullptr) int {};";
        SimpleTokenizer tokenizer(settings1, *this);
        tokenizer.tokenize(code);
        const Token *inttok = Token::findsimplematch(tokenizer.tokens(), "int");
        ASSERT(inttok);
        const Token *brace = inttok->next();
        ASSERT(brace);
        ASSERT_EQUALS(brace->astOperand1(), inttok);
        ASSERT_EQUALS(brace->astOperand2(), static_cast<const Token*>(nullptr));
    }

    void cpp20_default_bitfield_initializer() {
        const Settings s1 = settingsBuilder().cpp(Standards::CPP20).build();
        const char code[] = "struct S { int a:2 = 0; };";
        ASSERT_EQUALS("struct S { int a ; a = 0 ; } ;", tokenizeAndStringify(code, s1));
        const Settings s2 = settingsBuilder().cpp(Standards::CPP17).build();
        ASSERT_THROW_INTERNAL(tokenizeAndStringify(code, s2), SYNTAX);
    }

    void cpp11init() {
        #define testIsCpp11init(...) testIsCpp11init_(__FILE__, __LINE__, __VA_ARGS__)
        auto testIsCpp11init_ = [this](const char* file, int line, const std::string& code, const char* find, Token::Cpp11init expected) {
            SimpleTokenizer tokenizer(settingsDefault, *this);
            ASSERT_LOC(tokenizer.tokenize(code.data(), code.size()), file, line);

            const Token* tok = Token::findsimplematch(tokenizer.tokens(), find, strlen(find));
            ASSERT_LOC(tok, file, line);
            ASSERT_LOC(tok->isCpp11init() == expected, file, line);
        };

        testIsCpp11init("class X : public A<int>, C::D {};",
                        "D {",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("auto f() -> void {}",
                        "void {",
                        Token::Cpp11init::NOINIT);
        testIsCpp11init("auto f() & -> void {}",
                        "void {",
                        Token::Cpp11init::NOINIT);
        testIsCpp11init("auto f() const noexcept(false) -> void {}",
                        "void {",
                        Token::Cpp11init::NOINIT);
        testIsCpp11init("auto f() -> std::vector<int> { return {}; }",
                        "{ return",
                        Token::Cpp11init::NOINIT);
        testIsCpp11init("auto f() -> std::vector<int> { return {}; }",
                        "vector",
                        Token::Cpp11init::NOINIT);
        testIsCpp11init("auto f() -> std::vector<int> { return {}; }",
                        "std ::",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("class X{};",
                        "{ }",
                        Token::Cpp11init::NOINIT);
        testIsCpp11init("class X{}", // forgotten ; so not properly recognized as a class
                        "{ }",
                        Token::Cpp11init::CPP11INIT);

        testIsCpp11init("namespace abc::def { TEST(a, b) {} }",
                        "{ TEST",
                        Token::Cpp11init::NOINIT);
        testIsCpp11init("namespace { TEST(a, b) {} }", // anonymous namespace
                        "{ TEST",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("enum { e = decltype(s)::i };",
                        "{ e",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("template <typename T>\n" // #11378
                        "class D<M<T, 1>> : public B<M<T, 1>, T> {\n"
                        "public:\n"
                        "    D(int x) : B<M<T, 1>, T>(x) {}\n"
                        "};\n",
                        "{ public:",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("template <typename T>\n"
                        "class D<M<T, 1>> : B<M<T, 1>, T> {\n"
                        "public:\n"
                        "    D(int x) : B<M<T, 1>, T>(x) {}\n"
                        "};\n",
                        "{ public:",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("using namespace std;\n"
                        "namespace internal {\n"
                        "    struct S { S(); };\n"
                        "}\n"
                        "namespace internal {\n"
                        "    S::S() {}\n"
                        "}\n",
                        "{ } }",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("template <std::size_t N>\n"
                        "struct C : public C<N - 1>, public B {\n"
                        "    ~C() {}\n"
                        "};\n",
                        "{ } }",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("struct S { int i; } s;\n"
                        "struct T : decltype (s) {\n"
                        "    T() : decltype(s) ({ 0 }) { }\n"
                        "};\n",
                        "{ } }",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("struct S {};\n"
                        "template<class... Args>\n"
                        "struct T;\n"
                        "template<class... Args>\n"
                        "struct T<void, Args...> final : S {\n"
                        "    void operator()(Args...) {}\n"
                        "};\n",
                        "{ void",
                        Token::Cpp11init::NOINIT);

        testIsCpp11init("struct S {\n"
                        "    std::uint8_t* p;\n"
                        "    S() : p{ new std::uint8_t[1]{} } {}\n"
                        "};\n",
                        "{ } } {",
                        Token::Cpp11init::CPP11INIT);

        testIsCpp11init("struct S {\n"
                        "    S() : p{new (malloc(4)) int{}} {}\n"
                        "    int* p;\n"
                        "};\n",
                        "{ } } {",
                        Token::Cpp11init::CPP11INIT);

        testIsCpp11init("void f() { g([]() {}, { 1 }); }\n",
                        "{ 1",
                        Token::Cpp11init::CPP11INIT);

        ASSERT_NO_THROW(tokenizeAndStringify("template<typename U> struct X {};\n" // don't crash
                                             "template<typename T> auto f(T t) -> X<decltype(t + 1)> {}\n"));
        ASSERT_EQUALS("[test.cpp:2:22]: (debug) auto token with no type. [autoNoType]\n", errout_str());
        #undef testIsCpp11init
    }

    void testDirectiveIncludeTypes() {
        const char filedata[] = "#define macro some definition\n"
                                "#undef macro\n"
                                "#ifdef macro\n"
                                "#elif some (complex) condition\n"
                                "#else\n"
                                "#endif\n"
                                "#if some other condition\n"
                                "#pragma some proprietary content\n"
                                "#\n" /* may appear in old C code */
                                "#ident some text\n" /* may appear in old C code */
                                "#unknownmacro some unpredictable text\n"
                                "#warning some warning message\n"
                                "#error some error message\n";
        const char dumpdata[] = "  <directivelist>\n"
                                "    <directive file=\"test.c\" linenr=\"1\" str=\"#define macro some definition\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"define\"/>\n"
                                "      <token column=\"9\" str=\"macro\"/>\n"
                                "      <token column=\"15\" str=\"some\"/>\n"
                                "      <token column=\"20\" str=\"definition\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"2\" str=\"#undef macro\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"undef\"/>\n"
                                "      <token column=\"8\" str=\"macro\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"3\" str=\"#ifdef macro\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"ifdef\"/>\n"
                                "      <token column=\"8\" str=\"macro\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"4\" str=\"#elif some (complex) condition\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"elif\"/>\n"
                                "      <token column=\"7\" str=\"some\"/>\n"
                                "      <token column=\"12\" str=\"(\"/>\n"
                                "      <token column=\"13\" str=\"complex\"/>\n"
                                "      <token column=\"20\" str=\")\"/>\n"
                                "      <token column=\"22\" str=\"condition\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"5\" str=\"#else\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"else\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"6\" str=\"#endif\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"endif\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"7\" str=\"#if some other condition\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"if\"/>\n"
                                "      <token column=\"5\" str=\"some\"/>\n"
                                "      <token column=\"10\" str=\"other\"/>\n"
                                "      <token column=\"16\" str=\"condition\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"8\" str=\"#pragma some proprietary content\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"pragma\"/>\n"
                                "      <token column=\"9\" str=\"some\"/>\n"
                                "      <token column=\"14\" str=\"proprietary\"/>\n"
                                "      <token column=\"26\" str=\"content\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"9\" str=\"#\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"10\" str=\"#ident some text\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"ident\"/>\n"
                                "      <token column=\"8\" str=\"some\"/>\n"
                                "      <token column=\"13\" str=\"text\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"11\" str=\"#unknownmacro some unpredictable text\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"unknownmacro\"/>\n"
                                "      <token column=\"15\" str=\"some\"/>\n"
                                "      <token column=\"20\" str=\"unpredictable\"/>\n"
                                "      <token column=\"34\" str=\"text\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"12\" str=\"#warning some warning message\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"warning\"/>\n"
                                "      <token column=\"10\" str=\"some warning message\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"13\" str=\"#error some error message\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"error\"/>\n"
                                "      <token column=\"8\" str=\"some error message\"/>\n"
                                "    </directive>\n"
                                "  </directivelist>\n"
                                "  <tokenlist>\n"
                                "  </tokenlist>\n";

        std::ostringstream ostr;
        directiveDump(filedata, ostr);
        ASSERT_EQUALS(dumpdata, ostr.str());
    }

    void testDirectiveIncludeLocations() {
        const char filedata[] = "#define macro1 val\n"
                                "#file \"inc1.h\"\n"
                                "#define macro2 val\n"
                                "#file \"inc2.h\"\n"
                                "#define macro3 val\n"
                                "#endfile\n"
                                "#define macro4 val\n"
                                "#endfile\n"
                                "#define macro5 val\n";
        const char dumpdata[] = "  <directivelist>\n"
                                "    <directive file=\"test.c\" linenr=\"1\" str=\"#define macro1 val\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"define\"/>\n"
                                "      <token column=\"9\" str=\"macro1\"/>\n"
                                "      <token column=\"16\" str=\"val\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"2\" str=\"#include &quot;inc1.h&quot;\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"file\"/>\n"
                                "      <token column=\"7\" str=\"&quot;inc1.h&quot;\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"inc1.h\" linenr=\"1\" str=\"#define macro2 val\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"define\"/>\n"
                                "      <token column=\"9\" str=\"macro2\"/>\n"
                                "      <token column=\"16\" str=\"val\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"inc1.h\" linenr=\"2\" str=\"#include &quot;inc2.h&quot;\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"file\"/>\n"
                                "      <token column=\"7\" str=\"&quot;inc2.h&quot;\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"inc2.h\" linenr=\"1\" str=\"#define macro3 val\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"define\"/>\n"
                                "      <token column=\"9\" str=\"macro3\"/>\n"
                                "      <token column=\"16\" str=\"val\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"inc1.h\" linenr=\"3\" str=\"#define macro4 val\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"define\"/>\n"
                                "      <token column=\"9\" str=\"macro4\"/>\n"
                                "      <token column=\"16\" str=\"val\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"3\" str=\"#define macro5 val\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"define\"/>\n"
                                "      <token column=\"9\" str=\"macro5\"/>\n"
                                "      <token column=\"16\" str=\"val\"/>\n"
                                "    </directive>\n"
                                "  </directivelist>\n"
                                "  <tokenlist>\n"
                                "  </tokenlist>\n";
        std::ostringstream ostr;
        directiveDump(filedata, ostr);
        ASSERT_EQUALS(dumpdata, ostr.str());
    }

    void testDirectiveIncludeComments() {
        const char filedata[] = "#ifdef macro2 /* this will be removed */\n"
                                "#else /* this will be removed too */\n"
                                "#endif /* this will also be removed */\n";
        const char dumpdata[] = "  <directivelist>\n"
                                "    <directive file=\"test.c\" linenr=\"1\" str=\"#ifdef macro2\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"ifdef\"/>\n"
                                "      <token column=\"8\" str=\"macro2\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"2\" str=\"#else\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"else\"/>\n"
                                "    </directive>\n"
                                "    <directive file=\"test.c\" linenr=\"3\" str=\"#endif\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"endif\"/>\n"
                                "    </directive>\n"
                                "  </directivelist>\n"
                                "  <tokenlist>\n"
                                "  </tokenlist>\n";

        std::ostringstream ostr;
        directiveDump(filedata, ostr);
        ASSERT_EQUALS(dumpdata, ostr.str());
    }

    void testDirectiveRelativePath() {
        const char filedata[] = "#define macro 1\n";
        const char dumpdata[] = "  <directivelist>\n"
                                "    <directive file=\"test.c\" linenr=\"1\" str=\"#define macro 1\">\n"
                                "      <token column=\"1\" str=\"#\"/>\n"
                                "      <token column=\"2\" str=\"define\"/>\n"
                                "      <token column=\"9\" str=\"macro\"/>\n"
                                "      <token column=\"15\" str=\"1\"/>\n"
                                "    </directive>\n"
                                "  </directivelist>\n"
                                "  <tokenlist>\n"
                                "  </tokenlist>\n";
        std::ostringstream ostr;
        Settings s;
        s.relativePaths = true;
        s.basePaths.emplace_back("/some/path");
        directiveDump(filedata, "/some/path/test.c", s, ostr);
        ASSERT_EQUALS(dumpdata, ostr.str());
    }

    void funcnameInParenthesis1() { // #13554
        const char code[] = "void f(void) {\n"
                            "  double result = (strtod)(\"NAN\", NULL);\n"
                            "}\n";
        SimpleTokenizer tokenizer(settings1, *this, false);
        ASSERT_LOC(tokenizer.tokenize(code), __FILE__, __LINE__);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "strtod");
        ASSERT(f);
        ASSERT(!f->previous()->isCast());
    }

    void funcnameInParenthesis2() { // #13578
        const char code[] = "int f(double a, double b, double c) {\n"
                            "    return static_cast<int>(std::ceil((std::min)(a, (std::min)(b, c))));\n"
                            "}\n";
        SimpleTokenizer tokenizer(settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), __FILE__, __LINE__);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "min");
        ASSERT(f);
        const Token *par = f->next();
        ASSERT(par);
        ASSERT(Token::simpleMatch(par, "("));
        ASSERT_EQUALS(par->astOperand1(), f->astParent() /* :: */);
        ASSERT(Token::simpleMatch(par->astOperand2(), ","));
    }

    void funcnameInParenthesis3() { // #13585
        const char code[] = "int f(int a, int b) {\n"
                            "    return (a != 0) ? 1 : (std::min)(1, b);\n"
                            "}\n";
        SimpleTokenizer tokenizer(settings1, *this);
        ASSERT_LOC(tokenizer.tokenize(code), __FILE__, __LINE__);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "min");
        ASSERT(f);
        const Token *par = f->next();
        ASSERT(par);
        ASSERT(Token::simpleMatch(par, "("));
        ASSERT_EQUALS(par->astOperand1(), f->astParent() /* :: */);
        ASSERT(Token::simpleMatch(par->astOperand2(), ","));
    }

    void genericInIf() { // #13561
        const char code[] = "  if (_Generic(s, char * : 1, const float * : (a ? b, c : d), volatile int * : 3, default : 0)) {}";
        const char ast[] = "(( if (( _Generic (, (, (, (, s 1) (? a (: (, b c) d))) 3) 0)))";
        ASSERT_EQUALS(ast, testAst(code, AstStyle::Z3));
    }

    void preincrementInLambda() { // #13312
        const char code[] =
            "void f(const std::vector<int>& v, int a) {\n"
            "    std::for_each(v.begin(), v.end(), [&](int i)  {\n"
            "        switch (i) {\n"
            "        default:\n"
            "            ++a;\n"
            "        }\n"
            "    });\n"
            "}\n";
        ASSERT_NO_THROW(tokenizeAndStringify(code));
    }

    void atomicCast() { // #12605
        const char code[] = "int atomic_add_int(int *ptr, int v)\n"
                            "{\n"
                            "    return atomic_fetch_add((_Atomic(unsigned int) *)ptr, v) + v;\n"
                            "}\n";
        ASSERT_NO_THROW(tokenizeAndStringify(code, settingsDefault, false));
    }

    void constFunctionPtrTypedef() { // #12135
        const char code[] = "struct S {\n"
                            "    template<typename T> T g() {\n"
                            "        return T();\n"
                            "    }\n"
                            "};\n"
                            "void f() {\n"
                            "    S s;\n"
                            "    typedef void (*fp_t)();\n"
                            "    if (fp_t const fp = s.g<fp_t>()) {}\n"
                            "}\n";
        ASSERT_NO_THROW(tokenizeAndStringify(code));
        ASSERT_EQUALS("void ( * const f ) ( ) ;", tokenizeAndStringify("typedef void (*fp_t)(); fp_t const f;"));
    }

    void simplifyPlatformTypes() {
        {
            const char code[] = "size_t f();";
            ASSERT_EQUALS("unsigned long f ( ) ;", tokenizeAndStringify(code, settings2_unix32));
            ASSERT_EQUALS("unsigned long f ( ) ;", tokenizeAndStringify(code, settings2_unix64));
        }
        {
            const char code[] = "ssize_t f();";
            ASSERT_EQUALS("long f ( ) ;", tokenizeAndStringify(code, settings2_unix32));
            ASSERT_EQUALS("long f ( ) ;", tokenizeAndStringify(code, settings2_unix64));
        }
        {
            const char code[] = "std::ptrdiff_t f();";
            ASSERT_EQUALS("long f ( ) ;", tokenizeAndStringify(code, settings2_unix32));
            ASSERT_EQUALS("long f ( ) ;", tokenizeAndStringify(code, settings2_unix64));
        }
    }

    void dumpFallthrough() {
        const char code[] = "void f(int n) {\n"
                            "    void g(), h(), i();\n"
                            "    switch (n) {\n"
                            "        case 1:\n"
                            "        case 2:\n"
                            "            g();\n"
                            "            [[fallthrough]];\n"
                            "        case 3:\n"
                            "            h();\n"
                            "            break;\n"
                            "        default:\n"
                            "            i();\n"
                            "            break;\n"
                            "    }\n"
                            "}";
        SimpleTokenizer tokenizer(settingsDefault, *this, false);
        ASSERT(tokenizer.tokenize(code));
        std::ostringstream ostr;
        tokenizer.dump(ostr);
        const std::string dump = ostr.str();
        ASSERT(dump.find(" isAttributeFallthrough=\"true\"") != std::string::npos);
    }

    void simplifyRedundantParentheses() {
        const char code[] = "int f(struct S s) {\n"
                            "    return g(1, &(int){ s.i });\n"
                            "}\n";
        SimpleTokenizer tokenizer(settingsDefault, *this, false);
        ASSERT_NO_THROW(tokenizer.tokenize(code));
    }

    void simplifyEnum1() {
        const char code[] = "static enum {A,B} ab;";
        ASSERT_EQUALS("enum Anonymous0 { A , B } ; static enum Anonymous0 ab ;", tokenizeAndStringify(code));
        SimpleTokenizer tokenizer(settingsDefault, *this);
        tokenizer.tokenize(code);
        const Token* tok = Token::findsimplematch(tokenizer.tokens(), "static");
        ASSERT(tok);
        ASSERT_EQUALS(tok->column(), 1);
        ASSERT_EQUALS(tok->next()->column(), 8);
    }

    void simplifyEnum2() {
        const char code[] = "enum AB {A,B}; enum AB static ab; ";
        ASSERT_EQUALS("enum AB { A , B } ; static enum AB ab ;", tokenizeAndStringify(code));
        SimpleTokenizer tokenizer(settingsDefault, *this);
        tokenizer.tokenize(code);
        const Token* tok = Token::findsimplematch(tokenizer.tokens(), "static");
        ASSERT(tok);
        ASSERT_EQUALS(tok->column(), 24);
        ASSERT_EQUALS(tok->next()->column(), 16);
    }
};

REGISTER_TEST(TestTokenizer)
