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

#include "checkers.h"
#include "cmdlinelogger.h"
#include "cmdlineparser.h"
#include "config.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filesettings.h"
#include "fixture.h"
#include "helpers.h"
#include "path.h"
#include "platform.h"
#include "redirect.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "timer.h"
#include "utils.h"

#include <cstdio>
#include <list>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

class TestCmdlineParser : public TestFixture {
public:
    TestCmdlineParser() : TestFixture("TestCmdlineParser")
    {}

private:
    class CmdLineLoggerTest final : public CmdLineLogger
    {
    public:
        CmdLineLoggerTest() = default;

        void printMessage(const std::string &message) override
        {
            printInternal("cppcheck: " + message + '\n');
        }

        void printError(const std::string &message) override
        {
            printMessage("error: " + message);
        }

        void printRaw(const std::string &message) override
        {
            printInternal(message + '\n'); // TODO: should not append newline
        }

        std::string str()
        {
            std::string s;
            std::swap(buf, s);
            return s;
        }

        void destroy()
        {
            if (!buf.empty())
                throw std::runtime_error("unconsumed messages: " + buf);
        }

    private:
        void printInternal(const std::string &message)
        {
            buf += message;
        }

        std::string buf;
    };

    class CmdLineParserTest final : public CmdLineParser
    {
        friend class TestCmdlineParser;
    public:
        CmdLineParserTest(CmdLineLogger &logger, Settings &settings, Suppressions &suppressions)
            : CmdLineParser(logger, settings, suppressions)
        {}
    };

    std::unique_ptr<CmdLineLoggerTest> logger;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<Suppressions> supprs;
    std::unique_ptr<CmdLineParserTest> parser;

    void prepareTestInternal() override {
        logger.reset(new CmdLineLoggerTest());
        settings.reset(new Settings());
        supprs.reset(new Suppressions());
        parser.reset(new CmdLineParserTest(*logger, *settings, *supprs));
    }

    void teardownTestInternal() override {
        logger->destroy();
    }

    void asPremium() {
        // set this so we think it is the premium
        settings->cppcheckCfgProductName = "Cppcheck Premium 0.0.0";
    }

    template<size_t size>
    CmdLineParser::Result parseFromArgs(const char* const (&argv)[size]) {
        return parser->parseFromArgs(size, argv);
    }

    template<size_t size>
    bool fillSettingsFromArgs(const char* const (&argv)[size]) {
        return parser->fillSettingsFromArgs(size, argv);
    }

