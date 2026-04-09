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

#include "cmdlineparser.h"

#include "addoninfo.h"
#include "check.h"
#include "checkers.h"
#include "color.h"
#include "config.h"
#include "cppcheck.h"
#include "errorlogger.h"
#include "errortypes.h"
#include "filelister.h"
#include "filesettings.h"
#include "importproject.h"
#include "library.h"
#include "path.h"
#include "pathmatch.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "suppressions.h"
#include "timer.h"
#include "utils.h"

#include "frontend.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib> // EXIT_FAILURE
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_set>
#include <utility>

#ifdef HAVE_RULES
#include "regex.h"

// xml is used for rules
#include "xml.h"
#endif

static bool addFilesToList(const std::string& fileList, std::vector<std::string>& pathNames)
{
    std::istream *files;
    std::ifstream infile;
    if (fileList == "-") { // read from stdin
        files = &std::cin;
    } else {
        infile.open(fileList);
        if (!infile.is_open())
            return false;
        files = &infile;
    }
    std::string fileName;
    // cppcheck-suppress accessMoved - FP
    while (std::getline(*files, fileName)) { // next line
        // cppcheck-suppress accessMoved - FP
        if (!fileName.empty()) {
            pathNames.emplace_back(std::move(fileName));
        }
    }

    return true;
}

static bool addIncludePathsToList(const std::string& fileList, std::list<std::string>& pathNames)
{
    std::ifstream files(fileList);
    if (files) {
        std::string pathName;
        // cppcheck-suppress accessMoved - FP
        while (std::getline(files, pathName)) { // next line
            if (!pathName.empty()) {
                pathName = Path::removeQuotationMarks(std::move(pathName));
                pathName = Path::fromNativeSeparators(std::move(pathName));

                // If path doesn't end with / or \, add it
                if (!endsWith(pathName, '/'))
                    pathName += '/';

                pathNames.emplace_back(std::move(pathName));
            }
        }
        return true;
    }
    return false;
}

static bool addPathsToSet(const std::string& fileName, std::set<std::string>& set)
{
    std::list<std::string> templist;
    if (!addIncludePathsToList(fileName, templist))
        return false;
    set.insert(templist.cbegin(), templist.cend());
    return true;
}

namespace {
    class XMLErrorMessagesLogger : public ErrorLogger
    {
        void reportOut(const std::string & outmsg, Color /*c*/ = Color::Reset) override
        {
            std::cout << outmsg << std::endl;
        }

        void reportErr(const ErrorMessage &msg) override
        {
            reportOut(msg.toXML());
        }

        void reportMetric(const std::string &metric) override
        {
            /* Not used here */
            (void) metric;
        }

        void reportProgress(const std::string & /*filename*/, const char /*stage*/[], const std::size_t /*value*/) override
        {}
    };
}

CmdLineParser::CmdLineParser(CmdLineLogger &logger, Settings &settings, Suppressions &suppressions)
    : mLogger(logger)
    , mSettings(settings)
    , mSuppressions(suppressions)
{}

bool CmdLineParser::fillSettingsFromArgs(int argc, const char* const argv[])
{
    const Result result = parseFromArgs(argc, argv);

    switch (result) {
    case Result::Success:
        break;
    case Result::Exit:
        Settings::terminate();
        return true;
    case Result::Fail:
        return false;
    }

    // Libraries must be loaded before FileLister is executed to ensure markup files will be
    // listed properly.
    if (!loadLibraries(mSettings))
        return false;

    if (!loadAddons(mSettings))
        return false;

    // Check that all include paths exist
    {
        for (auto iter = mSettings.includePaths.cbegin();
             iter != mSettings.includePaths.cend();
             ) {
            const std::string path(Path::toNativeSeparators(*iter));
            if (Path::isDirectory(path))
                ++iter;
            else {
                // TODO: this bypasses the template format and other settings
                // If the include path is not found, warn user and remove the non-existing path from the list.
                if (mSettings.severity.isEnabled(Severity::information))
                    std::cout << "(information) Couldn't find path given by -I '" << path << '\'' << std::endl;
                iter = mSettings.includePaths.erase(iter);
            }
        }
    }

    // Output a warning for the user if he tries to exclude headers
    const std::vector<std::string>& ignored = mIgnoredPaths;
    const bool warn = std::any_of(ignored.cbegin(), ignored.cend(), [](const std::string& i) {
        return Path::isHeader(i);
    });
    if (warn) {
        mLogger.printMessage("filename exclusion does not apply to header (.h and .hpp) files.");
        mLogger.printMessage("Please use --suppress for ignoring results from the header files.");
    }

    const std::vector<std::string>& pathnamesRef = mPathNames;
    const std::list<FileSettings>& fileSettingsRef = mFileSettings;

    // the inputs can only be used exclusively - CmdLineParser should already handle this
    assert(!(!pathnamesRef.empty() && !fileSettingsRef.empty()));

    if (!fileSettingsRef.empty()) {
        std::list<FileSettings> fileSettings;
        if (!mSettings.fileFilters.empty()) {
            // filter only for the selected filenames from all project files
            PathMatch filtermatcher(mSettings.fileFilters, Path::getCurrentPath());
            std::copy_if(fileSettingsRef.cbegin(), fileSettingsRef.cend(), std::back_inserter(fileSettings), [&](const FileSettings &fs) {
                return filtermatcher.match(fs.filename());
            });
            if (fileSettings.empty()) {
                for (const std::string& f: mSettings.fileFilters)
                    mLogger.printError("could not find any files matching the filter:" + f);
                return false;
            }
        }
        else {
            fileSettings = fileSettingsRef;
        }

        // TODO: de-duplicate

        mFileSettings.clear();

        frontend::applyLang(fileSettings, mSettings, mEnforcedLang);

        // sort the markup last
        std::copy_if(fileSettings.cbegin(), fileSettings.cend(), std::back_inserter(mFileSettings), [&](const FileSettings &fs) {
            return !mSettings.library.markupFile(fs.filename()) || !mSettings.library.processMarkupAfterCode(fs.filename());
        });

        std::copy_if(fileSettings.cbegin(), fileSettings.cend(), std::back_inserter(mFileSettings), [&](const FileSettings &fs) {
            return mSettings.library.markupFile(fs.filename()) && mSettings.library.processMarkupAfterCode(fs.filename());
        });

        if (mFileSettings.empty()) {
            mLogger.printError("could not find or open any of the paths given.");
            return false;
        }
    }

    if (!pathnamesRef.empty()) {
        std::list<FileWithDetails> filesResolved;
        // Execute recursiveAddFiles() to each given file parameter
        // TODO: verbose log which files were ignored?
        const PathMatch matcher(ignored, Path::getCurrentPath());
        for (const std::string &pathname : pathnamesRef) {
            const std::string err = FileLister::recursiveAddFiles(filesResolved, Path::toNativeSeparators(pathname), mSettings.library.markupExtensions(), matcher, mSettings.debugignore);
            if (!err.empty()) {
                // TODO: bail out?
                mLogger.printMessage(err);
            }
        }

        if (filesResolved.empty()) {
            mLogger.printError("could not find or open any of the paths given.");
            // TODO: PathMatch should provide the information if files were ignored
            if (!ignored.empty())
                mLogger.printMessage("Maybe all paths were ignored?");
            return false;
        }

        std::list<FileWithDetails> files;
        if (!mSettings.fileFilters.empty()) {
            files = filterFiles(mSettings.fileFilters, filesResolved);
            if (files.empty()) {
                for (const std::string& f: mSettings.fileFilters)
                    mLogger.printError("could not find any files matching the filter:" + f);
                return false;
            }
        }
        else {
            files = std::move(filesResolved);
        }

        // de-duplicate files
        {
            auto it = files.begin();
            while (it != files.end()) {
                const std::string& absname = it->abspath();
                // TODO: log if duplicated files were dropped
                files.erase(std::remove_if(std::next(it), files.end(), [&](const FileWithDetails& entry) {
                    return entry.abspath() == absname;
                }), files.end());
                ++it;
            }
        }

        frontend::applyLang(files, mSettings, mEnforcedLang);

        // sort the markup last
        std::copy_if(files.cbegin(), files.cend(), std::inserter(mFiles, mFiles.end()), [&](const FileWithDetails& entry) {
            return !mSettings.library.markupFile(entry.path()) || !mSettings.library.processMarkupAfterCode(entry.path());
        });

        std::copy_if(files.cbegin(), files.cend(), std::inserter(mFiles, mFiles.end()), [&](const FileWithDetails& entry) {
            return mSettings.library.markupFile(entry.path()) && mSettings.library.processMarkupAfterCode(entry.path());
        });

        if (mFiles.empty()) {
            mLogger.printError("could not find or open any of the paths given.");
            return false;
        }
    }

    return true;
}