    void run() override {
        TEST_CASE(nooptions);
        TEST_CASE(nooptionsWithCfg);
        TEST_CASE(nooptionsWithInvalidCfg);
        TEST_CASE(helpshort);
        TEST_CASE(helpshortExclusive);
        TEST_CASE(helpshortWithCfg);
        TEST_CASE(helplong);
        TEST_CASE(helplongExclusive);
        TEST_CASE(helplongWithCfg);
        TEST_CASE(version);
        TEST_CASE(versionWithCfg);
        TEST_CASE(versionExclusive);
        TEST_CASE(checkVersionCorrect);
        TEST_CASE(checkVersionIncorrect);
        TEST_CASE(onefile);
        TEST_CASE(onepath);
        TEST_CASE(optionwithoutfile);
        TEST_CASE(verboseshort);
        TEST_CASE(verboselong);
        TEST_CASE(debugSimplified);
        TEST_CASE(debugwarnings);
        TEST_CASE(forceshort);
        TEST_CASE(forcelong);
        TEST_CASE(relativePaths1);
        TEST_CASE(relativePaths2);
        TEST_CASE(relativePaths3);
        TEST_CASE(relativePaths4);
        TEST_CASE(quietshort);
        TEST_CASE(quietlong);
        TEST_CASE(defines_noarg);
        TEST_CASE(defines_noarg2);
        TEST_CASE(defines_noarg3);
        TEST_CASE(defines);
        TEST_CASE(defines2);
        TEST_CASE(defines3);
        TEST_CASE(defines4);
        TEST_CASE(enforceLanguage1);
        TEST_CASE(enforceLanguage2);
        TEST_CASE(enforceLanguage3);
        TEST_CASE(enforceLanguage4);
        TEST_CASE(enforceLanguage5);
        TEST_CASE(enforceLanguage6);
        TEST_CASE(enforceLanguage7);
        TEST_CASE(includesnopath);
        TEST_CASE(includes);
        TEST_CASE(includesslash);
        TEST_CASE(includesbackslash);
        TEST_CASE(includesnospace);
        TEST_CASE(includes2);
        TEST_CASE(includesFile);
        TEST_CASE(includesFileNoFile);
        TEST_CASE(configExcludesFile);
        TEST_CASE(configExcludesFileNoFile);
        TEST_CASE(enabledAll);
        TEST_CASE(enabledStyle);
        TEST_CASE(enabledPerformance);
        TEST_CASE(enabledPortability);
        TEST_CASE(enabledInformation);
        TEST_CASE(enabledUnusedFunction);
        TEST_CASE(enabledMissingInclude);
        TEST_CASE(disabledMissingIncludeWithInformation);
        TEST_CASE(enabledMissingIncludeWithInformation);
        TEST_CASE(enabledMissingIncludeWithInformationReverseOrder);
#ifdef CHECK_INTERNAL
        TEST_CASE(enabledInternal);
#endif
        TEST_CASE(enabledMultiple);
        TEST_CASE(enabledInvalid);
        TEST_CASE(enabledError);
        TEST_CASE(enabledEmpty);
        TEST_CASE(disableAll);
        TEST_CASE(disableMultiple);
        TEST_CASE(disableStylePartial);
        TEST_CASE(disableInformationPartial);
        TEST_CASE(disableInformationPartial2);
        TEST_CASE(disableInvalid);
        TEST_CASE(disableError);
        TEST_CASE(disableEmpty);
        TEST_CASE(inconclusive);
        TEST_CASE(errorExitcode);
        TEST_CASE(errorExitcodeMissing);
        TEST_CASE(errorExitcodeStr);
        TEST_CASE(exitcodeSuppressionsOld);
        TEST_CASE(exitcodeSuppressions);
        TEST_CASE(exitcodeSuppressionsNoFile);
        TEST_CASE(fileFilterFileWithDetailsSimplifiedPath);
        TEST_CASE(fileFilterFileWithDetailsCase);
        TEST_CASE(fileFilterStdin);
        TEST_CASE(fileFilterPlus);
        TEST_CASE(fileFilterNoMatch);
        TEST_CASE(fileList);
        TEST_CASE(fileListNoFile);
        TEST_CASE(fileListStdin);
        TEST_CASE(fileListInvalid);
        TEST_CASE(inlineSuppr);
        TEST_CASE(jobs);
        TEST_CASE(jobs2);
        TEST_CASE(jobsMissingCount);
        TEST_CASE(jobsInvalid);
        TEST_CASE(jobsNoJobs);
        TEST_CASE(jobsTooBig);
        TEST_CASE(maxConfigs);
        TEST_CASE(maxConfigsMissingCount);
        TEST_CASE(maxConfigsInvalid);
        TEST_CASE(maxConfigsTooSmall);
        TEST_CASE(outputFormatText);
        TEST_CASE(outputFormatSarif);
        TEST_CASE(outputFormatXml);
        TEST_CASE(outputFormatXmlv2);
        TEST_CASE(outputFormatXmlv3);
        TEST_CASE(outputFormatOther);
        TEST_CASE(outputFormatImplicitPlist);
        TEST_CASE(outputFormatImplicitXml);
        TEST_CASE(outputFormatOverridePlist);
        TEST_CASE(outputFormatMixed1);
        TEST_CASE(outputFormatMixed2);
        TEST_CASE(outputFormatMixed3);
        TEST_CASE(premiumOptions1);
        TEST_CASE(premiumOptions2);
        TEST_CASE(premiumOptions3);
        TEST_CASE(premiumOptions4);
        TEST_CASE(premiumOptions5);
        TEST_CASE(premiumOptionsAll);
        TEST_CASE(premiumOptionsMetrics);
        TEST_CASE(premiumOptionsCertCIntPrecision);
        TEST_CASE(premiumOptionsLicenseFile);
        TEST_CASE(premiumOptionsInvalid1);
        TEST_CASE(premiumOptionsInvalid2);
        TEST_CASE(premiumSafety);
        TEST_CASE(premiumDebugProgress);
        TEST_CASE(reportProgress1);
        TEST_CASE(reportProgress2);
        TEST_CASE(reportProgress3);
        TEST_CASE(reportProgress4);
        TEST_CASE(reportProgress5);
        TEST_CASE(stdc99);
        TEST_CASE(stdgnu17);
        TEST_CASE(stdiso9899_1999);
        TEST_CASE(stdcpp11);
        TEST_CASE(stdgnupp23);
        TEST_CASE(stdunknown1);
        TEST_CASE(stdunknown2);
        TEST_CASE(stdunknown3);
        TEST_CASE(stdoverwrite1);
        TEST_CASE(stdoverwrite2);
        TEST_CASE(stdoverwrite3);
        TEST_CASE(stdoverwrite4);
        TEST_CASE(stdmulti1);
        TEST_CASE(stdmulti2);
        TEST_CASE(platformWin64);
        TEST_CASE(platformWin32A);
        TEST_CASE(platformWin32W);
        TEST_CASE(platformUnix32);
        TEST_CASE(platformUnix32Unsigned);
        TEST_CASE(platformUnix64);
        TEST_CASE(platformUnix64Unsigned);
        TEST_CASE(platformNative);
        TEST_CASE(platformUnspecified);
        TEST_CASE(platformPlatformFile);
        TEST_CASE(platformUnknown);
        TEST_CASE(platformEmpty);
        TEST_CASE(plistEmpty);
        TEST_CASE(plistDoesNotExist);
        TEST_CASE(suppressionsOld);
        TEST_CASE(suppressions);
        TEST_CASE(suppressionsNoFile1);
        TEST_CASE(suppressionsNoFile2);
        TEST_CASE(suppressionsNoFile3);
        TEST_CASE(suppressSingle);
        TEST_CASE(suppressSingleFile);
        TEST_CASE(suppressTwo);
        TEST_CASE(suppressTwoSeparate);
        TEST_CASE(templates);
        TEST_CASE(templatesGcc);
        TEST_CASE(templatesVs);
        TEST_CASE(templatesEdit);
        TEST_CASE(templatesCppcheck1);
        TEST_CASE(templatesDaca2);
        TEST_CASE(templatesSelfcheck);
        TEST_CASE(templatesSimple);
        TEST_CASE(templatesNoPlaceholder);
        TEST_CASE(templateFormatInvalid);
        TEST_CASE(templateFormatEmpty);
        TEST_CASE(templateLocationInvalid);
        TEST_CASE(templateLocationEmpty);
        TEST_CASE(xml);
        TEST_CASE(xmlver2);
        TEST_CASE(xmlver2both);
        TEST_CASE(xmlver2both2);
        TEST_CASE(xmlverunknown);
        TEST_CASE(xmlverinvalid);
        TEST_CASE(doc);
        TEST_CASE(docExclusive);
        TEST_CASE(showtimeSummary);
        TEST_CASE(showtimeFile);
        TEST_CASE(showtimeFileTotal);
        TEST_CASE(showtimeTop5);
        TEST_CASE(showtimeTop5File);
        TEST_CASE(showtimeTop5Summary);
        TEST_CASE(showtimeNone);
        TEST_CASE(showtimeEmpty);
        TEST_CASE(showtimeInvalid);
        TEST_CASE(errorlist);
        TEST_CASE(errorlistWithCfg);
        TEST_CASE(errorlistExclusive);
        TEST_CASE(ignorepathsnopath);
#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
        TEST_CASE(exceptionhandling);
        TEST_CASE(exceptionhandling2);
        TEST_CASE(exceptionhandling3);
        TEST_CASE(exceptionhandlingInvalid);
        TEST_CASE(exceptionhandlingInvalid2);
#else
        TEST_CASE(exceptionhandlingNotSupported);
        TEST_CASE(exceptionhandlingNotSupported2);
#endif
        TEST_CASE(clang);
        TEST_CASE(clangCustom);
        TEST_CASE(clangInvalid);
        TEST_CASE(valueFlowMaxIterations);
        TEST_CASE(valueFlowMaxIterations2);
        TEST_CASE(valueFlowMaxIterationsInvalid);
        TEST_CASE(valueFlowMaxIterationsInvalid2);
        TEST_CASE(valueFlowMaxIterationsInvalid3);
        TEST_CASE(checksMaxTime);
        TEST_CASE(checksMaxTime2);
        TEST_CASE(checksMaxTimeInvalid);
#ifdef HAS_THREADING_MODEL_FORK
        TEST_CASE(loadAverage);
        TEST_CASE(loadAverage2);
        TEST_CASE(loadAverageInvalid);
#else
        TEST_CASE(loadAverageNotSupported);
#endif
        TEST_CASE(maxCtuDepth);
        TEST_CASE(maxCtuDepth2);
        TEST_CASE(maxCtuDepthLimit);
        TEST_CASE(maxCtuDepthInvalid);
        TEST_CASE(performanceValueflowMaxTime);
        TEST_CASE(performanceValueflowMaxTimeInvalid);
        TEST_CASE(performanceValueFlowMaxIfCount);
        TEST_CASE(performanceValueFlowMaxIfCountInvalid);
        TEST_CASE(templateMaxTime);
        TEST_CASE(templateMaxTimeInvalid);
        TEST_CASE(templateMaxTimeInvalid2);
        TEST_CASE(typedefMaxTime);
        TEST_CASE(typedefMaxTimeInvalid);
        TEST_CASE(typedefMaxTimeInvalid2);
        TEST_CASE(templateMaxTime);
        TEST_CASE(templateMaxTime);
        TEST_CASE(project);
        TEST_CASE(projectMultiple);
        TEST_CASE(projectAndSource);
        TEST_CASE(projectEmpty);
        TEST_CASE(projectMissing);
        TEST_CASE(projectNoPaths);
        TEST_CASE(addon);
        TEST_CASE(addonMissing);
#ifdef HAVE_RULES
        TEST_CASE(rule);
        TEST_CASE(ruleMissingPattern);
        TEST_CASE(ruleInvalidPattern);
#else
        TEST_CASE(ruleNotSupported);
#endif
#ifdef HAVE_RULES
        TEST_CASE(ruleFileMulti);
        TEST_CASE(ruleFileSingle);
        TEST_CASE(ruleFileEmpty);
        TEST_CASE(ruleFileMissing);
        TEST_CASE(ruleFileInvalid);
        TEST_CASE(ruleFileNoRoot);
        TEST_CASE(ruleFileEmptyElements1);
        TEST_CASE(ruleFileEmptyElements2);
        TEST_CASE(ruleFileUnknownElement1);
        TEST_CASE(ruleFileUnknownElement2);
        TEST_CASE(ruleFileMissingTokenList);
        TEST_CASE(ruleFileUnknownTokenList);
        TEST_CASE(ruleFileMissingId);
        TEST_CASE(ruleFileInvalidSeverity1);
        TEST_CASE(ruleFileInvalidSeverity2);
        TEST_CASE(ruleFileInvalidPattern);
#else
        TEST_CASE(ruleFileNotSupported);
#endif
        TEST_CASE(signedChar);
        TEST_CASE(signedChar2);
        TEST_CASE(unsignedChar);
        TEST_CASE(unsignedChar2);
        TEST_CASE(signedCharUnsignedChar);
        TEST_CASE(library);
        TEST_CASE(libraryMissing);
        TEST_CASE(libraryMultiple);
        TEST_CASE(libraryMultipleEmpty);
        TEST_CASE(libraryMultipleEmpty2);
        TEST_CASE(suppressXml);
        TEST_CASE(suppressXmlEmpty);
        TEST_CASE(suppressXmlMissing);
        TEST_CASE(suppressXmlInvalid);
        TEST_CASE(suppressXmlNoRoot);
        TEST_CASE(executorDefault);
        TEST_CASE(executorAuto);
        TEST_CASE(executorAutoNoJobs);
#if defined(HAS_THREADING_MODEL_THREAD)
        TEST_CASE(executorThread);
        TEST_CASE(executorThreadNoJobs);
#else
        TEST_CASE(executorThreadNotSupported);
#endif
#if defined(HAS_THREADING_MODEL_FORK)
        TEST_CASE(executorProcess);
        TEST_CASE(executorProcessNoJobs);
#else
        TEST_CASE(executorProcessNotSupported);
#endif
        TEST_CASE(checkLevelDefault);
        TEST_CASE(checkLevelNormal);
        TEST_CASE(checkLevelExhaustive);
        TEST_CASE(checkLevelUnknown);
        TEST_CASE(cppHeaderProbe);
        TEST_CASE(cppHeaderProbe2);
        TEST_CASE(noCppHeaderProbe);
        TEST_CASE(noCppHeaderProbe2);
        TEST_CASE(debugLookup);
        TEST_CASE(debugLookupAll);
        TEST_CASE(debugLookupAddon);
        TEST_CASE(debugLookupConfig);
        TEST_CASE(debugLookupLibrary);
        TEST_CASE(debugLookupPlatform);
        TEST_CASE(maxTemplateRecursion);
        TEST_CASE(maxTemplateRecursionMissingCount);
        TEST_CASE(emitDuplicates);
        TEST_CASE(debugClangOutput);
        TEST_CASE(debugXmlMultiple);
        TEST_CASE(debugNormalXmlMultiple);
        TEST_CASE(debugIgnore);
        TEST_CASE(checkHeaders);
        TEST_CASE(noCheckHeaders);
        TEST_CASE(noCheckHeaders2);
        TEST_CASE(filesdir);
        TEST_CASE(checkUnusedTemplates);
        TEST_CASE(noCheckUnusedTemplates);
        TEST_CASE(noCheckUnusedTemplates);
        TEST_CASE(clangTidy);
        TEST_CASE(clangTidyCustom);
        TEST_CASE(projectConfigurationNoProject);
        TEST_CASE(projectConfigurationEmpty);
        TEST_CASE(analyzeAllVsConfigs);
        TEST_CASE(noAnalyzeAllVsConfigs);
        TEST_CASE(noAnalyzeAllVsConfigs2);
        TEST_CASE(debugSymdb);
        TEST_CASE(debugAst);
        TEST_CASE(debugValueflow);
        TEST_CASE(debugNormal);
        TEST_CASE(debugNormalVerbose);
        TEST_CASE(debug);
        TEST_CASE(debugVerbose);
        TEST_CASE(safety);
        TEST_CASE(safetyOverride);
        TEST_CASE(noSafety);
        TEST_CASE(noSafetyOverride);
        TEST_CASE(debugAnalyzerinfo);

        TEST_CASE(ignorepaths1);
        TEST_CASE(ignorepaths2);
        TEST_CASE(ignorepaths3);
        TEST_CASE(ignorepaths4);
        TEST_CASE(ignorefilepaths1);
        TEST_CASE(ignorefilepaths2);
        TEST_CASE(ignorefilepaths3);
        TEST_CASE(ignorefilepaths4);
        TEST_CASE(ignorefilepaths5);
        TEST_CASE(ignorefilepaths6);
        TEST_CASE(ignorefilepaths7);
        TEST_CASE(ignorefilepaths8);
        TEST_CASE(ignorefilepaths9);

        TEST_CASE(nonexistentpath);

        TEST_CASE(checkconfig);
        TEST_CASE(unknownParam);

        TEST_CASE(undefs_noarg);
        TEST_CASE(undefs_noarg2);
        TEST_CASE(undefs_noarg3);
        TEST_CASE(undefs);
        TEST_CASE(undefs2);

        TEST_CASE(cppcheckBuildDirExistent);
        TEST_CASE(cppcheckBuildDirNonExistent);
        TEST_CASE(cppcheckBuildDirEmpty);
        TEST_CASE(cppcheckBuildDirMultiple);
        TEST_CASE(noCppcheckBuildDir);
        TEST_CASE(noCppcheckBuildDir2);

        TEST_CASE(invalidCppcheckCfg);

        TEST_CASE(reportTypeAutosar);
        TEST_CASE(reportTypeCertCpp);
        TEST_CASE(reportTypeCertC);
        TEST_CASE(reportTypeMisraC2012);
        TEST_CASE(reportTypeMisraC2023);
        TEST_CASE(reportTypeMisraC2025);
        TEST_CASE(reportTypeMisraCpp2008);
        TEST_CASE(reportTypeMisraCpp2023);
        TEST_CASE(invalidReportType);
        TEST_CASE(defaultReportType);
    }

    void nooptions() {
        REDIRECT;
        const char * const argv[] = {"cppcheck"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void nooptionsWithCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n"
                        "\"productName\": \"Cppcheck Premium\","
                        "\"manualUrl\": \"https://docs.notcppcheck.com/manual.pdf\""
                        "}\n");
        const char * const argv[] = {"cppcheck"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->settingsFiles.size());
        ASSERT_EQUALS(file.path(), *settings->settingsFiles.cbegin());
        const std::string log_str = logger->str();
        ASSERT_MSG(startsWith(log_str, "Cppcheck - A tool for static C/C++ code analysis"), "header");
        ASSERT_MSG(log_str.find("https://docs.notcppcheck.com/manual.pdf") != std::string::npos, "help url");
    }

    void nooptionsWithInvalidCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n");
        const char * const argv[] = {"cppcheck"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 2 near: \n", logger->str());
    }

    void helpshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-h"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helpshortExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "-h"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helpshortWithCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n"
                        "\"productName\": \"Cppcheck Premium\","
                        "\"manualUrl\": \"https://docs.notcppcheck.com/manual.pdf\""
                        "}\n");
        const char * const argv[] = {"cppcheck", "-h"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->settingsFiles.size());
        ASSERT_EQUALS(file.path(), *settings->settingsFiles.cbegin());
        const std::string log_str = logger->str();
        ASSERT_MSG(startsWith(log_str, "Cppcheck - A tool for static C/C++ code analysis"), "header");
        ASSERT_MSG(log_str.find("https://docs.notcppcheck.com/manual.pdf") != std::string::npos, "help url");
    }

    void helplong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--help"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helplongExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--help"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(startsWith(logger->str(), "Cppcheck - A tool for static C/C++ code analysis"));
    }

    void helplongWithCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n"
                        "\"productName\": \"Cppcheck Premium\","
                        "\"manualUrl\": \"https://docs.notcppcheck.com/manual.pdf\""
                        "}\n");
        const char * const argv[] = {"cppcheck", "--help"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->settingsFiles.size());
        ASSERT_EQUALS(file.path(), *settings->settingsFiles.cbegin());
        const std::string log_str = logger->str();
        ASSERT_MSG(startsWith(log_str, "Cppcheck - A tool for static C/C++ code analysis"), "header");
        ASSERT_MSG(log_str.find("https://docs.notcppcheck.com/manual.pdf") != std::string::npos, "help url");
    }

    void version() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--version"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(logger->str().compare(0, 11, "Cppcheck 2.") == 0);
    }

    void versionWithCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n"
                        "\"productName\": \"The Product\""
                        "}\n");
        const char * const argv[] = {"cppcheck", "--version"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->settingsFiles.size());
        ASSERT_EQUALS(file.path(), *settings->settingsFiles.cbegin());
        ASSERT_EQUALS("The Product\n", logger->str()); // TODO: include version?
    }

    // TODO: test --version with extraVersion

    void versionExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--version"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(logger->str().compare(0, 11, "Cppcheck 2.") == 0);
    }

    void checkVersionCorrect() {
        REDIRECT;
        const std::string currentVersion = parser->getVersion();
        const std::string checkVersion = "--check-version=" + currentVersion;
        const char * const argv[] = {"cppcheck", checkVersion.c_str(), "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
    }

    void checkVersionIncorrect() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-version=Cppcheck 2.0", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --check-version check failed. Aborting.\n", logger->str());
    }

    void onefile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void onepath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "src"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("src", parser->mPathNames[0]);
    }

    void optionwithoutfile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS(0, parser->mPathNames.size());
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", logger->str());
    }

    void verboseshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-v", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->verbose);
    }

    void verboselong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--verbose", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->verbose);
    }

    void debugSimplified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-simplified", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugSimplified);
    }

    void debugwarnings() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-warnings", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugwarnings);
    }

    void forceshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->force);
    }

    void forcelong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--force", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->force);
    }

    void relativePaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-rp", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->relativePaths);
    }

    void relativePaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--relative-paths", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->relativePaths);
    }

    void relativePaths3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-rp=C:/foo;C:\\bar", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS(2, settings->basePaths.size());
        ASSERT_EQUALS("C:/foo", settings->basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings->basePaths[1]);
    }

    void relativePaths4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--relative-paths=C:/foo;C:\\bar", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->relativePaths);
        ASSERT_EQUALS(2, settings->basePaths.size());
        ASSERT_EQUALS("C:/foo", settings->basePaths[0]);
        ASSERT_EQUALS("C:/bar", settings->basePaths[1]);
    }

    void quietshort() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-q", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->quiet);
    }

    void quietlong() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--quiet", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->quiet);
    }

    void defines_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D"};
        // Fails since -D has no param
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "-v", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "--quiet", "file.cpp"};
        // Fails since -D has no param
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-D' is missing.\n", logger->str());
    }

    void defines() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("_WIN32=1", settings->userDefines);
    }

    void defines2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D_WIN32", "-DNODEBUG", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("_WIN32=1;NODEBUG=1", settings->userDefines);
    }

    void defines3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-D", "DEBUG", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("DEBUG=1", settings->userDefines);
    }

    void defines4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-DDEBUG=", "file.cpp"}; // #5137 - defining empty macro
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("DEBUG=", settings->userDefines);
    }

    void enforceLanguage1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Standards::Language::None, parser->mEnforcedLang);
    }

    void enforceLanguage2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x", "c++", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Standards::Language::CPP, parser->mEnforcedLang);
    }

    void enforceLanguage3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", logger->str());
    }

    void enforceLanguage4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-x", "--inconclusive", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: no language given to '-x' option.\n", logger->str());
    }

    void enforceLanguage5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=c++", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Standards::Language::CPP, parser->mEnforcedLang);
    }

    void enforceLanguage6() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=c", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Standards::Language::C, parser->mEnforcedLang);
    }

    void enforceLanguage7() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--language=unknownLanguage", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unknown language 'unknownLanguage' enforced.\n", logger->str());
    }

    void includesnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I"};
        // Fails since -I has no param
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-I' is missing.\n", logger->str());
    }

    void includes() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includesslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includesbackslash() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include\\", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includesnospace() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-Iinclude", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
    }

    void includes2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-I", "include/", "-I", "framework/", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("include/", settings->includePaths.front());
        settings->includePaths.pop_front();
        ASSERT_EQUALS("framework/", settings->includePaths.front());
    }

    void includesFile() {
        REDIRECT;
        ScopedFile file("includes.txt",
                        "path/sub\n"
                        "path2/sub1\n");
        const char * const argv[] = {"cppcheck", "--includes-file=includes.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(2, settings->includePaths.size());
        auto it = settings->includePaths.cbegin();
        ASSERT_EQUALS("path/sub/", *it++);
        ASSERT_EQUALS("path2/sub1/", *it);
    }

    void includesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--includes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to open includes file at 'fileThatDoesNotExist.txt'\n", logger->str());
    }

    void configExcludesFile() {
        REDIRECT;
        ScopedFile file("excludes.txt",
                        "path/sub\n"
                        "path2/sub1\n");
        const char * const argv[] = {"cppcheck", "--config-excludes-file=excludes.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(2, settings->configExcludePaths.size());
        auto it = settings->configExcludePaths.cbegin();
        ASSERT_EQUALS("path/sub/", *it++);
        ASSERT_EQUALS("path2/sub1/", *it);
    }

    void configExcludesFileNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--config-excludes-file=fileThatDoesNotExist.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to open config excludes file at 'fileThatDoesNotExist.txt'\n", logger->str());
    }

    void enabledAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT(!settings->checks.isEnabled(Checks::internalCheck));
    }

    void enabledStyle() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::internalCheck));
    }

    void enabledPerformance() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=performance", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(!settings->severity.isEnabled(Severity::warning));
        ASSERT(settings->severity.isEnabled(Severity::performance));
        ASSERT(!settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
    }

    void enabledPortability() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=portability", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(!settings->severity.isEnabled(Severity::warning));
        ASSERT(!settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
    }

    void enabledInformation() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
    }

    void enabledUnusedFunction() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=unusedFunction", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->checks.isEnabled(Checks::unusedFunction));
    }

    void enabledMissingInclude() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
    }

    void disabledMissingIncludeWithInformation() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=missingInclude", "--enable=information", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledMissingIncludeWithInformation() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "--enable=missingInclude", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void enabledMissingIncludeWithInformationReverseOrder() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "--enable=information", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

#ifdef CHECK_INTERNAL
    void enabledInternal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=internal", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(settings->checks.isEnabled(Checks::internalCheck));
    }