// TODO: normalize/simplify/native all path parameters
// TODO: error out on all missing given files/paths
CmdLineParser::Result CmdLineParser::parseFromArgs(int argc, const char* const argv[])
{
    mSettings.exename = Path::getCurrentExecutablePath(argv[0]);

    bool xmlOptionProvided = false;
    bool outputFormatOptionProvided = false;

    // default to --check-level=normal from CLI for now
    mSettings.setCheckLevel(Settings::CheckLevel::normal);

    // read --debug-lookup early so the option is available for the cppcheck.cfg loading
    for (int i = 1; i < argc; i++) {
        // Show debug warnings for lookup for configuration files
        if (std::strcmp(argv[i], "--debug-lookup") == 0)
            mSettings.debuglookup = true;

        else if (std::strncmp(argv[i], "--debug-lookup=", 15) == 0) {
            const std::string lookup = argv[i] + 15;
            if (lookup == "all")
                mSettings.debuglookup = true;
            else if (lookup == "addon")
                mSettings.debuglookupAddon = true;
            else if (lookup == "config")
                mSettings.debuglookupConfig = true;
            else if (lookup == "library")
                mSettings.debuglookupLibrary = true;
            else if (lookup == "platform")
                mSettings.debuglookupPlatform = true;
            else
            {
                mLogger.printError("unknown lookup '" + lookup + "'");
                return Result::Fail;
            }
        }
    }

    if (!loadCppcheckCfg())
        return Result::Fail;

    if (argc <= 1) {
        printHelp();
        return Result::Exit;
    }

    // check for exclusive options
    for (int i = 1; i < argc; i++) {
        // documentation..
        if (std::strcmp(argv[i], "--doc") == 0) {
            std::ostringstream doc;
            // Get documentation..
            for (const Check * it : Check::instances()) {
                const std::string& name(it->name());
                const std::string info(it->classInfo());
                if (!name.empty() && !info.empty())
                    doc << "## " << name << " ##\n"
                        << info << "\n";
            }

            mLogger.printRaw(doc.str());
            return Result::Exit;
        }

        // print all possible error messages..
        if (std::strcmp(argv[i], "--errorlist") == 0) {
            {
                XMLErrorMessagesLogger xmlLogger;
                std::cout << ErrorMessage::getXMLHeader(mSettings.cppcheckCfgProductName, 2);
                CppCheck::getErrorMessages(xmlLogger);
                std::cout << ErrorMessage::getXMLFooter(2) << std::endl;
            }
            return Result::Exit;
        }

        // Print help
        if (std::strcmp(argv[i], "-h") == 0 || std::strcmp(argv[i], "--help") == 0) {
            printHelp();
            return Result::Exit;
        }

        if (std::strcmp(argv[i], "--filesdir") == 0) {
#ifdef FILESDIR
            mLogger.printRaw(FILESDIR); // TODO: should not include newline
#endif
            return Result::Exit;
        }

        if (std::strcmp(argv[i], "--version") == 0) {
            const std::string version = getVersion();
            mLogger.printRaw(version); // TODO: should not include newline
            return Result::Exit;
        }
    }

    bool debug = false;
    bool inputAsFilter = false; // set by: --file-filter=+

    ImportProject::Type projectType = ImportProject::Type::NONE;
    ImportProject project;
    std::string vsConfig;

    std::string platform;
    char defaultSign = '\0';

    std::vector<std::string> lookupPaths{
        Path::getCurrentPath(), // TODO: do we want to look in CWD?
        Path::getPathFromFilename(mSettings.exename),
    };

    bool executorAuto = true;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            mPathNames.emplace_back(Path::fromNativeSeparators(Path::removeQuotationMarks(argv[i])));
        }

        // User define
        else if (std::strncmp(argv[i], "-D", 2) == 0) {
            std::string define;

            // "-D define"
            if (std::strcmp(argv[i], "-D") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    mLogger.printError("argument to '-D' is missing.");
                    return Result::Fail;
                }

                define = argv[i];
            }
            // "-Ddefine"
            else {
                define = 2 + argv[i];
            }

            // No "=", append a "=1"
            if (define.find('=') == std::string::npos)
                define += "=1";

            if (!mSettings.userDefines.empty())
                mSettings.userDefines += ";";
            mSettings.userDefines += define;
        }

        // -E
        else if (std::strcmp(argv[i], "-E") == 0) {
            mSettings.preprocessOnly = true;
            mSettings.quiet = true;
        }

        // Include paths
        else if (std::strncmp(argv[i], "-I", 2) == 0) {
            std::string path;

            // "-I path/"
            if (std::strcmp(argv[i], "-I") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    mLogger.printError("argument to '-I' is missing.");
                    return Result::Fail;
                }
                path = argv[i];
            }

            // "-Ipath/"
            else {
                path = 2 + argv[i];
            }
            path = Path::removeQuotationMarks(std::move(path));
            path = Path::fromNativeSeparators(std::move(path));

            // If path doesn't end with / or \, add it
            if (!endsWith(path,'/'))
                path += '/';

            mSettings.includePaths.emplace_back(std::move(path));
        }

        // User undef
        else if (std::strncmp(argv[i], "-U", 2) == 0) {
            std::string undef;

            // "-U undef"
            if (std::strcmp(argv[i], "-U") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    mLogger.printError("argument to '-U' is missing.");
                    return Result::Fail;
                }

                undef = argv[i];
            }
            // "-Uundef"
            else {
                undef = 2 + argv[i];
            }

            mSettings.userUndefs.insert(std::move(undef));
        }

        else if (std::strncmp(argv[i], "--addon=", 8) == 0)
            mSettings.addons.emplace(argv[i]+8);

        else if (std::strncmp(argv[i],"--addon-python=", 15) == 0)
            mSettings.addonPython.assign(argv[i]+15);

        else if (std::strcmp(argv[i],"--analyze-all-vs-configs") == 0) {
            mSettings.analyzeAllVsConfigs = true;
            mAnalyzeAllVsConfigsSetOnCmdLine = true;
        }

        // Check configuration
        else if (std::strcmp(argv[i], "--check-config") == 0)
            mSettings.checkConfiguration = true;

        else if (std::strcmp(argv[i], "--check-headers") == 0)
            mSettings.checkHeaders = true;

        // Check level
        else if (std::strncmp(argv[i], "--check-level=", 14) == 0) {
            Settings::CheckLevel level = Settings::CheckLevel::normal;
            const std::string level_s(argv[i] + 14);
            if (level_s == "reduced")
                level = Settings::CheckLevel::reduced;
            else if (level_s == "normal")
                level = Settings::CheckLevel::normal;
            else if (level_s == "exhaustive")
                level = Settings::CheckLevel::exhaustive;
            else {
                mLogger.printError("unknown '--check-level' value '" + level_s + "'.");
                return Result::Fail;
            }

            mSettings.setCheckLevel(level);
        }

        // Check library definitions
        else if (std::strcmp(argv[i], "--check-library") == 0) {
            mSettings.checkLibrary = true;
        }

        else if (std::strcmp(argv[i], "--check-unused-templates") == 0)
            mSettings.checkUnusedTemplates = true;

        else if (std::strncmp(argv[i], "--check-version=", 16) == 0) {
            if (!loadCppcheckCfg())
                return Result::Fail;
            const std::string actualVersion = getVersion();
            const std::string wantedVersion = argv[i] + 16;
            if (actualVersion != wantedVersion) {
                mLogger.printError("--check-version check failed. Aborting.");
                return Result::Fail;
            }
        }

        else if (std::strncmp(argv[i], "--checkers-report=", 18) == 0)
            mSettings.checkersReportFilename = argv[i] + 18;

        else if (std::strncmp(argv[i], "--checks-max-time=", 18) == 0) {
            if (!parseNumberArg(argv[i], 18, mSettings.checksMaxTime, true))
                return Result::Fail;
        }

        else if (std::strcmp(argv[i], "--clang") == 0) {
            mSettings.clang = true;
        }

        else if (std::strncmp(argv[i], "--clang=", 8) == 0) {
            mSettings.clang = true;
            mSettings.clangExecutable = argv[i] + 8;
        }

        else if (std::strcmp(argv[i], "--clang-tidy") == 0) {
            mSettings.clangTidy = true;
        }

        else if (std::strncmp(argv[i], "--clang-tidy=", 13) == 0) {
            mSettings.clangTidy = true;
            mSettings.clangTidyExecutable = argv[i] + 13;
        }

        else if (std::strncmp(argv[i], "--config-exclude=",17) ==0) {
            mSettings.configExcludePaths.insert(Path::fromNativeSeparators(argv[i] + 17));
        }

        else if (std::strncmp(argv[i], "--config-excludes-file=", 23) == 0) {
            // open this file and read every input file (1 file name per line)
            const std::string cfgExcludesFile(23 + argv[i]);
            if (!addPathsToSet(cfgExcludesFile, mSettings.configExcludePaths)) {
                mLogger.printError("unable to open config excludes file at '" + cfgExcludesFile + "'");
                return Result::Fail;
            }
        }

        else if (std::strncmp(argv[i], "--cppcheck-build-dir=", 21) == 0) {
            std::string path = Path::fromNativeSeparators(argv[i] + 21);
            if (path.empty()) {
                mLogger.printError("no path has been specified for --cppcheck-build-dir");
                return Result::Fail;
            }
            if (endsWith(path, '/'))
                path.pop_back();
            mSettings.buildDir = std::move(path);
        }

        else if (std::strcmp(argv[i], "--cpp-header-probe") == 0) {
            mSettings.cppHeaderProbe = true;
        }

        else if (std::strcmp(argv[i], "--debug-analyzerinfo") == 0)
            mSettings.debugainfo = true;

        else if (std::strcmp(argv[i], "--debug-ast") == 0)
            mSettings.debugast = true;

        // Show debug warnings for lookup for configuration files
        else if (std::strcmp(argv[i], "--debug-clang-output") == 0)
            mSettings.debugClangOutput = true;

        // Show debug messages for ignored files
        else if (std::strcmp(argv[i], "--debug-ignore") == 0)
            mSettings.debugignore = true;

        // Show --debug output after the first simplifications
        else if (std::strcmp(argv[i], "--debug") == 0 ||
                 std::strcmp(argv[i], "--debug-normal") == 0)
            debug = true;

        else if (std::strcmp(argv[i], "--debug-lookup") == 0)
            continue; // already handled above

        else if (std::strncmp(argv[i], "--debug-lookup=", 15) == 0)
            continue; // already handled above

        // Flag used for various purposes during debugging
        else if (std::strcmp(argv[i], "--debug-simplified") == 0)
            mSettings.debugSimplified = true;

        else if (std::strcmp(argv[i], "--debug-symdb") == 0)
            mSettings.debugsymdb = true;

        // Show template information
        else if (std::strcmp(argv[i], "--debug-template") == 0)
            mSettings.debugtemplate = true;

        else if (std::strcmp(argv[i], "--debug-valueflow") == 0)
            mSettings.debugvalueflow = true;

        // Show debug warnings
        else if (std::strcmp(argv[i], "--debug-warnings") == 0)
            mSettings.debugwarnings = true;

        else if (std::strncmp(argv[i], "--disable=", 10) == 0) {
            const std::string errmsg = mSettings.removeEnabled(argv[i] + 10);
            if (!errmsg.empty()) {
                mLogger.printError(errmsg);
                return Result::Fail;
            }
        }

        // dump cppcheck data
        else if (std::strcmp(argv[i], "--dump") == 0)
            mSettings.dump = true;

        else if (std::strcmp(argv[i], "--emit-duplicates") == 0)
            mSettings.emitDuplicates = true;

        else if (std::strncmp(argv[i], "--enable=", 9) == 0) {
            const std::string enable_arg = argv[i] + 9;
            const std::string errmsg = mSettings.addEnabled(enable_arg);
            if (!errmsg.empty()) {
                mLogger.printError(errmsg);
                return Result::Fail;
            }
            // when "style" is enabled, also enable "warning", "performance" and "portability"
            if (enable_arg.find("style") != std::string::npos) {
                mSettings.addEnabled("warning");
                mSettings.addEnabled("performance");
                mSettings.addEnabled("portability");
            }
        }

        // --error-exitcode=1
        else if (std::strncmp(argv[i], "--error-exitcode=", 17) == 0) {
            if (!parseNumberArg(argv[i], 17, mSettings.exitCode))
                return Result::Fail;
        }

        // Exception handling inside cppcheck client
        else if (std::strcmp(argv[i], "--exception-handling") == 0) {
#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
            mSettings.exceptionHandling = true;
#else
            mLogger.printError("Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.");
            return Result::Fail;
#endif
        }

        // Exception handling inside cppcheck client
        else if (std::strncmp(argv[i], "--exception-handling=", 21) == 0) {
#if defined(USE_WINDOWS_SEH) || defined(USE_UNIX_SIGNAL_HANDLING)
            const std::string exceptionOutfilename = argv[i] + 21;
            if (exceptionOutfilename != "stderr" && exceptionOutfilename != "stdout") {
                mLogger.printError("invalid '--exception-handling' argument");
                return Result::Fail;
            }
            mSettings.exceptionHandling = true;
            mSettings.exceptionOutput = (exceptionOutfilename == "stderr") ? stderr : stdout;
#else
            mLogger.printError("Option --exception-handling is not supported since Cppcheck has not been built with any exception handling enabled.");
            return Result::Fail;
#endif
        }

        else if (std::strncmp(argv[i], "--executor=", 11) == 0) {
            const std::string type = 11 + argv[i];
            if (type == "auto") {
                executorAuto = true;
                mSettings.executor = Settings::defaultExecutor();
            }
            else if (type == "thread") {
#if defined(HAS_THREADING_MODEL_THREAD)
                executorAuto = false;
                mSettings.executor = Settings::ExecutorType::Thread;
#else
                mLogger.printError("executor type 'thread' cannot be used as Cppcheck has not been built with a respective threading model.");
                return Result::Fail;
#endif
            }
            else if (type == "process") {
#if defined(HAS_THREADING_MODEL_FORK)
                executorAuto = false;
                mSettings.executor = Settings::ExecutorType::Process;
#else
                mLogger.printError("executor type 'process' cannot be used as Cppcheck has not been built with a respective threading model.");
                return Result::Fail;
#endif
            }
            else {
                mLogger.printError("unknown executor: '" + type + "'.");
                return Result::Fail;
            }
        }

        else if (std::strncmp(argv[i], "--exitcode-suppress=", 20) == 0) {
            const std::string suppression = argv[i]+20;
            const std::string errmsg(mSuppressions.nofail.addSuppressionLine(suppression));
            if (!errmsg.empty()) {
                mLogger.printError(errmsg);
                return Result::Fail;
            }
        }

        // Filter errors
        else if (std::strncmp(argv[i], "--exitcode-suppressions=", 24) == 0) {
            // exitcode-suppressions=filename.txt
            std::string filename = 24 + argv[i];

            std::ifstream f(filename);
            if (!f.is_open()) {
                mLogger.printError("couldn't open the file: \"" + filename + "\".");
                return Result::Fail;
            }
            const std::string errmsg(mSuppressions.nofail.parseFile(f));
            if (!errmsg.empty()) {
                mLogger.printError(errmsg);
                return Result::Fail;
            }
        }

        // use a file filter
        else if (std::strncmp(argv[i], "--file-filter=", 14) == 0) {
            const char *filter = argv[i] + 14;
            if (std::strcmp(filter, "-") == 0) {
                if (!addFilesToList(filter, mSettings.fileFilters)) {
                    mLogger.printError("Failed: --file-filter=-");
                    return Result::Fail;
                }
            } else if (std::strcmp(filter, "+") == 0) {
                inputAsFilter = true;
            } else {
                mSettings.fileFilters.emplace_back(filter);
            }
        }

        // file list specified
        else if (std::strncmp(argv[i], "--file-list=", 12) == 0) {
            // open this file and read every input file (1 file name per line)
            const std::string fileList = argv[i] + 12;
            if (!addFilesToList(fileList, mPathNames)) {
                mLogger.printError("couldn't open the file: \"" + fileList + "\".");
                return Result::Fail;
            }
        }

        // Force checking of files that have "too many" configurations
        else if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--force") == 0) {
            mSettings.force = true;
            mSettings.maxConfigsOption = Settings::maxConfigsNotAssigned;
        }

        else if (std::strcmp(argv[i], "--fsigned-char") == 0)
            defaultSign = 's';

        else if (std::strcmp(argv[i], "--funsigned-char") == 0)
            defaultSign = 'u';

        // Ignored paths
        else if (std::strncmp(argv[i], "-i", 2) == 0) {
            std::string path;

            // "-i path/"
            if (std::strcmp(argv[i], "-i") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    mLogger.printError("argument to '-i' is missing.");
                    return Result::Fail;
                }
                path = argv[i];
            }

            // "-ipath/"
            else {
                path = 2 + argv[i];
            }

            if (!path.empty()) {
                mIgnoredPaths.emplace_back(std::move(path));
            }
        }

        else if (std::strncmp(argv[i], "--include=", 10) == 0) {
            mSettings.userIncludes.emplace_back(Path::fromNativeSeparators(argv[i] + 10));
        }

        else if (std::strncmp(argv[i], "--includes-file=", 16) == 0) {
            // open this file and read every input file (1 file name per line)
            const std::string includesFile(16 + argv[i]);
            if (!addIncludePathsToList(includesFile, mSettings.includePaths)) {
                mLogger.printError("unable to open includes file at '" + includesFile + "'");
                return Result::Fail;
            }
        }

        // Inconclusive checking
        else if (std::strcmp(argv[i], "--inconclusive") == 0)
            mSettings.certainty.enable(Certainty::inconclusive);

        // Enables inline suppressions.
        else if (std::strcmp(argv[i], "--inline-suppr") == 0)
            mSettings.inlineSuppressions = true;

        // Checking threads
        else if (std::strncmp(argv[i], "-j", 2) == 0) {
            std::string numberString;

            // "-j 3"
            if (std::strcmp(argv[i], "-j") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    mLogger.printError("argument to '-j' is missing.");
                    return Result::Fail;
                }

                numberString = argv[i];
            }

            // "-j3"
            else
                numberString = argv[i]+2;

            unsigned int tmp;
            std::string err;
            if (!strToInt(numberString, tmp, &err)) {
                mLogger.printError("argument to '-j' is not valid - " +  err + ".");
                return Result::Fail;
            }
            if (tmp == 0) {
                // TODO: implement get CPU logical core count and use that.
                // Usually, -j 0 would mean "use all available cores," but
                // if we get a 0, we just stall and don't do any work.
                mLogger.printError("argument for '-j' must be greater than 0.");
                return Result::Fail;
            }
            if (tmp > 1024) {
                // Almost nobody has 1024 logical cores, but somebody out
                // there does.
                mLogger.printError("argument for '-j' is allowed to be 1024 at max.");
                return Result::Fail;
            }
            mSettings.jobs = tmp;
        }

        else if (std::strncmp(argv[i], "-l", 2) == 0) {
#ifdef HAS_THREADING_MODEL_FORK
            std::string numberString;

            // "-l 3"
            if (std::strcmp(argv[i], "-l") == 0) {
                ++i;
                if (i >= argc || argv[i][0] == '-') {
                    mLogger.printError("argument to '-l' is missing.");
                    return Result::Fail;
                }

                numberString = argv[i];
            }

            // "-l3"
            else
                numberString = argv[i]+2;

            int tmp;
            std::string err;
            if (!strToInt(numberString, tmp, &err)) {
                mLogger.printError("argument to '-l' is not valid - " + err + ".");
                return Result::Fail;
            }
            mSettings.loadAverage = tmp;
#else
            mLogger.printError("Option -l cannot be used as Cppcheck has not been built with fork threading model.");
            return Result::Fail;
#endif
        }

        // Enforce language (--language=, -x)
        else if (std::strncmp(argv[i], "--language=", 11) == 0 || std::strcmp(argv[i], "-x") == 0) {
            std::string str;
            if (argv[i][2]) {
                str = argv[i]+11;
            } else {
                i++;
                if (i >= argc || argv[i][0] == '-') {
                    mLogger.printError("no language given to '-x' option.");
                    return Result::Fail;
                }
                str = argv[i];
            }

            if (str == "c")
                mEnforcedLang = Standards::Language::C;
            else if (str == "c++")
                mEnforcedLang = Standards::Language::CPP;
            else {
                mLogger.printError("unknown language '" + str + "' enforced.");
                return Result::Fail;
            }
        }

        // --library
        else if (std::strncmp(argv[i], "--library=", 10) == 0) {
            std::vector<std::string> libs = splitString(argv[i] + 10, ',');
            for (auto& l : libs) {
                if (l.empty()) {
                    mLogger.printError("empty library specified.");
                    return Result::Fail;
                }
                mSettings.libraries.emplace_back(std::move(l));
            }
        }

        // Set maximum number of #ifdef configurations to check
        else if (std::strncmp(argv[i], "--max-configs=", 14) == 0) {
            int tmp;
            if (!parseNumberArg(argv[i], 14, tmp))
                return Result::Fail;
            if (tmp < 1) {
                mLogger.printError("argument to '--max-configs=' must be greater than 0.");
                return Result::Fail;
            }

            mSettings.maxConfigsOption = tmp;
            mSettings.force = false;
        }

        // max ctu depth
        else if (std::strncmp(argv[i], "--max-ctu-depth=", 16) == 0) {
            int temp = 0;
            if (!parseNumberArg(argv[i], 16, temp))
                return Result::Fail;
            if (temp > 10) {
                mLogger.printMessage("--max-ctu-depth is being capped at 10. This limitation will be removed in a future Cppcheck version.");
                temp = 10;
            }
            mSettings.maxCtuDepth = temp;
        }

        else if (std::strncmp(argv[i], "--max-template-recursion=", 25) == 0) {
            if (!parseNumberArg(argv[i], 25, mSettings.maxTemplateRecursion))
                return Result::Fail;
        }

        else if (std::strcmp(argv[i],"--no-analyze-all-vs-configs") == 0) {
            mSettings.analyzeAllVsConfigs = false;
            mAnalyzeAllVsConfigsSetOnCmdLine = true;
        }

        else if (std::strcmp(argv[i], "--no-check-headers") == 0)
            mSettings.checkHeaders = false;

        else if (std::strcmp(argv[i], "--no-check-unused-templates") == 0)
            mSettings.checkUnusedTemplates = false;

        // undocumented option for usage in Python tests to indicate that no build dir should be injected
        else if (std::strcmp(argv[i], "--no-cppcheck-build-dir") == 0) {
            mSettings.buildDir.clear();
        }

        else if (std::strcmp(argv[i], "--no-cpp-header-probe") == 0) {
            mSettings.cppHeaderProbe = false;
        }

        else if (std::strcmp(argv[i], "--no-safety") == 0)
            mSettings.safety = false;

        // Write results in file
        else if (std::strncmp(argv[i], "--output-file=", 14) == 0)
            mSettings.outputFile = Path::simplifyPath(argv[i] + 14);

        else if (std::strncmp(argv[i], "--output-format=", 16) == 0) {
            if (xmlOptionProvided) {
                outputFormatOptionMixingError();
                return Result::Fail;
            }
            const std::string format = argv[i] + 16;
            // plist can not be handled here because it requires additional data
            if (format == "text")
                mSettings.outputFormat = Settings::OutputFormat::text;
            else if (format == "sarif")
                mSettings.outputFormat = Settings::OutputFormat::sarif;
            else if (format == "xml")
                mSettings.outputFormat = Settings::OutputFormat::xml;
            else if (format == "xmlv2") {
                mSettings.outputFormat = Settings::OutputFormat::xml;
                mSettings.xml_version = 2;
            } else if (format == "xmlv3") {
                mSettings.outputFormat = Settings::OutputFormat::xml;
                mSettings.xml_version = 3;
            } else {
                mLogger.printError("argument to '--output-format=' must be 'text', 'sarif', 'xml' (deprecated), 'xmlv2' or 'xmlv3'.");
                return Result::Fail;
            }
            mSettings.plistOutput = "";
            outputFormatOptionProvided = true;
        }


        // Experimental: limit execution time for extended valueflow analysis. basic valueflow analysis
        // is always executed.
        else if (std::strncmp(argv[i], "--performance-valueflow-max-time=", 33) == 0) {
            if (!parseNumberArg(argv[i], 33, mSettings.vfOptions.maxTime, true))
                return Result::Fail;
        }

        else if (std::strncmp(argv[i], "--performance-valueflow-max-if-count=", 37) == 0) {
            if (!parseNumberArg(argv[i], 37, mSettings.vfOptions.maxIfCount, true))
                return Result::Fail;
        }

        else if (std::strncmp(argv[i], "--performance-valueflow-max-iterations=", 39) == 0) {
            if (!parseNumberArg(argv[i], 39, mSettings.vfOptions.maxIterations, true))
                return Result::Fail;
        }

        // Specify platform
        else if (std::strncmp(argv[i], "--platform=", 11) == 0) {
            std::string p = 11 + argv[i];
            if (p.empty()) {
                mLogger.printError("empty platform specified.");
                return Result::Fail;
            }
            platform = std::move(p);
        }

        // Write results in results.plist
        else if (std::strncmp(argv[i], "--plist-output=", 15) == 0) {
            std::string path = Path::simplifyPath(argv[i] + 15);
            if (path.empty())
                path = ".";

            const std::string plistOutput = Path::toNativeSeparators(path);
            if (!Path::isDirectory(plistOutput)) {
                std::string message("plist folder does not exist: '");
                message += plistOutput;
                message += "'.";
                mLogger.printError(message);
                return Result::Fail;
            }

            if (!endsWith(path,'/'))
                path += '/';

            mSettings.outputFormat = Settings::OutputFormat::plist;
            mSettings.plistOutput = std::move(path);
        }

        // Special Cppcheck Premium options
        else if ((std::strncmp(argv[i], "--premium=", 10) == 0 || std::strncmp(argv[i], "--premium-", 10) == 0) && mSettings.premium) {
            // valid options --premium=..
            const std::set<std::string> valid{
                "autosar",
                "cert-c-2016",
                "cert-c++-2016",
                "cert-cpp-2016",
                "cert-c",
                "cert-c++",
                "metrics",
                "misra-c-2012",
                "misra-c-2023",
                "misra-c-2025",
                "misra-c++-2008",
                "misra-cpp-2008",
                "misra-c++-2023",
                "misra-cpp-2023",
                "bughunting",
                "safety",  // TODO: deprecate in favor of the regular --safety/--no-safety
                "safety-profiles",
                "debug-progress"};
            // valid options --premium-..=
            const std::set<std::string> valid2{
                "cert-c-int-precision",
                "license-file"
            };

            if (std::strcmp(argv[i], "--premium=safety-off") == 0) {
                mSettings.safety = false;
                continue;
            }
            if (std::strcmp(argv[i], "--premium=safety") == 0)
                mSettings.safety = true;
            if (!mSettings.premiumArgs.empty())
                mSettings.premiumArgs += " ";
            const std::string p(argv[i] + 10);
            const std::string p2(p.find('=') != std::string::npos ? p.substr(0, p.find('=')) : "");
            const bool isCodingStandard = startsWith(p, "autosar") || startsWith(p,"cert-") || startsWith(p,"misra-") || p == "safety-profiles";
            const std::string p3(endsWith(p,":all") && isCodingStandard ? p.substr(0,p.rfind(':')) : p);
            if (!valid.count(p3) && !valid2.count(p2)) {
                mLogger.printError("invalid --premium option '" + (p2.empty() ? p : p2) + "'.");
                return Result::Fail;
            }
            mSettings.premiumArgs += "--" + p;
            if (isCodingStandard) {
                // All checkers related to the coding standard should be enabled. The coding standards
                // do not all undefined behavior or portability issues.
                mSettings.addEnabled("warning");
                mSettings.addEnabled("portability");
            }
        }

        // --project
        else if (std::strncmp(argv[i], "--project=", 10) == 0) {
            if (projectType != ImportProject::Type::NONE)
            {
                mLogger.printError("multiple --project options are not supported.");
                return Result::Fail;
            }

            std::string projectFile = argv[i]+10;
            projectType = project.import(projectFile, &mSettings, &mSuppressions);
            if (projectType == ImportProject::Type::CPPCHECK_GUI) {
                for (const std::string &lib : project.guiProject.libraries)
                    mSettings.libraries.emplace_back(lib);

                const auto& excludedPaths = project.guiProject.excludedPaths;
                std::copy(excludedPaths.cbegin(), excludedPaths.cend(), std::back_inserter(mIgnoredPaths));

                if (!project.guiProject.platform.empty())
                    platform = project.guiProject.platform;

                // look for external files relative to project first
                lookupPaths.insert(lookupPaths.cbegin(), Path::getPathFromFilename(projectFile));

                const auto& projectFileGui = project.guiProject.projectFile;
                if (!projectFileGui.empty()) {
                    // read underlying project
                    projectFile = projectFileGui;
                    projectType = project.import(projectFileGui, &mSettings, &mSuppressions);
                    if (projectType == ImportProject::Type::CPPCHECK_GUI) {
                        mLogger.printError("nested Cppcheck GUI projects are not supported.");
                        return Result::Fail;
                    }
                }
            }
            if (projectType == ImportProject::Type::COMPILE_DB)
                mSettings.maxConfigsProject = 1;
            if (projectType == ImportProject::Type::VS_SLN ||
                projectType == ImportProject::Type::VS_SLNX ||
                projectType == ImportProject::Type::VS_VCXPROJ) {
                mSettings.libraries.emplace_back("windows");
            }
            for (const auto &error : project.errors)
                mLogger.printError(error);
            if (projectType == ImportProject::Type::MISSING) {
                mLogger.printError("failed to open project '" + projectFile + "'. The file does not exist.");
                return Result::Fail;
            }
            if (projectType == ImportProject::Type::UNKNOWN) {
                mLogger.printError("failed to load project '" + projectFile + "'. The format is unknown.");
                return Result::Fail;
            }
            if (projectType == ImportProject::Type::FAILURE) {
                mLogger.printError("failed to load project '" + projectFile + "'. An error occurred.");
                return Result::Fail;
            }
        }

        // --project-configuration
        else if (std::strncmp(argv[i], "--project-configuration=", 24) == 0) {
            vsConfig = argv[i] + 24;
            if (vsConfig.empty()) {
                mLogger.printError("--project-configuration parameter is empty.");
                return Result::Fail;
            }
            if (projectType != ImportProject::Type::VS_SLN &&
                projectType != ImportProject::Type::VS_SLNX &&
                projectType != ImportProject::Type::VS_VCXPROJ) {
                mLogger.printError("--project-configuration has no effect - no Visual Studio project provided.");
                return Result::Fail;
            }
        }

        // Only print something when there are errors
        else if (std::strcmp(argv[i], "-q") == 0 || std::strcmp(argv[i], "--quiet") == 0)
            mSettings.quiet = true;

        // Output relative paths
        else if (std::strcmp(argv[i], "-rp") == 0 || std::strcmp(argv[i], "--relative-paths") == 0)
            mSettings.relativePaths = true;
        else if (std::strncmp(argv[i], "-rp=", 4) == 0 || std::strncmp(argv[i], "--relative-paths=", 17) == 0) {
            mSettings.relativePaths = true;
            if (argv[i][argv[i][3]=='='?4:17] != 0) {
                std::string paths = argv[i]+(argv[i][3]=='='?4:17);
                for (;;) {
                    const std::string::size_type pos = paths.find(';');
                    if (pos == std::string::npos) {
                        mSettings.basePaths.emplace_back(Path::fromNativeSeparators(std::move(paths)));
                        break;
                    }
                    mSettings.basePaths.emplace_back(Path::fromNativeSeparators(paths.substr(0, pos)));
                    paths.erase(0, pos + 1);
                }
            } else {
                mLogger.printError("no paths specified for the '" + std::string(argv[i]) + "' option.");
                return Result::Fail;
            }
        }

        // Report progress
        else if (std::strcmp(argv[i], "--report-progress") == 0) {
            mSettings.reportProgress = 10;
        }

        else if (std::strncmp(argv[i], "--report-progress=", 18) == 0) {
            if (!parseNumberArg(argv[i], 18, mSettings.reportProgress, true))
                return Result::Fail;
        }

        else if (std::strncmp(argv[i], "--report-type=", 14) == 0) {
            const std::string typeStr = argv[i] + 14;
            if (typeStr == "normal") {
                mSettings.reportType = ReportType::normal;
            } else if (typeStr == "autosar") {
                mSettings.reportType = ReportType::autosar;
            } else if (typeStr == "cert-c-2016") {
                mSettings.reportType = ReportType::certC;
            } else if (typeStr == "cert-cpp-2016") {
                mSettings.reportType = ReportType::certCpp;
            } else if (typeStr == "misra-c-2012") {
                mSettings.reportType = ReportType::misraC2012;
            } else if (typeStr == "misra-c-2023") {
                mSettings.reportType = ReportType::misraC2023;
            } else if (typeStr == "misra-c-2025") {
                mSettings.reportType = ReportType::misraC2025;
            } else if (typeStr == "misra-cpp-2008") {
                mSettings.reportType = ReportType::misraCpp2008;
            } else if (typeStr == "misra-cpp-2023") {
                mSettings.reportType = ReportType::misraCpp2023;
            } else {
                mLogger.printError("Unknown report type \'" + typeStr + "\'");
                return Result::Fail;
            }
        }

        // Rule given at command line
        else if (std::strncmp(argv[i], "--rule=", 7) == 0) {
#ifdef HAVE_RULES
            Settings::Rule rule;
            rule.pattern = 7 + argv[i];

            if (rule.pattern.empty()) {
                mLogger.printError("no rule pattern provided.");
                return Result::Fail;
            }

            std::string regex_err;
            auto regex = Regex::create(rule.pattern, Regex::Engine::Pcre, regex_err);
            if (!regex) {
                mLogger.printError("failed to compile rule pattern '" + rule.pattern + "' (" + regex_err + ").");
                return Result::Fail;
            }
            rule.regex = std::move(regex);
            mSettings.rules.emplace_back(std::move(rule));
#else
            mLogger.printError("Option --rule cannot be used as Cppcheck has not been built with rules support.");
            return Result::Fail;
#endif
        }

        // Rule file
        else if (std::strncmp(argv[i], "--rule-file=", 12) == 0) {
#ifdef HAVE_RULES
            // TODO: improved error handling - wrong root node, etc.
            // TODO: consume unused "version" attribute
            const std::string ruleFile = argv[i] + 12;
            tinyxml2::XMLDocument doc;
            const tinyxml2::XMLError err = doc.LoadFile(ruleFile.c_str());
            if (err == tinyxml2::XML_SUCCESS) {
                const tinyxml2::XMLElement *node = doc.FirstChildElement();
                // check if it is a single or multi rule configuration
                if (node && strcmp(node->Value(), "rules") == 0)
                    node = node->FirstChildElement("rule");
                for (; node && strcmp(node->Value(), "rule") == 0; node = node->NextSiblingElement()) {
                    Settings::Rule rule;

                    for (const tinyxml2::XMLElement *subnode = node->FirstChildElement(); subnode; subnode = subnode->NextSiblingElement()) {
                        const char * const subname = subnode->Name();
                        const char * const subtext = subnode->GetText();
                        if (std::strcmp(subname, "tokenlist") == 0) {
                            rule.tokenlist = empty_if_null(subtext);
                        }
                        else if (std::strcmp(subname, "pattern") == 0) {
                            rule.pattern = empty_if_null(subtext);
                        }
                        else if (std::strcmp(subname, "message") == 0) {
                            for (const tinyxml2::XMLElement *msgnode = subnode->FirstChildElement(); msgnode; msgnode = msgnode->NextSiblingElement()) {
                                const char * const msgname = msgnode->Name();
                                const char * const msgtext = msgnode->GetText();
                                if (std::strcmp(msgname, "severity") == 0) {
                                    rule.severity = severityFromString(empty_if_null(msgtext));
                                }
                                else if (std::strcmp(msgname, "id") == 0) {
                                    rule.id = empty_if_null(msgtext);
                                }
                                else if (std::strcmp(msgname, "summary") == 0) {
                                    rule.summary = empty_if_null(msgtext);
                                }
                                else {
                                    mLogger.printError("unable to load rule-file '" + ruleFile + "' - unknown element '" + msgname + "' encountered in 'message'.");
                                    return Result::Fail;
                                }
                            }
                        }
                        else if (std::strcmp(subname, "engine") == 0) {
                            const char * const engine = empty_if_null(subtext);
                            if (std::strcmp(engine, "pcre") == 0) {
                                rule.engine = Regex::Engine::Pcre;
                            }
                            else {
                                mLogger.printError(std::string("unknown regex engine '") + engine + "'.");
                                return Result::Fail;
                            }
                        }
                        else {
                            mLogger.printError("unable to load rule-file '" + ruleFile + "' - unknown element '" + subname + "' encountered in 'rule'.");
                            return Result::Fail;
                        }
                    }

                    if (rule.pattern.empty()) {
                        mLogger.printError("unable to load rule-file '" + ruleFile + "' - a rule is lacking a pattern.");
                        return Result::Fail;
                    }

                    if (rule.id.empty()) {
                        mLogger.printError("unable to load rule-file '" + ruleFile + "' - a rule is lacking an id.");
                        return Result::Fail;
                    }

                    if (rule.tokenlist.empty()) {
                        mLogger.printError("unable to load rule-file '" + ruleFile + "' - a rule is lacking a tokenlist.");
                        return Result::Fail;
                    }

                    if (rule.tokenlist != "normal" && rule.tokenlist != "define" && rule.tokenlist != "raw") {
                        mLogger.printError("unable to load rule-file '" + ruleFile + "' - a rule is using the unsupported tokenlist '" + rule.tokenlist + "'.");
                        return Result::Fail;
                    }

                    std::string regex_err;
                    auto regex = Regex::create(rule.pattern, rule.engine, regex_err);
                    if (!regex) {
                        mLogger.printError("unable to load rule-file '" + ruleFile + "' - pattern '" + rule.pattern + "' failed to compile (" + regex_err + ").");
                        return Result::Fail;
                    }
                    rule.regex = std::move(regex);

                    if (rule.severity == Severity::none) {
                        mLogger.printError("unable to load rule-file '" + ruleFile + "' - a rule has an invalid severity.");
                        return Result::Fail;
                    }

                    mSettings.rules.emplace_back(std::move(rule));
                }
            } else {
                mLogger.printError("unable to load rule-file '" + ruleFile + "' (" + tinyxml2::XMLDocument::ErrorIDToName(err) + ").");
                return Result::Fail;
            }
#else
            mLogger.printError("Option --rule-file cannot be used as Cppcheck has not been built with rules support.");
            return Result::Fail;
#endif
        }

        // Safety certified behavior
        else if (std::strcmp(argv[i], "--safety") == 0)
            mSettings.safety = true;

        // show timing information..
        else if (std::strncmp(argv[i], "--showtime=", 11) == 0) {
            const std::string showtimeMode = argv[i] + 11;
            if (showtimeMode == "file")
                mSettings.showtime = ShowTime::FILE;
            else if (showtimeMode == "file-total")
                mSettings.showtime = ShowTime::FILE_TOTAL;
            else if (showtimeMode == "summary")
                mSettings.showtime = ShowTime::SUMMARY;
            else if (showtimeMode == "top5_file")
                mSettings.showtime = ShowTime::TOP5_FILE;
            else if (showtimeMode == "top5_summary")
                mSettings.showtime = ShowTime::TOP5_SUMMARY;
            else if (showtimeMode == "none")
                mSettings.showtime = ShowTime::NONE;
            else if (showtimeMode.empty()) {
                mLogger.printError("no mode provided for --showtime");
                return Result::Fail;
            }
            else {
                mLogger.printError("unrecognized --showtime mode: '" + showtimeMode + "'. Supported modes: file, file-total, summary, top5_file, top5_summary.");
                return Result::Fail;
            }
        }

        // --std
        else if (std::strncmp(argv[i], "--std=", 6) == 0) {
            const std::string std = argv[i] + 6;
            if (!mSettings.standards.setStd(std)) {
                mLogger.printError("unknown --std value '" + std + "'");
                return Result::Fail;
            }
        }

        else if (std::strncmp(argv[i], "--suppress=", 11) == 0) {
            const std::string suppression = argv[i]+11;
            const std::string errmsg(mSuppressions.nomsg.addSuppressionLine(suppression));
            if (!errmsg.empty()) {
                mLogger.printError(errmsg);
                return Result::Fail;
            }
        }

        // Filter errors
        else if (std::strncmp(argv[i], "--suppressions-list=", 20) == 0) {
            std::string filename = argv[i]+20;
            std::ifstream f(filename);
            if (!f.is_open()) {
                std::string message("couldn't open the file: \"");
                message += filename;
                message += "\".";
                if (std::count(filename.cbegin(), filename.cend(), ',') > 0 ||
                    std::count(filename.cbegin(), filename.cend(), '.') > 1) {
                    // If user tried to pass multiple files (we can only guess that)
                    // e.g. like this: --suppressions-list=a.txt,b.txt
                    // print more detailed error message to tell user how he can solve the problem
                    message += "\nIf you want to pass two files, you can do it e.g. like this:";
                    message += "\n    cppcheck --suppressions-list=a.txt --suppressions-list=b.txt file.cpp";
                }

                mLogger.printError(message);
                return Result::Fail;
            }
            const std::string errmsg(mSuppressions.nomsg.parseFile(f));
            if (!errmsg.empty()) {
                mLogger.printError(errmsg);
                return Result::Fail;
            }
        }

        else if (std::strncmp(argv[i], "--suppress-xml=", 15) == 0) {
            const char * filename = argv[i] + 15;
            const std::string errmsg(mSuppressions.nomsg.parseXmlFile(filename));
            if (!errmsg.empty()) {
                mLogger.printError(errmsg);
                return Result::Fail;
            }
        }

        // Output formatter
        else if (std::strncmp(argv[i], "--template=", 11) == 0) {
            mSettings.templateFormat = argv[i] + 11;
            // TODO: bail out when no template is provided?

            if (mSettings.templateFormat == "gcc") {
                mSettings.templateFormat = "{bold}{file}:{line}:{column}: {magenta}warning:{default} {message} [{id}]{reset}\\n{code}";
                mSettings.templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
            } else if (mSettings.templateFormat == "daca2") {
                mSettings.daca = true;
                mSettings.templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]";
                mSettings.templateLocation = "{file}:{line}:{column}: note: {info}";
            } else if (mSettings.templateFormat == "vs")
                mSettings.templateFormat = "{file}({line}): {severity}: {message}";
            else if (mSettings.templateFormat == "edit")
                mSettings.templateFormat = "{file} +{line}: {severity}: {message}";
            else if (mSettings.templateFormat == "cppcheck1")
                mSettings.templateFormat = "{callstack}: ({severity}{inconclusive:, inconclusive}) {message}";
            else if (mSettings.templateFormat == "selfcheck") {
                mSettings.templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]\\n{code}";
                mSettings.templateLocation = "{file}:{line}:{column}: note: {info}\\n{code}";
                mSettings.daca = true;
            } else if (mSettings.templateFormat == "simple") {
                mSettings.templateFormat = "{file}:{line}:{column}: {severity}:{inconclusive:inconclusive:} {message} [{id}]";
                mSettings.templateLocation = "";
            }
            // TODO: bail out when no placeholders are found?
        }

        else if (std::strncmp(argv[i], "--template-location=", 20) == 0) {
            mSettings.templateLocation = argv[i] + 20;
            // TODO: bail out when no template is provided?
            // TODO: bail out when no placeholders are found?
        }

        else if (std::strncmp(argv[i], "--template-max-time=", 20) == 0) {
            if (!parseNumberArg(argv[i], 20, mSettings.templateMaxTime))
                return Result::Fail;
        }

        else if (std::strncmp(argv[i], "--typedef-max-time=", 19) == 0) {
            if (!parseNumberArg(argv[i], 19, mSettings.typedefMaxTime))
                return Result::Fail;
        }

        else if (std::strncmp(argv[i], "--valueflow-max-iterations=", 27) == 0) {
            if (!parseNumberArg(argv[i], 27, mSettings.vfOptions.maxIterations))
                return Result::Fail;
        }

        else if (std::strcmp(argv[i], "-v") == 0 || std::strcmp(argv[i], "--verbose") == 0)
            mSettings.verbose = true;

        // Write results in results.xml
        else if (std::strcmp(argv[i], "--xml") == 0) {
            if (outputFormatOptionProvided) {
                outputFormatOptionMixingError();
                return Result::Fail;
            }
            mSettings.outputFormat = Settings::OutputFormat::xml;
            xmlOptionProvided = true;
        }

        // Define the XML file version (and enable XML output)
        else if (std::strncmp(argv[i], "--xml-version=", 14) == 0) {
            if (outputFormatOptionProvided) {
                outputFormatOptionMixingError();
                return Result::Fail;
            }
            int tmp;
            if (!parseNumberArg(argv[i], 14, tmp))
                return Result::Fail;
            if (tmp != 2 && tmp != 3) {
                // We only have xml version 2 and 3
                mLogger.printError("'--xml-version' can only be 2 or 3.");
                return Result::Fail;
            }

            mSettings.xml_version = tmp;
            // Enable also XML if version is set
            mSettings.outputFormat = Settings::OutputFormat::xml;
            xmlOptionProvided = true;
        }

        else {
            std::string message("unrecognized command line option: \"");
            message += argv[i];
            message += "\".";
            mLogger.printError(message);
            return Result::Fail;
        }
    }

    // TODO: bail out?
    if (!executorAuto && mSettings.useSingleJob())
        mLogger.printMessage("'--executor' has no effect as only a single job will be used.");

    // Default template format..
    if (mSettings.templateFormat.empty()) {
        mSettings.templateFormat = "{bold}{file}:{line}:{column}: {red}{inconclusive:{magenta}}{severity}:{inconclusive: inconclusive:}{default} {message} [{id}]{reset}\\n{code}";
        if (mSettings.templateLocation.empty())
            mSettings.templateLocation = "{bold}{file}:{line}:{column}: {dim}note:{reset} {info}\\n{code}";
    }
    // replace static parts of the templates
    substituteTemplateFormatStatic(mSettings.templateFormat, !mSettings.outputFile.empty());
    substituteTemplateLocationStatic(mSettings.templateLocation, !mSettings.outputFile.empty());

    if (debug) {
        mSettings.debugnormal = true;
        mSettings.debugvalueflow = true;
        if (mSettings.verbose) {
            mSettings.debugast = true;
            mSettings.debugsymdb = true;
        }
    }

    if (mSettings.jobs > 1 && mSettings.buildDir.empty()) {
        // TODO: bail out instead?
        if (mSettings.checks.isEnabled(Checks::unusedFunction))
        {
            mLogger.printMessage("unusedFunction check requires --cppcheck-build-dir to be active with -j.");
            mSettings.checks.disable(Checks::unusedFunction);
            // TODO: is there some later logic to remove?
        }
        // TODO: enable
        //mLogger.printMessage("whole program analysis requires --cppcheck-build-dir to be active with -j.");
    }

    if (!mSettings.checks.isEnabled(Checks::unusedFunction))
        mSettings.unmatchedSuppressionFilters.emplace_back("unusedFunction");
    if (!mSettings.addons.count("misra"))
        mSettings.unmatchedSuppressionFilters.emplace_back("misra-*");
    if (!mSettings.premium)
        mSettings.unmatchedSuppressionFilters.emplace_back("premium-*");

    if (inputAsFilter) {
        mSettings.fileFilters.insert(mSettings.fileFilters.end(), mPathNames.cbegin(), mPathNames.cend());
        mPathNames.clear();
    }

    if (!mPathNames.empty() && projectType != ImportProject::Type::NONE) {
        mLogger.printError("--project cannot be used in conjunction with source files.");
        return Result::Fail;
    }

    // TODO: conflicts with analyzeAllVsConfigs
    if (!vsConfig.empty()) {
        // TODO: bail out when this does nothing
        project.ignoreOtherConfigs(vsConfig);
    }

    if (!platform.empty())
    {
        std::string errstr;
        if (!mSettings.platform.set(platform, errstr, lookupPaths, mSettings.debuglookup || mSettings.debuglookupPlatform)) {
            mLogger.printError(errstr);
            return Result::Fail;
        }
    }

    if (defaultSign != '\0')
        mSettings.platform.defaultSign = defaultSign;

    if (!mSettings.analyzeAllVsConfigs) {
        if (projectType != ImportProject::Type::VS_SLN &&
            projectType != ImportProject::Type::VS_SLNX &&
            projectType != ImportProject::Type::VS_VCXPROJ) {
            if (mAnalyzeAllVsConfigsSetOnCmdLine) {
                mLogger.printError("--no-analyze-all-vs-configs has no effect - no Visual Studio project provided.");
                return Result::Fail;
            }
        } else {
            // TODO: bail out when this does nothing
            project.selectOneVsConfig(mSettings.platform.type);
        }
    }

    if (!mSettings.buildDir.empty() && !Path::isDirectory(mSettings.buildDir)) {
        mLogger.printError("Directory '" + mSettings.buildDir + "' specified by --cppcheck-build-dir argument has to be existent.");
        return Result::Fail;
    }

    // Print error only if we have "real" command and expect files
    if (mPathNames.empty() && project.guiProject.pathNames.empty() && project.fileSettings.empty()) {
        // TODO: this message differs from the one reported in fillSettingsFromArgs()
        mLogger.printError("no C or C++ source files found.");
        return Result::Fail;
    }

    for (auto& path : mIgnoredPaths)
    {
        path = Path::removeQuotationMarks(std::move(path));
        path = Path::fromNativeSeparators(std::move(path));
    }

    if (!project.guiProject.pathNames.empty())
        mPathNames = project.guiProject.pathNames;

    if (!project.fileSettings.empty()) {
        project.ignorePaths(mIgnoredPaths, mSettings.debugignore);
        if (project.fileSettings.empty()) {
            mLogger.printError("no C or C++ source files found.");
            mLogger.printMessage("all paths were ignored"); // TODO: log this differently?
            return Result::Fail;
        }
        mFileSettings = project.fileSettings;
    }

    if (mSettings.debugnormal && mSettings.outputFormat == Settings::OutputFormat::xml && (mPathNames.size() > 1 || mFileSettings.size() > 1))
    {
        mLogger.printError("printing debug output in XML format does not support multiple input files.");
        return Result::Fail;
    }

    // Use paths _pathnames if no base paths for relative path output are given
    if (mSettings.basePaths.empty() && mSettings.relativePaths)
        mSettings.basePaths = mPathNames;

    return Result::Success;
}