#endif

    void enabledMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude,portability,warning", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT(!settings->severity.isEnabled(Severity::style));
        ASSERT(settings->severity.isEnabled(Severity::warning));
        ASSERT(!settings->severity.isEnabled(Severity::performance));
        ASSERT(settings->severity.isEnabled(Severity::portability));
        ASSERT(!settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
    }

    void enabledInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=warning,missingIncludeSystem,style", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'missingIncludeSystem'\n", logger->str());
    }

    void enabledError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=error", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter with the unknown name 'error'\n", logger->str());
    }

    void enabledEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --enable parameter is empty\n", logger->str());
    }


    void disableAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=all", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
    }

    void disableMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=all", "--disable=style", "--disable=unusedFunction", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(true, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
    }

    // make sure the implied "style" checks are not added when "--enable=style" is specified
    void disableStylePartial() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=style", "--disable=performance", "--enable=unusedFunction", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::style));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::performance));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::portability));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::debug));
        ASSERT_EQUALS(true, settings->checks.isEnabled(Checks::unusedFunction));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS(false, settings->checks.isEnabled(Checks::internalCheck));
    }

    void disableInformationPartial() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=information", "--disable=missingInclude", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::information));
        ASSERT(!settings->checks.isEnabled(Checks::missingInclude));
        ASSERT_EQUALS("", logger->str());
    }

    void disableInformationPartial2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--enable=missingInclude", "--disable=information", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(!settings->severity.isEnabled(Severity::information));
        ASSERT(settings->checks.isEnabled(Checks::missingInclude));
    }

    void disableInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=leaks", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'leaks'\n", logger->str());
    }

    void disableError() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=error", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter with the unknown name 'error'\n", logger->str());
    }

    void disableEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--disable=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --disable parameter is empty\n", logger->str());
    }

    void inconclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inconclusive", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->certainty.isEnabled(Certainty::inconclusive));
    }

    void errorExitcode() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=5", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(5, settings->exitCode);
    }

    void errorExitcodeMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=", "file.cpp"};
        // Fails since exit code not given
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--error-exitcode=' is not valid - not an integer.\n", logger->str());
    }

    void errorExitcodeStr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--error-exitcode=foo", "file.cpp"};
        // Fails since invalid exit code
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--error-exitcode=' is not valid - not an integer.\n", logger->str());
    }

    void exitcodeSuppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "suppr.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", logger->str());
    }

    void exitcodeSuppressions() {
        REDIRECT;
        ScopedFile file("suppr.txt",
                        "uninitvar\n"
                        "unusedFunction\n");
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions=suppr.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        const auto supprsNofail = supprs->nofail.getSuppressions();
        ASSERT_EQUALS(2, supprsNofail.size());
        auto it = supprsNofail.cbegin();
        ASSERT_EQUALS("uninitvar", (it++)->errorId);
        ASSERT_EQUALS("unusedFunction", it->errorId);
    }

    void exitcodeSuppressionsNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exitcode-suppressions", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exitcode-suppressions\".\n", logger->str());
    }

    void fileFilterFileWithDetailsSimplifiedPath() const {
        // match against simplified path
        const std::vector<std::string> fileFilters{"m1.c"};
        const std::list<FileWithDetails> filesResolved{ FileWithDetails("./m1.c", Standards::Language::C, 123) };
        const std::list<FileWithDetails> files = CmdLineParser::filterFiles(fileFilters, filesResolved);
        ASSERT_EQUALS(1U, files.size());
    }

    void fileFilterFileWithDetailsCase() {
        // in windows, paths are case insensitive
        const std::vector<std::string> fileFilters{"m1.c"};
        const std::list<FileWithDetails> filesResolved{ FileWithDetails("M1.C", Standards::Language::C, 123) };
        const std::list<FileWithDetails> files = CmdLineParser::filterFiles(fileFilters, filesResolved);
#ifdef _WIN32
        ASSERT_EQUALS(1U, files.size());
#else
        ASSERT_EQUALS(0U, files.size());
#endif
    }

    void fileFilterStdin() {
        REDIRECT;
        RedirectInput input("file1.c\nfile2.cpp\n");
        const char * const argv[] = {"cppcheck", "--file-filter=-"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", logger->str());
        ASSERT_EQUALS(2U, settings->fileFilters.size());
        ASSERT_EQUALS("file1.c", settings->fileFilters[0]);
        ASSERT_EQUALS("file2.cpp", settings->fileFilters[1]);
    }

    void fileFilterPlus() {
        REDIRECT;
        ScopedFile file("project.cppcheck",
                        "<project>\n"
                        "<paths>\n"
                        "<dir name=\"src\"/>\n"
                        "</paths>\n"
                        "</project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "--file-filter=+", "src/file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("src/file.cpp", settings->fileFilters.front());
    }

    void fileFilterNoMatch() {
        REDIRECT;
        RedirectInput input("notexist1.c\nnotexist2.cpp\n");
        const char * const argv[] = {"cppcheck", "--file-filter=-", "."};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: could not find any files matching the filter:notexist1.c\n"
                      "cppcheck: error: could not find any files matching the filter:notexist2.cpp\n",
                      logger->str());
    }

    void fileList() {
        REDIRECT;
        ScopedFile file("files.txt",
                        "file1.c\n"
                        "file2.cpp\n");
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(3, parser->mPathNames.size());
        auto it = parser->mPathNames.cbegin();
        ASSERT_EQUALS("file1.c", *it++);
        ASSERT_EQUALS("file2.cpp", *it++);
        ASSERT_EQUALS("file.cpp", *it);
    }

    void fileListNoFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list=files.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: couldn't open the file: \"files.txt\".\n", logger->str());
    }

    void fileListStdin() {
        REDIRECT;
        RedirectInput input("file1.c\nfile2.cpp\n");
        const char * const argv[] = {"cppcheck", "--file-list=-", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(3, parser->mPathNames.size());
        auto it = parser->mPathNames.cbegin();
        ASSERT_EQUALS("file1.c", *it++);
        ASSERT_EQUALS("file2.cpp", *it++);
        ASSERT_EQUALS("file.cpp", *it);
    }

    void fileListInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--file-list", "files.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--file-list\".\n", logger->str());
    }

    void inlineSuppr() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--inline-suppr", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->inlineSuppressions);
    }

    void jobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "3", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(3, settings->jobs);
    }

    void jobs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j3", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(3, settings->jobs);
    }

    void jobsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "file.cpp"};
        // Fails since -j is missing thread count
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not valid - not an integer.\n", logger->str());
    }

    void jobsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j", "e", "file.cpp"};
        // Fails since invalid count given for -j
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-j' is not valid - not an integer.\n", logger->str());
    }

    void jobsNoJobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j0", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument for '-j' must be greater than 0.\n", logger->str());
    }

    void jobsTooBig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j1025", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument for '-j' is allowed to be 1024 at max.\n", logger->str());
    }

    void maxConfigs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-f", "--max-configs=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->maxConfigsOption);
        ASSERT_EQUALS(false, settings->force);
    }

    void maxConfigsMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=", "file.cpp"};
        // Fails since --max-configs= is missing limit
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not valid - not an integer.\n", logger->str());
    }

    void maxConfigsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=e", "file.cpp"};
        // Fails since invalid count given for --max-configs=
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' is not valid - not an integer.\n", logger->str());
    }

    void maxConfigsTooSmall() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-configs=0", "file.cpp"};
        // Fails since limit must be greater than 0
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-configs=' must be greater than 0.\n", logger->str());
    }

    void outputFormatText() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--output-format=text", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::text, settings->outputFormat);
    }

    void outputFormatSarif() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--output-format=sarif", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::sarif, settings->outputFormat);
    }

    void outputFormatXml() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--output-format=xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
    }

    void outputFormatXmlv2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--output-format=xmlv2", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void outputFormatXmlv3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--output-format=xmlv3", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
        ASSERT_EQUALS(3, settings->xml_version);
    }

    void outputFormatOther() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--output-format=plist", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--output-format=' must be 'text', 'sarif', 'xml' (deprecated), 'xmlv2' or 'xmlv3'.\n", logger->str());
    }

    void outputFormatImplicitPlist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=.", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::plist, settings->outputFormat);
        ASSERT_EQUALS("./", settings->plistOutput);
    }

    void outputFormatImplicitXml() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
    }

    void outputFormatOverridePlist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=.", "--output-format=text", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::text, settings->outputFormat);
        ASSERT_EQUALS("", settings->plistOutput);
    }

    void outputFormatMixed1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--output-format=xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: '--output-format' and '--xml...' may not be used in conjunction.\n", logger->str());
    }

    void outputFormatMixed2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--output-format=xml", "--xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: '--output-format' and '--xml...' may not be used in conjunction.\n", logger->str());
    }

    void outputFormatMixed3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "--output-format=xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: '--output-format' and '--xml...' may not be used in conjunction.\n", logger->str());
    }

    void premiumOptions1() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=autosar", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
    }

    void premiumOptions2() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=misra-c-2012", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
    }

    void premiumOptions3() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=misra-c++-2023", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
    }

    void premiumOptions4() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=cert-c++-2016", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(true, settings->severity.isEnabled(Severity::warning));
    }

    void premiumOptions5() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=safety", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::warning));
    }

    void premiumOptionsAll() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {
            "cppcheck",
            "--premium=autosar:all",
            "--premium=cert-c:all",
            "--premium=cert-c++:all",
            "--premium=misra-c-2023:all",
            "--premium=misra-c++-2023:all",
            "file.c"
        };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("--autosar:all --cert-c:all --cert-c++:all --misra-c-2023:all --misra-c++-2023:all",
                      settings->premiumArgs);
    }

    void premiumOptionsMetrics() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=metrics", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("--metrics", settings->premiumArgs);
        ASSERT(settings->severity.isEnabled(Severity::error));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::warning));
        ASSERT_EQUALS(false, settings->severity.isEnabled(Severity::portability));
    }

    void premiumOptionsCertCIntPrecision() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium-cert-c-int-precision=12", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("--cert-c-int-precision=12", settings->premiumArgs);
    }

    void premiumOptionsLicenseFile() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium-license-file=file.lic", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("--license-file=file.lic", settings->premiumArgs);
    }

    void premiumOptionsInvalid1() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=misra", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: invalid --premium option 'misra'.\n", logger->str());
    }

    void premiumOptionsInvalid2() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=cert", "file.c"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: invalid --premium option 'cert'.\n", logger->str());
    }

    void premiumSafety() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=safety", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->safety);
    }

    void premiumDebugProgress() {
        REDIRECT;
        asPremium();
        const char * const argv[] = {"cppcheck", "--premium=debug-progress", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("--debug-progress", settings->premiumArgs);
    }

    void reportProgress1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(10, settings->reportProgress);
    }

    void reportProgress2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--report-progress=' is not valid - not an integer.\n", logger->str());
    }

    void reportProgress3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=-1", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--report-progress=' needs to be a positive integer.\n", logger->str());
    }

    void reportProgress4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=0", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(0, settings->reportProgress);
    }

    void reportProgress5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--report-progress=1", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->reportProgress);
    }

    void stdc99() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c99", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->standards.c == Standards::C99);
    }

    void stdgnu17() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=gnu17", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->standards.c == Standards::C17);
    }

    void stdiso9899_1999() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=iso9899:1999", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->standards.c == Standards::C99);
    }

    void stdcpp11() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=c++11", "file.cpp"};
        settings->standards.cpp = Standards::CPP03;
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->standards.cpp == Standards::CPP11);
    }

    void stdgnupp23() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--std=gnu++23", "file.cpp"};
        settings->standards.cpp = Standards::CPP03;
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->standards.cpp == Standards::CPP23);
    }

    void stdunknown1() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=d++11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unknown --std value 'd++11'\n", logger->str());
    }

    void stdunknown2() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=cplusplus11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unknown --std value 'cplusplus11'\n", logger->str());
    }

    void stdunknown3() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=C++11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unknown --std value 'C++11'\n", logger->str());
    }

    void stdoverwrite1() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=c++11", "--std=c++23", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Standards::CPP23, settings->standards.cpp);
        ASSERT_EQUALS_ENUM(Standards::CLatest, settings->standards.c);
    }

    void stdoverwrite2() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=c99", "--std=c11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Standards::C11, settings->standards.c);
        ASSERT_EQUALS_ENUM(Standards::CPPLatest, settings->standards.cpp);
    }

    void stdoverwrite3() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=c99", "--std=c++11", "--std=c11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Standards::C11, settings->standards.c);
        ASSERT_EQUALS_ENUM(Standards::CPP11, settings->standards.cpp);
    }

    void stdoverwrite4() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=c++11", "--std=c99", "--std=c++23", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Standards::CPP23, settings->standards.cpp);
        ASSERT_EQUALS_ENUM(Standards::C99, settings->standards.c);
    }

    void stdmulti1() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=c99", "--std=c++11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Standards::C99, settings->standards.c);
        ASSERT_EQUALS_ENUM(Standards::CPP11, settings->standards.cpp);
    }

    void stdmulti2() {
        REDIRECT;
        const char *const argv[] = {"cppcheck", "--std=c++20", "--std=c11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Standards::C11, settings->standards.c);
        ASSERT_EQUALS_ENUM(Standards::CPP20, settings->standards.cpp);
    }

    void platformWin64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win64", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::Win64, settings->platform.type);
    }

    void platformWin32A() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32A", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::Win32A, settings->platform.type);
    }

    void platformWin32W() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win32W", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::Win32W, settings->platform.type);
    }

    void platformUnix32() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::Unix32, settings->platform.type);
    }

    void platformUnix32Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix32-unsigned", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized platform: 'unix32-unsigned'.\n", logger->str());
    }

    void platformUnix64() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::Unix64, settings->platform.type);
    }

    void platformUnix64Unsigned() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unix64-unsigned", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized platform: 'unix64-unsigned'.\n", logger->str());
    }

    void platformNative() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=native", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::Native, settings->platform.type);
    }

    void platformUnspecified() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=unspecified", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Native));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::Unspecified, settings->platform.type);
    }

    void platformPlatformFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=avr8", "file.cpp"};
        ASSERT(settings->platform.set(Platform::Type::Unspecified));
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(Platform::Type::File, settings->platform.type);
    }

    void platformUnknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=win128", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized platform: 'win128'.\n", logger->str());
    }

    void platformEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: empty platform specified.\n", logger->str());
    }

    void plistEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->plistOutput == "./");
    }

    void plistDoesNotExist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--plist-output=./cppcheck_reports", "file.cpp"};
        // Fails since folder pointed by --plist-output= does not exist
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: plist folder does not exist: 'cppcheck_reports'.\n", logger->str());
    }

    void suppressionsOld() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions", "suppr.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--suppressions\".\n", logger->str());
    }

    void suppressions() {
        REDIRECT;
        ScopedFile file("suppr.txt",
                        "uninitvar\n"
                        "unusedFunction\n");
        const char * const argv[] = {"cppcheck", "--suppressions-list=suppr.txt", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        const auto supprsNomsg = supprs->nomsg.getSuppressions();
        ASSERT_EQUALS(2, supprsNomsg.size());
        auto it = supprsNomsg.cbegin();
        ASSERT_EQUALS("uninitvar", (it++)->errorId);
        ASSERT_EQUALS("unusedFunction", it->errorId);
    }

    void suppressionsNoFile1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS(false, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    void suppressionsNoFile2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr,b.suppr", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS(true, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    void suppressionsNoFile3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppressions-list=a.suppr b.suppr", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS(true, logger->str().find("If you want to pass two files") != std::string::npos);
    }

    static SuppressionList::ErrorMessage errorMessage(const std::string &errorId, const std::string &fileName, int lineNumber) {
        SuppressionList::ErrorMessage e;
        e.errorId = errorId;
        e.setFileName(fileName);
        e.lineNumber = lineNumber;
        return e;
    }

    void suppressSingle() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, supprs->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1)));
    }

    void suppressSingleFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar:file.cpp", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, supprs->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
    }

    void suppressTwo() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar,noConstructor", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Failed to add suppression. Invalid id \"uninitvar,noConstructor\"\n", logger->str());
    }

    void suppressTwoSeparate() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress=uninitvar", "--suppress=noConstructor", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, supprs->nomsg.isSuppressed(errorMessage("uninitvar", "file.cpp", 1U)));
        ASSERT_EQUALS(true, supprs->nomsg.isSuppressed(errorMessage("noConstructor", "file.cpp", 1U)));
    }

    void templates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template={file}:{line},{severity},{id},{message}", "--template-location={file}:{line}:{column} {info}", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}:{line},{severity},{id},{message}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column} {info}", settings->templateLocation);
    }

    void templatesGcc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=gcc", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}:{line}:{column}: warning: {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    void templatesVs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=vs", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}({line}): {severity}: {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templatesEdit() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=edit", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file} +{line}: {severity}: {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templatesCppcheck1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=cppcheck1", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{callstack}: ({severity}{inconclusive:, inconclusive}) {message}", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templatesDaca2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=daca2", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}", settings->templateLocation);
        ASSERT_EQUALS(true, settings->daca);
    }

    void templatesSelfcheck() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=selfcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    void templatesSimple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=simple", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    // TODO: we should bail out on this
    void templatesNoPlaceholder() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template=selfchek", "file.cpp"};
        TODO_ASSERT_EQUALS(static_cast<int>(CmdLineParser::Result::Fail), static_cast<int>(CmdLineParser::Result::Success), static_cast<int>(parseFromArgs(argv)));
        ASSERT_EQUALS("selfchek", settings->templateFormat);
        ASSERT_EQUALS("", settings->templateLocation);
    }

    void templateFormatInvalid() {
        REDIRECT;
        const char* const argv[] = { "cppcheck", "--template", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--template\".\n", logger->str());
    }

    // will use the default
    // TODO: bail out on empty?
    void templateFormatEmpty() {
        REDIRECT;
        const char* const argv[] = { "cppcheck", "--template=", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {inconclusive:}{severity}:{inconclusive: inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    void templateLocationInvalid() {
        REDIRECT;
        const char* const argv[] = { "cppcheck", "--template-location", "--template={file}", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--template-location\".\n", logger->str());
    }

    // will use the default
    // TODO: bail out on empty?
    void templateLocationEmpty() {
        REDIRECT;
        const char* const argv[] = { "cppcheck", "--template-location=", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS("{file}:{line}:{column}: {inconclusive:}{severity}:{inconclusive: inconclusive:} {message} [{id}]\n{code}", settings->templateFormat);
        ASSERT_EQUALS("{file}:{line}:{column}: note: {info}\n{code}", settings->templateLocation);
    }

    void xml() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void xmlver2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void xmlver2both() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=2", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void xmlver2both2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml-version=2", "--xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::OutputFormat::xml, settings->outputFormat);
        ASSERT_EQUALS(2, settings->xml_version);
    }

    void xmlverunknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=4", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: '--xml-version' can only be 2 or 3.\n", logger->str());
    }

    void xmlverinvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--xml", "--xml-version=a", "file.cpp"};
        // FAils since unknown XML format version
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--xml-version=' is not valid - not an integer.\n", logger->str());
    }

    void doc() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--doc"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(startsWith(logger->str(), "## "));
    }

    void docExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--doc"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        ASSERT(startsWith(logger->str(), "## "));
    }

    void showtimeSummary() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=summary", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->showtime == ShowTime::SUMMARY);
    }

    void showtimeFile() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=file", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->showtime == ShowTime::FILE);
    }

    void showtimeFileTotal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=file-total", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->showtime == ShowTime::FILE_TOTAL);
    }

    void showtimeTop5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized --showtime mode: 'top5'. Supported modes: file, file-total, summary, top5_file, top5_summary.\n", logger->str());
    }

    void showtimeTop5File() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5_file", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->showtime == ShowTime::TOP5_FILE);
    }

    void showtimeTop5Summary() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top5_summary", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->showtime == ShowTime::TOP5_SUMMARY);
    }

    void showtimeNone() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=none", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->showtime == ShowTime::NONE);
    }

    void showtimeEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: no mode provided for --showtime\n", logger->str());
    }

    void showtimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--showtime=top10", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized --showtime mode: 'top10'. Supported modes: file, file-total, summary, top5_file, top5_summary.\n", logger->str());
    }

    void errorlist() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        const std::string errout_s = GET_REDIRECT_OUTPUT;
        ASSERT_EQUALS("", logger->str()); // empty since it is logged via ErrorLogger
        ASSERT(startsWith(errout_s, ErrorMessage::getXMLHeader("")));
        ASSERT(endsWith(errout_s, "</results>\n"));
    }

    void errorlistWithCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        R"({"productName": "The Product"}\n)");
        const char * const argv[] = {"cppcheck", "--errorlist"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        const std::string errout_s = GET_REDIRECT_OUTPUT;
        ASSERT_EQUALS(1, settings->settingsFiles.size());
        ASSERT_EQUALS(file.path(), *settings->settingsFiles.cbegin());
        ASSERT_EQUALS("", logger->str()); // empty since it is logged via ErrorLogger
        ASSERT(startsWith(errout_s, ErrorMessage::getXMLHeader("The Product")));
    }

    void errorlistExclusive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=missing", "--errorlist"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
        const std::string errout_s = GET_REDIRECT_OUTPUT;
        ASSERT_EQUALS("", logger->str()); // empty since it is logged via ErrorLogger
        ASSERT(startsWith(errout_s, ErrorMessage::getXMLHeader("")));
        ASSERT(endsWith(errout_s, "</results>\n"));
    }

    void ignorepathsnopath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i"};
        // Fails since no ignored path given
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-i' is missing.\n", logger->str());
    }