void CmdLineParser::printHelp() const
{
    std::ostringstream oss;
    // TODO: display product name
    oss << "Cppcheck - A tool for static C/C++ code analysis\n"
        "\n"
        "Syntax:\n"
        "    cppcheck [OPTIONS] [files or paths]\n"
        "\n"
        "If a directory is given instead of a filename, *.cpp, *.cxx, *.cc, *.c++, *.c, *.ipp,\n"
        "*.ixx, *.tpp, and *.txx files are checked recursively from the given directory.\n\n"
        "Options:\n"
        "    --addon=<addon>\n"
        "                         Execute addon. i.e. --addon=misra. If options must be\n"
        "                         provided a json configuration is needed.\n"
        "    --addon-python=<python interpreter>\n"
        "                         You can specify the python interpreter either in the\n"
        "                         addon json files or through this command line option.\n"
        "                         If not present, Cppcheck will try \"python3\" first and\n"
        "                         then \"python\".\n"
        "    --cppcheck-build-dir=<dir>\n"
        "                         Cppcheck work folder. Advantages:\n"
        "                          * whole program analysis\n"
        "                          * faster analysis; Cppcheck will reuse the results if\n"
        "                            the hash for a file is unchanged.\n"
        "                          * some useful debug information, i.e. commands used to\n"
        "                            execute clang/clang-tidy/addons.\n"
        "    --check-config       Check cppcheck configuration. The normal code\n"
        "                         analysis is disabled by this flag.\n"
        "    --check-level=<level>\n"
        "                         Configure how much valueflow analysis you want:\n"
        "                          * reduced: Reduce valueflow to finish checking quickly.\n"
        "                          * normal: Cppcheck uses some compromises in the analysis so\n"
        "                            the checking will finish in reasonable time.\n"
        "                          * exhaustive: deeper analysis that you choose when you can\n"
        "                            wait.\n"
        "                         The default choice is 'normal'.\n"
        "    --check-library      Show information messages when library files have\n"
        "                         incomplete info.\n"
        "    --checkers-report=<file>\n"
        "                         Write a report of all the active checkers to the given file.\n"
        "    --clang=<path>       Experimental: Use Clang parser instead of the builtin Cppcheck\n"
        "                         parser. Takes the executable as optional parameter and\n"
        "                         defaults to `clang`. Cppcheck will run the given Clang\n"
        "                         executable, import the Clang AST and convert it into\n"
        "                         Cppcheck data. After that the normal Cppcheck analysis is\n"
        "                         used. You must have the executable in PATH if no path is\n"
        "                         given.\n"
        "    --config-exclude=<dir>\n"
        "                         Path (prefix) to be excluded from configuration\n"
        "                         checking. Preprocessor configurations defined in\n"
        "                         headers (but not sources) matching the prefix will not\n"
        "                         be considered for evaluation.\n"
        "    --config-excludes-file=<file>\n"
        "                         A file that contains a list of config-excludes\n"
        "    --disable=<severity> Disable checks with the given severity.\n"
        "                         Please refer to the documentation of --enable for\n"
        "                         further details.\n"
        "    --dump               Dump xml data for each translation unit. The dump\n"
        "                         files have the extension .dump and contain ast,\n"
        "                         tokenlist, symboldatabase, valueflow.\n"
        "    -D<ID>               Define preprocessor symbol. Unless --max-configs or\n"
        "                         --force is used, Cppcheck will only check the given\n"
        "                         configuration when -D is used.\n"
        "                         Example: '-DDEBUG=1 -D__cplusplus'.\n"
        "    -E                   Print preprocessor output on stdout and don't do any\n"
        "                         further processing.\n"
        "    --enable=<severity>  Enable additional checks grouped by severity. The available\n"
        "                         severities are:\n"
        "                          * warning\n"
        "                          * performance\n"
        "                          * portability\n"
        "                          * information\n"
        "                          * style\n"
        "                                  Enable checks with severities 'style', 'warning',\n"
        "                                  'performance' and 'portability'.\n"
        "                          * unusedFunction\n"
        "                                  Check for unused functions. It is recommended\n"
        "                                  to only enable this when the whole program is\n"
        "                                  scanned.\n"
        "                          * missingInclude\n"
        "                                  Check for missing include files.\n"
        "                          * all\n"
        "                                  Enable all checks.\n"
        "                         Pass multiple severities as a comma-separated list.\n"
        "    --error-exitcode=<n> If errors are found, integer [n] is returned instead of\n"
        "                         the default '0'. '" << EXIT_FAILURE << "' is returned\n"
        "                         if arguments are not valid or if no input files are\n"
        "                         provided. Note that your operating system can modify\n"
        "                         this value, e.g. '256' can become '0'.\n"
        "    --errorlist          Print a list of all the error messages in XML format.\n"
        "    --exitcode-suppress=<spec>\n"
        "                         Used to specify an error ID which should not result in\n"
        "                         a non-zero exitcode."
        "    --exitcode-suppressions=<file>\n"
        "                         Used when certain messages should be displayed but\n"
        "                         should not cause a non-zero exitcode.\n"
        "    --file-filter=<str>  Analyze only those files matching the given filter str.\n"
        "                         Can be used multiple times. When str is '-', the file\n"
        "                         filter will be read from standard input. When str is '+',\n"
        "                         given files on CLI will be treated as file filters.\n"
        "                         Example: --file-filter=*bar.cpp analyzes only files\n"
        "                         that end with bar.cpp.\n"
        "    --file-list=<file>   Specify the files to check in a text file. Add one\n"
        "                         filename per line. When file is '-,' the file list will\n"
        "                         be read from standard input.\n"
        "    -f, --force          Force checking of all configurations in files. If used\n"
        "                         together with '--max-configs=', the last option is the\n"
        "                         one that is effective.\n"
        "    --fsigned-char       Treat char type as signed.\n"
        "    --funsigned-char     Treat char type as unsigned.\n"
        "    -h, --help           Print this help.\n"
        "    -I <dir>             Give path to search for include files. Give several -I\n"
        "                         parameters to give several paths. First given path is\n"
        "                         searched for contained header files first. If paths are\n"
        "                         relative to source files, this is not needed.\n"
        "    --includes-file=<file>\n"
        "                         Specify directory paths to search for included header\n"
        "                         files in a text file. Add one include path per line.\n"
        "                         First given path is searched for contained header\n"
        "                         files first. If paths are relative to source files,\n"
        "                         this is not needed.\n"
        "    --include=<file>\n"
        "                         Force inclusion of a file before the checked file.\n"
        "    -i <str>             Ignore files that match <str>. <str> can be a filename\n"
        "                         or directory and can contain *,**,?. A file that is\n"
        "                         ignored will not be checked directly (the whole\n"
        "                         translation unit is skipped completely). Header files\n"
        "                         are checked indirectly when they are #include'd.\n"
        "                         Note: If you want to prevent warnings in some headers,\n"
        "                         use suppressions instead.\n"
        "    --inconclusive       Allow that Cppcheck reports even though the analysis is\n"
        "                         inconclusive.\n"
        "                         There are false positives with this option. Each result\n"
        "                         must be carefully investigated before you know if it is\n"
        "                         good or bad.\n"
        "    --inline-suppr       Enable inline suppressions. Use them by placing one or\n"
        "                         more comments, like: '// cppcheck-suppress warningId'\n"
        "                         on the lines before the warning to suppress.\n"
        "    -j <jobs>            Start <jobs> threads to do the checking simultaneously.\n"
        "    -l <load>            Specifies that no new threads should be started if\n"
        "                         there are other threads running and the load average is\n"
        "                         at least <load>.\n"
        "    --language=<language>, -x <language>\n"
        "                         Forces cppcheck to check all files as the given\n"
        "                         language. Valid values are: c, c++\n"
        "    --library=<cfg>      Load file <cfg> that contains information about types\n"
        "                         and functions. With such information Cppcheck\n"
        "                         understands your code better and therefore you\n"
        "                         get better results. The std.cfg file that is\n"
        "                         distributed with Cppcheck is loaded automatically.\n"
        "                         For more information about library files, read the\n"
        "                         manual.\n"
        "    --max-configs=<limit>\n"
        "                         Maximum number of configurations to check in a file\n"
        "                         before skipping it. Default is '12'. If used together\n"
        "                         with '--force', the last option is the one that is\n"
        "                         effective.\n"
        "    --max-ctu-depth=N    Max depth in whole program analysis. The default value\n"
        "                         is 2. A larger value will mean more errors can be found\n"
        "                         but also means the analysis will be slower.\n"
        "    --output-file=<file> Write results to file, rather than standard error.\n"
        "    --output-format=<format>\n"
        "                        Specify the output format. The available formats are:\n"
        "                          * text\n"
        "                          * sarif\n"
        "                          * xml (deprecated)\n"
        "                          * xmlv2\n"
        "                          * xmlv3\n"
        "    --platform=<type>, --platform=<file>\n"
        "                         Specifies platform specific types and sizes. The\n"
        "                         available builtin platforms are:\n"
        "                          * unix32\n"
        "                                 32 bit unix variant\n"
        "                          * unix64\n"
        "                                 64 bit unix variant\n"
        "                          * win32A\n"
        "                                 32 bit Windows ASCII character encoding\n"
        "                          * win32W\n"
        "                                 32 bit Windows UNICODE character encoding\n"
        "                          * win64\n"
        "                                 64 bit Windows\n"
        "                          * avr8\n"
        "                                 8 bit AVR microcontrollers\n"
        "                          * elbrus-e1cp\n"
        "                                 Elbrus e1c+ architecture\n"
        "                          * pic8\n"
        "                                 8 bit PIC microcontrollers\n"
        "                                 Baseline and mid-range architectures\n"
        "                          * pic8-enhanced\n"
        "                                 8 bit PIC microcontrollers\n"
        "                                 Enhanced mid-range and high end (PIC18) architectures\n"
        "                          * pic16\n"
        "                                 16 bit PIC microcontrollers\n"
        "                          * mips32\n"
        "                                 32 bit MIPS microcontrollers\n"
        "                          * native\n"
        "                                 Type sizes of host system are assumed, but no\n"
        "                                 further assumptions.\n"
        "                          * unspecified\n"
        "                                 Unknown type sizes\n"
        "    --plist-output=<path>\n"
        "                         Generate Clang-plist output files in folder.\n";

    if (mSettings.premium) {
        oss <<
            "    --premium=<option>\n"
            "                         Coding standards:\n"
            "                          * autosar           Autosar (partial)\n"
            "                          * cert-c            Cert C checking\n"
            "                          * cert-c++          Cert C++ checking\n"
            "                          * misra-c-2012      Misra C 2012\n"
            "                          * misra-c-2023      Misra C 2023\n"
            "                          * misra-c-2025      Misra C 2025\n"
            "                          * misra-c++-2008    Misra C++ 2008\n"
            "                          * misra-c++-2023    Misra C++ 2023\n"
            "                         By default 'Misra/Cert C' only checks C files.\n"
            "                         By default 'Autosar/Misra/Cert C++' only checks C++ files.\n"
            "                         To check all files, append \":all\" i.e. --premium=misra-c++-2023:all.\n"
            "                         Other:\n"
            "                          * bughunting        Soundy analysis\n"
            "                          * metrics           Calculate metrics. Metrics are only reported in xmlv3 output.\n"
            "                          * safety            Turn on safety certified behavior (ON by default)\n"
            "                          * safety-off        Turn off safety certified behavior\n"
            "    --premium-cert-c-int-precision=BITS\n"
            "                         Integer precision to use in Cert C analysis. Only needed if precision of 'int' is\n"
            "                         less than the size of 'int'. BITS shall be a positive value that is less than the\n"
            "                         size of 'int'. This precision is used for INT35-C checking.\n"
            "    --premium-license-file=<path>\n"
            "                         Provide license file path with argument.\n";
    }

    oss <<
        "    --project=<file>     Run Cppcheck on project. The <file> can be a Visual\n"
        "                         Studio Solution (*.sln) or (*.slnx), Visual Studio Project\n"
        "                         (*.vcxproj), compile database (compile_commands.json),\n"
        "                         or Borland C++ Builder 6 (*.bpr). The files to analyse,\n"
        "                         include paths, defines, platform and undefines in\n"
        "                         the specified file will be used.\n"
        "    --project-configuration=<config>\n"
        "                         If used together with a Visual Studio Solution (*.sln) or (*.slnx)\n"
        "                         or Visual Studio Project (*.vcxproj) you can limit\n"
        "                         the configuration cppcheck should check.\n"
        "                         For example: '--project-configuration=Release|Win32'\n"
        "    -q, --quiet          Do not show progress reports.\n"
        "                         Note that this option is not mutually exclusive with --verbose.\n"
        "    -rp=<paths>, --relative-paths=<paths>\n"
        "                         Use relative paths in output. When given, <paths> are\n"
        "                         used as base. You can separate multiple paths by ';'.\n"
        "                         Otherwise path where source files are searched is used.\n"
        "                         We use string comparison to create relative paths, so\n"
        "                         using e.g. ~ for home folder does not work. It is\n"
        "                         currently only possible to apply the base paths to\n"
        "                         files that are on a lower level in the directory tree.\n"
        "    --report-progress    Report progress messages while checking a file (single job only).\n"
        "    --report-type=<type> Add guideline and classification fields for specified coding standard.\n"
        "                         The available report types are:\n"
        "                          * normal           Default, only show cppcheck error ID and severity\n"
        "                          * autosar          Autosar\n"
        "                          * cert-c-2016      Cert C 2016\n"
        "                          * cert-cpp-2016    Cert C++ 2016\n"
        "                          * misra-c-2012     Misra C 2012\n"
        "                          * misra-c-2023     Misra C 2023\n"
        "                          * misra-c-2025     Misra C 2025\n"
        "                          * misra-cpp-2008   Misra C++ 2008\n"
        "                          * misra-cpp-2023   Misra C++ 2023\n"
        "    --rule=<rule>        Match regular expression.\n"
        "    --rule-file=<file>   Use given rule file. For more information, see:\n"
        "                         http://sourceforge.net/projects/cppcheck/files/Articles/\n"
        "    --safety             Enable safety-certified checking mode: display checker summary, enforce\n"
        "                         stricter checks for critical errors, and return a non-zero exit code\n"
        "                         if such errors occur.\n"
        "    --showtime=<mode>    Show timing information.\n"
        "                         The available modes are:\n"
        "                          * none\n"
        "                                 Show nothing (default)\n"
        "                          * file\n"
        "                                 Show for each processed file\n"
        "                          * file-total\n"
        "                                 Show total time only for each processed file\n"
        "                          * summary\n"
        "                                 Show a summary at the end\n"
        "                          * top5_file\n"
        "                                 Show the top 5 for each processed file\n"
        "                          * top5_summary\n"
        "                                 Show the top 5 summary at the end\n"
        "    --std=<id>           Set standard.\n"
        "                         The available options are:\n"
        "                          * c89\n"
        "                                 C code is C89 compatible\n"
        "                          * c99\n"
        "                                 C code is C99 compatible\n"
        "                          * c11\n"
        "                                 C code is C11 compatible (default)\n"
        "                          * c++03\n"
        "                                 C++ code is C++03 compatible\n"
        "                          * c++11\n"
        "                                 C++ code is C++11 compatible\n"
        "                          * c++14\n"
        "                                 C++ code is C++14 compatible\n"
        "                          * c++17\n"
        "                                 C++ code is C++17 compatible\n"
        "                          * c++20\n"
        "                                 C++ code is C++20 compatible (default)\n"
        "    --suppress=<spec>    Suppress warnings that match <spec>. The format of\n"
        "                         <spec> is:\n"
        "                         [error id]:[filename]:[line]\n"
        "                         The [filename] and [line] are optional. If [error id]\n"
        "                         is a wildcard '*', all error ids match.\n"
        "    --suppressions-list=<file>\n"
        "                         Suppress warnings listed in the file. Each suppression\n"
        "                         is in the same format as <spec> above.\n"
        "    --suppress-xml=<file>\n"
        "                         Suppress warnings listed in a xml file. XML file should\n"
        "                         follow the manual.pdf format specified in section.\n"
        "                         `6.4 XML suppressions` .\n"
        "    --template='<text>'  Format the error messages. Available fields:\n"
        "                           {file}              file name\n"
        "                           {line}              line number\n"
        "                           {column}            column number\n"
        "                           {callstack}         show a callstack. Example:\n"
        "                                                 [file.c:1] -> [file.c:100]\n"
        "                           {inconclusive:text} if warning is inconclusive, text\n"
        "                                               is written\n"
        "                           {severity}          severity\n"
        "                           {message}           warning message\n"
        "                           {id}                warning id\n"
        "                           {cwe}               CWE id (Common Weakness Enumeration)\n"
        "                           {code}              show the real code\n"
        "                           \\t                 insert tab\n"
        "                           \\n                 insert newline\n"
        "                           \\r                 insert carriage return\n"
        "                         Example formats:\n"
        "                         '{file}:{line},{severity},{id},{message}' or\n"
        "                         '{file}({line}):({severity}) {message}' or\n"
        "                         '{callstack} {message}'\n"
        "                         Pre-defined templates: gcc (default), cppcheck1 (old default), vs, edit.\n"
        // Note: template daca2 also exists, but is for internal use (cppcheck scripts).
        "    --template-location='<text>'\n"
        "                         Format error message location. If this is not provided\n"
        "                         then no extra location info is shown.\n"
        "                         Available fields:\n"
        "                           {file}      file name\n"
        "                           {line}      line number\n"
        "                           {column}    column number\n"
        "                           {info}      location info\n"
        "                           {code}      show the real code\n"
        "                           \\t         insert tab\n"
        "                           \\n         insert newline\n"
        "                           \\r         insert carriage return\n"
        "                         Example format (gcc-like):\n"
        "                         '{file}:{line}:{column}: note: {info}\\n{code}'\n"
        "    -U<ID>               Undefine preprocessor symbol. Use -U to explicitly\n"
        "                         hide certain #ifdef <ID> code paths from checking.\n"
        "                         Example: '-UDEBUG'\n"
        "    -v, --verbose        Output more detailed error information.\n"
        "                         Note that this option is not mutually exclusive with --quiet.\n"
        "    --version            Print out version number.\n"
        "    --xml                Write results in xml format to error stream (stderr).\n"
        "\n"
        "Example usage:\n"
        "  # Recursively check the current folder. Print the progress on the screen and\n"
        "  # write errors to a file:\n"
        "  cppcheck . 2> err.txt\n"
        "\n"
        "  # Recursively check ../myproject/ and don't print progress:\n"
        "  cppcheck --quiet ../myproject/\n"
        "\n"
        "  # Check test.cpp, enable all checks:\n"
        "  cppcheck --enable=all --inconclusive --library=posix test.cpp\n"
        "\n"
        "  # Check f.cpp and search include files from inc1/ and inc2/:\n"
        "  cppcheck -I inc1/ -I inc2/ f.cpp\n"
        "\n"
        "For more information:\n"
        "    " << mSettings.manualUrl << "\n"
        "\n"
        "Many thanks to the 3rd party libraries we use:\n"
        " * tinyxml2 -- loading project/library/ctu files.\n"
        " * picojson -- loading compile database.\n"
        " * pcre -- rules.\n"
        " * qt -- used in GUI\n";

    mLogger.printRaw(oss.str());
}

std::string CmdLineParser::getVersion() const {
    // TODO: this should not contain the version - it should set the extraVersion
    if (!mSettings.cppcheckCfgProductName.empty())
        return mSettings.cppcheckCfgProductName;
    const char * const extraVersion = CppCheck::extraVersion();
    if (*extraVersion != '\0')
        return std::string("Cppcheck ") + CppCheck::version() + " ("+ extraVersion + ')';
    return std::string("Cppcheck ") + CppCheck::version();
}

bool CmdLineParser::tryLoadLibrary(Library& destination, const std::string& basepath, const char* filename, bool debug)
{
    const Library::Error err = destination.load(basepath.c_str(), filename, debug);

    if (err.errorcode == Library::ErrorCode::UNKNOWN_ELEMENT)
        mLogger.printMessage("Found unknown elements in configuration file '" + std::string(filename) + "': " + err.reason); // TODO: print as errors
    else if (err.errorcode != Library::ErrorCode::OK) {
        std::string msg = "Failed to load library configuration file '" + std::string(filename) + "'. ";
        switch (err.errorcode) {
        case Library::ErrorCode::OK:
            break;
        case Library::ErrorCode::FILE_NOT_FOUND:
            msg += "File not found";
            break;
        case Library::ErrorCode::BAD_XML:
            msg += "Bad XML";
            break;
        case Library::ErrorCode::UNKNOWN_ELEMENT:
            msg += "Unexpected element";
            break;
        case Library::ErrorCode::MISSING_ATTRIBUTE:
            msg +="Missing attribute";
            break;
        case Library::ErrorCode::BAD_ATTRIBUTE_VALUE:
            msg += "Bad attribute value";
            break;
        case Library::ErrorCode::UNSUPPORTED_FORMAT:
            msg += "File is of unsupported format version";
            break;
        case Library::ErrorCode::DUPLICATE_PLATFORM_TYPE:
            msg += "Duplicate platform type";
            break;
        case Library::ErrorCode::PLATFORM_TYPE_REDEFINED:
            msg += "Platform type redefined";
            break;
        case Library::ErrorCode::DUPLICATE_DEFINE:
            msg += "Duplicate define";
            break;
        }
        if (!err.reason.empty())
            msg += " '" + err.reason + "'";
        mLogger.printMessage(msg); // TODO: print as errors
        return false;
    }
    return true;
}