#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
    void exceptionhandling() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stdout, settings->exceptionOutput);
    }

    void exceptionhandling2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stderr", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stderr, settings->exceptionOutput);
    }

    void exceptionhandling3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stdout", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->exceptionHandling);
        ASSERT_EQUALS(stdout, settings->exceptionOutput);
    }

    void exceptionhandlingInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=exfile"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: invalid '--exception-handling' argument\n", logger->str());
    }

    void exceptionhandlingInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling-foo"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--exception-handling-foo\".\n", logger->str());
    }
#else
    void exceptionhandlingNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.\n", logger->str());
    }

    void exceptionhandlingNotSupported2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--exception-handling=stderr", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.\n", logger->str());
    }
#endif

    void clang() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->clang);
        ASSERT_EQUALS("clang", settings->clangExecutable);
    }

    void clangCustom() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang=clang-14", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->clang);
        ASSERT_EQUALS("clang-14", settings->clangExecutable);
    }

    void clangInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang-foo"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--clang-foo\".\n", logger->str());
    }

    void valueFlowMaxIterations() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=0", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(0, settings->vfOptions.maxIterations);
    }

    void valueFlowMaxIterations2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=11", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(11, settings->vfOptions.maxIterations);
    }

    void valueFlowMaxIterationsInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--valueflow-max-iterations\".\n", logger->str());
    }

    void valueFlowMaxIterationsInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=seven"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iterations=' is not valid - not an integer.\n", logger->str());
    }

    void valueFlowMaxIterationsInvalid3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--valueflow-max-iterations=-1"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--valueflow-max-iterations=' is not valid - needs to be positive.\n", logger->str());
    }

    void checksMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->checksMaxTime);
    }

    void checksMaxTime2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=-1", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--checks-max-time=' needs to be a positive integer.\n", logger->str());
    }

    void checksMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--checks-max-time=one", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--checks-max-time=' is not valid - not an integer.\n", logger->str());
    }