bool CmdLineParser::loadLibraries(Settings& settings)
{
    if (!tryLoadLibrary(settings.library, settings.exename, "std.cfg", settings.debuglookup || settings.debuglookupLibrary)) {
        const std::string msg("Failed to load std.cfg. Your Cppcheck installation is broken, please re-install.");
#ifdef FILESDIR
        const std::string details("The Cppcheck binary was compiled with FILESDIR set to \""
                                  FILESDIR "\" and will therefore search for "
                                  "std.cfg in " FILESDIR "/cfg.");
#else
        const std::string cfgfolder(Path::fromNativeSeparators(Path::getPathFromFilename(settings.exename)) + "cfg");
        const std::string details("The Cppcheck binary was compiled without FILESDIR set. Either the "
                                  "std.cfg should be available in " + cfgfolder + " or the FILESDIR "
                                  "should be configured.");
#endif
        mLogger.printRaw(msg + " " + details); // TODO: do not print as raw?
        return false;
    }

    bool result = true;
    for (const auto& lib : settings.libraries) {
        if (!tryLoadLibrary(settings.library, settings.exename, lib.c_str(), settings.debuglookup || settings.debuglookupLibrary)) {
            result = false;
        }
    }
    return result;
}

bool CmdLineParser::loadAddons(Settings& settings)
{
    bool result = true;
    for (const std::string &addon: settings.addons) {
        AddonInfo addonInfo;
        const std::string failedToGetAddonInfo = addonInfo.getAddonInfo(addon, settings.exename, settings.debuglookup || settings.debuglookupAddon);
        if (!failedToGetAddonInfo.empty()) {
            mLogger.printRaw(failedToGetAddonInfo); // TODO: do not print as raw
            result = false;
            continue;
        }
        settings.addonInfos.emplace_back(std::move(addonInfo));
    }
    return result;
}