#ifdef HAS_THREADING_MODEL_FORK
    void loadAverage() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->loadAverage);
    }

    void loadAverage2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->loadAverage);
    }

    void loadAverageInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "one", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-l' is not valid - not an integer.\n", logger->str());
    }
#else
    void loadAverageNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-l", "12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Option -l cannot be used as Cppcheck has not been built with fork threading model.\n", logger->str());
    }
#endif

    void maxCtuDepth() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=5", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(5, settings->maxCtuDepth);
    }

    void maxCtuDepth2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=10", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(10, settings->maxCtuDepth);
    }

    void maxCtuDepthLimit() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(10, settings->maxCtuDepth);
        ASSERT_EQUALS("cppcheck: --max-ctu-depth is being capped at 10. This limitation will be removed in a future Cppcheck version.\n", logger->str());
    }

    void maxCtuDepthInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-ctu-depth=one", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-ctu-depth=' is not valid - not an integer.\n", logger->str());
    }

    void performanceValueflowMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-time=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->vfOptions.maxTime);
    }

    void performanceValueflowMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-time=one", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--performance-valueflow-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void performanceValueFlowMaxIfCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-if-count=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->vfOptions.maxIfCount);
    }

    void performanceValueFlowMaxIfCountInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--performance-valueflow-max-if-count=one", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--performance-valueflow-max-if-count=' is not valid - not an integer.\n", logger->str());
    }

    void templateMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->templateMaxTime);
    }

    void templateMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=one", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--template-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void templateMaxTimeInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--template-max-time=-1", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--template-max-time=' is not valid - needs to be positive.\n", logger->str());
    }

    void typedefMaxTime() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->typedefMaxTime);
    }

    void typedefMaxTimeInvalid() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=one", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--typedef-max-time=' is not valid - not an integer.\n", logger->str());
    }

    void typedefMaxTimeInvalid2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--typedef-max-time=-1", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--typedef-max-time=' is not valid - needs to be positive.\n", logger->str());
    }

    void project() {
        REDIRECT;
        ScopedFile file("project.cppcheck",
                        "<project>\n"
                        "<paths>\n"
                        "<dir name=\"dir\"/>\n"
                        "</paths>\n"
                        "</project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mPathNames.size());
        auto it = parser->mPathNames.cbegin();
        ASSERT_EQUALS("dir", *it);
    }

    void projectMultiple() {
        REDIRECT;
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: multiple --project options are not supported.\n", logger->str());
    }

    void projectAndSource() {
        REDIRECT;
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --project cannot be used in conjunction with source files.\n", logger->str());
    }

    void projectEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: failed to open project ''. The file does not exist.\n", logger->str());
    }

    void projectMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: failed to open project 'project.cppcheck'. The file does not exist.\n", logger->str());
    }

    void projectNoPaths() {
        ScopedFile file("project.cppcheck", "<project></project>");
        const char * const argv[] = {"cppcheck", "--project=project.cppcheck"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: no C or C++ source files found.\n", logger->str());
    }

    void addon() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--addon=misra", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->addons.size());
        ASSERT_EQUALS("misra", *settings->addons.cbegin());
    }

    void addonMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--addon=misra2", "file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, settings->addons.size());
        ASSERT_EQUALS("misra2", *settings->addons.cbegin());
        ASSERT_EQUALS("Did not find addon misra2.py\n", logger->str());
    }

    void signedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--fsigned-char", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS('s', settings->platform.defaultSign);
    }

    void signedChar2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=avr8", "--fsigned-char", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS('s', settings->platform.defaultSign);
    }

    void unsignedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--funsigned-char", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
    }

    void unsignedChar2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--platform=mips32", "--funsigned-char", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
    }

    void signedCharUnsignedChar() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--fsigned-char", "--funsigned-char", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS('u', settings->platform.defaultSign);
    }

#ifdef HAVE_RULES
    void rule() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=.+", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->rules.size());
        auto it = settings->rules.cbegin();
        ASSERT_EQUALS(".+", it->pattern);
    }

    void ruleMissingPattern() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: no rule pattern provided.\n", logger->str());
    }

    void ruleInvalidPattern() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=.*\\", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: failed to compile rule pattern '.*\\' (pcre_compile failed: \\ at end of pattern).\n", logger->str());
    }
#else
    void ruleNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule=.+", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Option --rule cannot be used as Cppcheck has not been built with rules support.\n", logger->str());
    }
#endif

#ifdef HAVE_RULES
    void ruleFileMulti() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rules>\n"
                        "<rule>\n"
                        "<tokenlist>raw</tokenlist>\n"
                        "<pattern>.+</pattern>\n"
                        "<message>\n"
                        "<severity>error</severity>\n"
                        "<id>ruleId1</id>\n"
                        "<summary>ruleSummary1</summary>\n"
                        "</message>\n"
                        "</rule>\n"
                        "<rule>\n"
                        "<tokenlist>define</tokenlist>\n"
                        "<pattern>.*</pattern>\n"
                        "<message>\n"
                        "<severity>warning</severity>\n"
                        "<id>ruleId2</id>\n"
                        "<summary>ruleSummary2</summary>\n"
                        "</message>\n"
                        "</rule>\n"
                        "</rules>");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(2, settings->rules.size());
        auto it = settings->rules.cbegin();
        ASSERT_EQUALS("raw", it->tokenlist);
        ASSERT_EQUALS(".+", it->pattern);
        ASSERT_EQUALS_ENUM(Severity::error, it->severity);
        ASSERT_EQUALS("ruleId1", it->id);
        ASSERT_EQUALS("ruleSummary1", it->summary);
        ++it;
        ASSERT_EQUALS("define", it->tokenlist);
        ASSERT_EQUALS(".*", it->pattern);
        ASSERT_EQUALS_ENUM(Severity::warning, it->severity);
        ASSERT_EQUALS("ruleId2", it->id);
        ASSERT_EQUALS("ruleSummary2", it->summary);
    }

    void ruleFileSingle() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>\n"
                        "<tokenlist>define</tokenlist>\n"
                        "<pattern>.+</pattern>\n"
                        "<message>\n"
                        "<severity>error</severity>\n"
                        "<id>ruleId</id>\n"
                        "<summary>ruleSummary</summary>\n"
                        "</message>\n"
                        "</rule>\n");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->rules.size());
        auto it = settings->rules.cbegin();
        ASSERT_EQUALS("define", it->tokenlist);
        ASSERT_EQUALS(".+", it->pattern);
        ASSERT_EQUALS_ENUM(Severity::error, it->severity);
        ASSERT_EQUALS("ruleId", it->id);
        ASSERT_EQUALS("ruleSummary", it->summary);
    }

    void ruleFileEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file '' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void ruleFileMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void ruleFileInvalid() {
        REDIRECT;
        ScopedFile file("rule.xml", "");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' (XML_ERROR_EMPTY_DOCUMENT).\n", logger->str());
    }

    void ruleFileNoRoot() {
        REDIRECT;
        ScopedFile file("rule.xml", "<?xml version=\"1.0\"?>");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(0, settings->rules.size());
    }

    void ruleFileEmptyElements1() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>"
                        "<tokenlist/>"
                        "<pattern/>"
                        "<message/>"
                        "</rule>"
                        );
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv)); // do not crash
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - a rule is lacking a pattern.\n", logger->str());
    }

    void ruleFileEmptyElements2() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>"
                        "<message>"
                        "<severity/>"
                        "<id/>"
                        "<summary/>"
                        "</message>"
                        "</rule>"
                        );
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv)); // do not crash
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - a rule is lacking a pattern.\n", logger->str());
    }

    void ruleFileUnknownElement1() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>"
                        "<messages/>"
                        "</rule>"
                        );
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - unknown element 'messages' encountered in 'rule'.\n", logger->str());
    }

    void ruleFileUnknownElement2() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>"
                        "<message>"
                        "<pattern/>"
                        "</message>"
                        "</rule>"
                        );
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - unknown element 'pattern' encountered in 'message'.\n", logger->str());
    }

    void ruleFileMissingTokenList() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>\n"
                        "<tokenlist/>\n"
                        "<pattern>.+</pattern>\n"
                        "</rule>\n");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - a rule is lacking a tokenlist.\n", logger->str());
    }

    void ruleFileUnknownTokenList() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>\n"
                        "<tokenlist>simple</tokenlist>\n"
                        "<pattern>.+</pattern>\n"
                        "</rule>\n");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - a rule is using the unsupported tokenlist 'simple'.\n", logger->str());
    }

    void ruleFileMissingId() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>\n"
                        "<pattern>.+</pattern>\n"
                        "<message>\n"
                        "<id/>"
                        "</message>\n"
                        "</rule>\n");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - a rule is lacking an id.\n", logger->str());
    }

    void ruleFileInvalidSeverity1() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>\n"
                        "<pattern>.+</pattern>\n"
                        "<message>\n"
                        "<severity/>"
                        "</message>\n"
                        "</rule>\n");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - a rule has an invalid severity.\n", logger->str());
    }

    void ruleFileInvalidSeverity2() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>\n"
                        "<pattern>.+</pattern>\n"
                        "<message>\n"
                        "<severity>none</severity>"
                        "</message>\n"
                        "</rule>\n");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - a rule has an invalid severity.\n", logger->str());
    }

    void ruleFileInvalidPattern() {
        REDIRECT;
        ScopedFile file("rule.xml",
                        "<rule>\n"
                        "<pattern>.+\\</pattern>\n"
                        "</rule>\n");
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parser->parseFromArgs(3, argv));
        ASSERT_EQUALS("cppcheck: error: unable to load rule-file 'rule.xml' - pattern '.+\\' failed to compile (pcre_compile failed: \\ at end of pattern).\n", logger->str());
    }