bool CmdLineParser::loadCppcheckCfg()
{
    if (!mSettings.settingsFiles.empty())
    {
        // should never happen - programming error
        mLogger.printError("cppcheck.cfg has already been loaded from " + mSettings.settingsFiles[0]);
        return false;
    }
    const std::string cfgErr = Settings::loadCppcheckCfg(mSettings, mSuppressions, mSettings.debuglookup || mSettings.debuglookupConfig);
    if (!cfgErr.empty()) {
        // TODO: log full path
        mLogger.printError("could not load cppcheck.cfg - " + cfgErr);
        return false;
    }
    mSettings.premium = startsWith(mSettings.cppcheckCfgProductName, "Cppcheck Premium");
    return true;
}

std::list<FileWithDetails> CmdLineParser::filterFiles(const std::vector<std::string>& fileFilters,
                                                      const std::list<FileWithDetails>& filesResolved) {
    std::list<FileWithDetails> files;
    PathMatch filtermatcher(fileFilters, Path::getCurrentPath());
    std::copy_if(filesResolved.cbegin(), filesResolved.cend(), std::inserter(files, files.end()), [&](const FileWithDetails& entry) {
        return filtermatcher.match(entry.path());
    });
    return files;
}

void CmdLineParser::outputFormatOptionMixingError() const
{
    mLogger.printError("'--output-format' and '--xml...' may not be used in conjunction.");
}