#else
    void ruleFileNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--rule-file=rule.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Option --rule-file cannot be used as Cppcheck has not been built with rules support.\n", logger->str());
    }
#endif

    void library() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=posix", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->libraries.size());
        ASSERT_EQUALS("posix", *settings->libraries.cbegin());
    }

    void libraryMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=posix2", "file.cpp"};
        ASSERT_EQUALS(false, fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, settings->libraries.size());
        ASSERT_EQUALS("posix2", *settings->libraries.cbegin());
        ASSERT_EQUALS("cppcheck: Failed to load library configuration file 'posix2'. File not found\n", logger->str());
    }

    void libraryMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=posix,gnu", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(2, settings->libraries.size());
        auto it = settings->libraries.cbegin();
        ASSERT_EQUALS("posix", *it++);
        ASSERT_EQUALS("gnu", *it);
    }

    void libraryMultipleEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=posix,,gnu", "file.cpp"};
        ASSERT_EQUALS(false, fillSettingsFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: empty library specified.\n", logger->str());
    }

    void libraryMultipleEmpty2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--library=posix,gnu,", "file.cpp"};
        ASSERT_EQUALS(false, fillSettingsFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: empty library specified.\n", logger->str());
    }

    void suppressXml() {
        REDIRECT;
        ScopedFile file("suppress.xml",
                        "<suppressions>\n"
                        "<suppress>\n"
                        "<id>uninitvar</id>\n"
                        "</suppress>\n"
                        "</suppressions>");
        const char * const argv[] = {"cppcheck", "--suppress-xml=suppress.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        const auto supprsNomsg = supprs->nomsg.getSuppressions();
        ASSERT_EQUALS(1, supprsNomsg.size());
        const auto it = supprsNomsg.cbegin();
        ASSERT_EQUALS("uninitvar", it->errorId);
    }

    void suppressXmlEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress-xml=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: failed to load suppressions XML '' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void suppressXmlMissing() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--suppress-xml=suppress.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: failed to load suppressions XML 'suppress.xml' (XML_ERROR_FILE_NOT_FOUND).\n", logger->str());
    }

    void suppressXmlInvalid() {
        REDIRECT;
        ScopedFile file("suppress.xml", "");
        const char * const argv[] = {"cppcheck", "--suppress-xml=suppress.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: failed to load suppressions XML 'suppress.xml' (XML_ERROR_EMPTY_DOCUMENT).\n", logger->str());
    }

    void suppressXmlNoRoot() {
        REDIRECT;
        ScopedFile file("suppress.xml", "<?xml version=\"1.0\"?>");
        const char * const argv[] = {"cppcheck", "--suppress-xml=suppress.xml", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: failed to load suppressions XML 'suppress.xml' (no root node found).\n", logger->str());
    }

    void executorDefault() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
#if defined(HAS_THREADING_MODEL_FORK)
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Process, settings->executor);
#elif defined(HAS_THREADING_MODEL_THREAD)
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Thread, settings->executor);
#endif
    }

    void executorAuto() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j2", "--executor=auto", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
#if defined(HAS_THREADING_MODEL_FORK)
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Process, settings->executor);
#elif defined(HAS_THREADING_MODEL_THREAD)
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Thread, settings->executor);
#endif
    }

    void executorAutoNoJobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--executor=auto", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
#if defined(HAS_THREADING_MODEL_FORK)
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Process, settings->executor);
#elif defined(HAS_THREADING_MODEL_THREAD)
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Thread, settings->executor);
#endif
    }

#if defined(HAS_THREADING_MODEL_THREAD)
    void executorThread() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j2", "--executor=thread", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Thread, settings->executor);
    }

    void executorThreadNoJobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--executor=thread", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Thread, settings->executor);
        ASSERT_EQUALS("cppcheck: '--executor' has no effect as only a single job will be used.\n", logger->str());
    }
#else
    void executorThreadNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j2", "--executor=thread", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: executor type 'thread' cannot be used as Cppcheck has not been built with a respective threading model.\n", logger->str());
    }
#endif

#if defined(HAS_THREADING_MODEL_FORK)
    void executorProcess() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j2", "--executor=process", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Process, settings->executor);
    }

    void executorProcessNoJobs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--executor=process", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::ExecutorType::Process, settings->executor);
        ASSERT_EQUALS("cppcheck: '--executor' has no effect as only a single job will be used.\n", logger->str());
    }
#else
    void executorProcessNotSupported() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-j2", "--executor=process", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: executor type 'process' cannot be used as Cppcheck has not been built with a respective threading model.\n", logger->str());
    }
#endif

    // the CLI default to --check-level=normal
    void checkLevelDefault() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::CheckLevel::normal, settings->checkLevel);
        ASSERT_EQUALS(100, settings->vfOptions.maxIfCount);
        ASSERT_EQUALS(8, settings->vfOptions.maxSubFunctionArgs);
        ASSERT_EQUALS(false, settings->vfOptions.doConditionExpressionAnalysis);
        ASSERT_EQUALS(4, settings->vfOptions.maxForwardBranches);
    }

    void checkLevelNormal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-level=normal", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::CheckLevel::normal, settings->checkLevel);
        ASSERT_EQUALS(100, settings->vfOptions.maxIfCount);
        ASSERT_EQUALS(8, settings->vfOptions.maxSubFunctionArgs);
        ASSERT_EQUALS(false, settings->vfOptions.doConditionExpressionAnalysis);
        ASSERT_EQUALS(4, settings->vfOptions.maxForwardBranches);
    }

    void checkLevelExhaustive() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-level=exhaustive", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(Settings::CheckLevel::exhaustive, settings->checkLevel);
        ASSERT_EQUALS(-1, settings->vfOptions.maxIfCount);
        ASSERT_EQUALS(256, settings->vfOptions.maxSubFunctionArgs);
        ASSERT_EQUALS(true, settings->vfOptions.doConditionExpressionAnalysis);
        ASSERT_EQUALS(-1, settings->vfOptions.maxForwardBranches);
    }

    void checkLevelUnknown() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-level=default", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unknown '--check-level' value 'default'.\n", logger->str());
    }

    void cppHeaderProbe() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cpp-header-probe", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->cppHeaderProbe);
    }

    void cppHeaderProbe2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-cpp-header-probe", "--cpp-header-probe", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->cppHeaderProbe);
    }

    void noCppHeaderProbe() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-cpp-header-probe", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(false, settings->cppHeaderProbe);
    }

    void noCppHeaderProbe2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cpp-header-probe", "--no-cpp-header-probe", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(false, settings->cppHeaderProbe);
    }

    void debugLookup() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-lookup", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        GET_REDIRECT_OUTPUT; // ignore config lookup output
        ASSERT_EQUALS(true, settings->debuglookup);
    }

    void debugLookupAll() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-lookup=all", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        GET_REDIRECT_OUTPUT; // ignore config lookup output
        ASSERT_EQUALS(true, settings->debuglookup);
    }

    void debugLookupAddon() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-lookup=addon", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debuglookupAddon);
    }

    void debugLookupConfig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-lookup=config", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        GET_REDIRECT_OUTPUT; // ignore config lookup output
        ASSERT_EQUALS(true, settings->debuglookupConfig);
    }

    void debugLookupLibrary() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-lookup=library", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debuglookupLibrary);
    }

    void debugLookupPlatform() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-lookup=platform", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debuglookupPlatform);
    }

    void maxTemplateRecursion() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-template-recursion=12", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(12, settings->maxTemplateRecursion);
    }

    void maxTemplateRecursionMissingCount() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--max-template-recursion=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '--max-template-recursion=' is not valid - not an integer.\n", logger->str());
    }

    void emitDuplicates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--emit-duplicates", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->emitDuplicates);
    }

    void debugClangOutput() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-clang-output", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugClangOutput);
    }

    void debugXmlMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug", "--xml", "1.cpp", "2.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: printing debug output in XML format does not support multiple input files.\n", logger->str());
    }

    void debugNormalXmlMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-normal", "--xml", "1.cpp", "2.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: printing debug output in XML format does not support multiple input files.\n", logger->str());
    }

    void debugIgnore() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-ignore", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugignore);
    }

    void checkHeaders() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-headers", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->checkHeaders);
    }

    void noCheckHeaders() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-check-headers", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(false, settings->checkHeaders);
    }

    void noCheckHeaders2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-headers", "--no-check-headers", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(false, settings->checkHeaders);
    }

    void filesdir() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--filesdir"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Exit, parseFromArgs(argv));
#ifdef FILESDIR
        ASSERT_EQUALS(std::string(FILESDIR) + '\n', logger->str());
#else
        ASSERT_EQUALS("", logger->str());
#endif
    }

    void checkUnusedTemplates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-unused-templates", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->checkUnusedTemplates);
    }

    void noCheckUnusedTemplates() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-check-unused-templates", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs( argv));
        ASSERT_EQUALS(false, settings->checkUnusedTemplates);
    }

    void noCheckUnusedTemplates2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-unused-templates", "--no-check-unused-templates", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(false, settings->checkUnusedTemplates);
    }

    void clangTidy() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang-tidy", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->clangTidy);
        ASSERT_EQUALS("clang-tidy", settings->clangTidyExecutable);
    }

    void clangTidyCustom() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--clang-tidy=clang-tidy-14", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->clangTidy);
        ASSERT_EQUALS("clang-tidy-14", settings->clangTidyExecutable);
    }

    void projectConfigurationNoProject() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project-configuration=Debug|Win32", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --project-configuration has no effect - no Visual Studio project provided.\n", logger->str());
    }

    void projectConfigurationEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--project-configuration=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --project-configuration parameter is empty.\n", logger->str());
    }

    void analyzeAllVsConfigs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--analyze-all-vs-configs", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->analyzeAllVsConfigs);
        ASSERT(parser->mAnalyzeAllVsConfigsSetOnCmdLine);
    }

    void noAnalyzeAllVsConfigs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-analyze-all-vs-configs", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --no-analyze-all-vs-configs has no effect - no Visual Studio project provided.\n", logger->str());
        ASSERT(parser->mAnalyzeAllVsConfigsSetOnCmdLine);
    }

    void noAnalyzeAllVsConfigs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--analyze-all-vs-configs", "--no-analyze-all-vs-configs", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: --no-analyze-all-vs-configs has no effect - no Visual Studio project provided.\n", logger->str());
        ASSERT(parser->mAnalyzeAllVsConfigsSetOnCmdLine);
    }

    void debugSymdb() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-symdb", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugsymdb);
    }

    void debugAst() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-ast", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugast);
    }

    void debugValueflow() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-valueflow", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugvalueflow);
    }

    void debugNormal() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-normal", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugnormal);
        ASSERT_EQUALS(false, settings->debugSimplified);
        ASSERT_EQUALS(true, settings->debugvalueflow);
        ASSERT_EQUALS(false, settings->debugast);
        ASSERT_EQUALS(false, settings->debugsymdb);
    }

    void debugNormalVerbose() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-normal", "--verbose", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugnormal);
        ASSERT_EQUALS(false, settings->debugSimplified);
        ASSERT_EQUALS(true, settings->debugvalueflow);
        ASSERT_EQUALS(true, settings->debugast);
        ASSERT_EQUALS(true, settings->debugsymdb);
    }

    void debug() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugnormal);
        ASSERT_EQUALS(false, settings->debugSimplified);
        ASSERT_EQUALS(true, settings->debugvalueflow);
        ASSERT_EQUALS(false, settings->debugast);
        ASSERT_EQUALS(false, settings->debugsymdb);
    }

    void debugVerbose() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug", "--verbose", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugnormal);
        ASSERT_EQUALS(false, settings->debugSimplified);
        ASSERT_EQUALS(true, settings->debugvalueflow);
        ASSERT_EQUALS(true, settings->debugast);
        ASSERT_EQUALS(true, settings->debugsymdb);
    }

    void safety() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--safety", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->safety);
    }

    void safetyOverride() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-safety", "--safety", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->safety);
    }

    void noSafety() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-safety", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(false, settings->safety);
    }

    void noSafetyOverride() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--safety", "--no-safety", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(false, settings->safety);
    }

    void debugAnalyzerinfo() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--debug-analyzerinfo", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->debugainfo);
    }

    void ignorepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void ignorepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void ignorepaths3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc", "-imodule", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(2, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS("module", parser->mIgnoredPaths[1]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void ignorepaths4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "src", "-i", "module", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(2, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS("module", parser->mIgnoredPaths[1]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void ignorefilepaths1() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-ifoo.cpp", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("foo.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void ignorefilepaths2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc/foo.cpp", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src/foo.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void ignorefilepaths3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-i", "foo.cpp", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("foo.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
    }

    void ignorefilepaths4() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-ifoo.cpp", "file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("foo.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
        TODO_ASSERT_EQUALS("cppcheck: error: could not find or open any of the paths given.\n", "cppcheck: error: could not find or open any of the paths given.\ncppcheck: Maybe all paths were ignored?\n", logger->str());
    }

    void ignorefilepaths5() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-ifile.cpp", "file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("file.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("file.cpp", parser->mPathNames[0]);
        ASSERT_EQUALS("cppcheck: error: could not find or open any of the paths given.\ncppcheck: Maybe all paths were ignored?\n", logger->str());
    }

    void ignorefilepaths6() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc/file.cpp", "src/file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src/file.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("src/file.cpp", parser->mPathNames[0]);
        ASSERT_EQUALS("cppcheck: error: could not find or open any of the paths given.\ncppcheck: Maybe all paths were ignored?\n", logger->str());
    }

    void ignorefilepaths7() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc\\file.cpp", "src/file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src/file.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("src/file.cpp", parser->mPathNames[0]);
        ASSERT_EQUALS("cppcheck: error: could not find or open any of the paths given.\ncppcheck: Maybe all paths were ignored?\n", logger->str());
    }

    void ignorefilepaths8() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc/file.cpp", "src\\file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src/file.cpp", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("src/file.cpp", parser->mPathNames[0]);
        ASSERT_EQUALS("cppcheck: error: could not find or open any of the paths given.\ncppcheck: Maybe all paths were ignored?\n", logger->str());
    }

    void ignorefilepaths9() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-isrc\\", "src\\file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS(1, parser->mIgnoredPaths.size());
        ASSERT_EQUALS("src/", parser->mIgnoredPaths[0]);
        ASSERT_EQUALS(1, parser->mPathNames.size());
        ASSERT_EQUALS("src/file.cpp", parser->mPathNames[0]);
        ASSERT_EQUALS("cppcheck: error: could not find or open any of the paths given.\ncppcheck: Maybe all paths were ignored?\n", logger->str());
    }

    void nonexistentpath() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "file.cpp"};
        ASSERT(!fillSettingsFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: could not find or open any of the paths given.\n", logger->str());
    }

    void checkconfig() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--check-config", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(true, settings->checkConfiguration);
    }

    void unknownParam() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--foo", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: unrecognized command line option: \"--foo\".\n", logger->str());
    }

    void undefs() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(1, settings->userUndefs.size());
        ASSERT(settings->userUndefs.find("_WIN32") != settings->userUndefs.end());
    }

    void undefs2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U_WIN32", "-UNODEBUG", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(2, settings->userUndefs.size());
        ASSERT(settings->userUndefs.find("_WIN32") != settings->userUndefs.end());
        ASSERT(settings->userUndefs.find("NODEBUG") != settings->userUndefs.end());
    }

    void undefs_noarg() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U"};
        // Fails since -U has no param
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void undefs_noarg2() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "-v", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void undefs_noarg3() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "-U", "--quiet", "file.cpp"};
        // Fails since -U has no param
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: argument to '-U' is missing.\n", logger->str());
    }

    void cppcheckBuildDirExistent() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=.", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(".", settings->buildDir);
    }

    void cppcheckBuildDirNonExistent() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=non-existent-path", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Directory 'non-existent-path' specified by --cppcheck-build-dir argument has to be existent.\n", logger->str());
    }

    void cppcheckBuildDirEmpty() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: no path has been specified for --cppcheck-build-dir\n", logger->str());
    }

    void cppcheckBuildDirMultiple() {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=non-existent-path", "--cppcheck-build-dir=.", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS(".", settings->buildDir);
    }

    void noCppcheckBuildDir()
    {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--no-cppcheck-build-dir", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->buildDir.empty());
    }

    void noCppcheckBuildDir2()
    {
        REDIRECT;
        const char * const argv[] = {"cppcheck", "--cppcheck-build-dir=b1", "--no-cppcheck-build-dir", "file.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT(settings->buildDir.empty());
    }

    void invalidCppcheckCfg() {
        REDIRECT;
        ScopedFile file(Path::join(Path::getPathFromFilename(Path::getCurrentExecutablePath("")), "cppcheck.cfg"),
                        "{\n");
        const char * const argv[] = {"cppcheck", "test.cpp"};
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: could not load cppcheck.cfg - not a valid JSON - syntax error at line 2 near: \n", logger->str());
    }

    void reportTypeAutosar() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=autosar", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::autosar, settings->reportType);
    }

    void reportTypeCertCpp() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=cert-cpp-2016", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::certCpp, settings->reportType);
    }

    void reportTypeCertC() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=cert-c-2016", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::certC, settings->reportType);
    }

    void reportTypeMisraC2012() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=misra-c-2012", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::misraC2012, settings->reportType);
    }

    void reportTypeMisraC2023() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=misra-c-2023", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::misraC2023, settings->reportType);
    }

    void reportTypeMisraC2025() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=misra-c-2025", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::misraC2025, settings->reportType);
    }

    void reportTypeMisraCpp2008() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=misra-cpp-2008", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::misraCpp2008, settings->reportType);
    }

    void reportTypeMisraCpp2023() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=misra-cpp-2023", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::misraCpp2023, settings->reportType);
    }

    void invalidReportType() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "--report-type=invalid", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Fail, parseFromArgs(argv));
        ASSERT_EQUALS("cppcheck: error: Unknown report type 'invalid'\n", logger->str());
    }

    void defaultReportType() {
        REDIRECT;
        const char *const argv[] = { "cppcheck", "file.cpp" };
        ASSERT_EQUALS_ENUM(CmdLineParser::Result::Success, parseFromArgs(argv));
        ASSERT_EQUALS_ENUM(ReportType::normal, settings->reportType);
    }
};

REGISTER_TEST(TestCmdlineParser)
