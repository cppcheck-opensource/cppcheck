/* -*- C++ -*-
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

//---------------------------------------------------------------------------
#ifndef importprojectH
#define importprojectH
//---------------------------------------------------------------------------

#include "config.h"
#include "filesettings.h"
#include "platform.h"
#include "utils.h"

#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

class Settings;
struct Suppressions;
namespace tinyxml2 {
    class XMLDocument;
}

/// @addtogroup Core
/// @{

namespace cppcheck {
    struct stricmp {
        bool operator()(const std::string &lhs, const std::string &rhs) const {
            return caseInsensitiveStringCompare(lhs,rhs) < 0;
        }
    };
}

/**
 * @brief Importing project settings.
 */
class CPPCHECKLIB WARN_UNUSED ImportProject {
public:
    enum class Type : std::uint8_t {
        NONE,
        UNKNOWN,
        MISSING,
        FAILURE,
        COMPILE_DB,
        VS_SLN,
        VS_SLNX,
        VS_VCXPROJ,
        BORLAND,
        CPPCHECK_GUI
    };

protected:
    static void fsSetDefines(FileSettings& fs, std::string defs);
    static void fsSetIncludePaths(FileSettings& fs, const std::string &basepath, const std::list<std::string> &in, std::map<std::string, std::string, cppcheck::stricmp> &variables);

public:
    std::list<FileSettings> fileSettings;
    std::vector<std::string> errors;

    ImportProject() = default;
    virtual ~ImportProject() = default;
    ImportProject(const ImportProject&) = default;
    ImportProject& operator=(const ImportProject&) & = default;

    void selectOneVsConfig(Platform::Type platform);
    void selectVsConfigurations(Platform::Type platform, const std::vector<std::string> &configurations);

    std::list<std::string> getVSConfigs();

    // Cppcheck GUI output
    struct {
        std::vector<std::string> pathNames;
        std::list<std::string> libraries;
        std::list<std::string> excludedPaths;
        std::list<std::string> checkVsConfigs;
        std::string projectFile;
        std::string platform;
    } guiProject;

    void ignorePaths(const std::vector<std::string> &ipaths, bool debug = false);
    void ignoreOtherConfigs(const std::string &cfg);

    Type import(const std::string &filename, Settings *settings=nullptr, Suppressions *supprs=nullptr);
protected:
    bool importCompileCommands(std::istream &istr);
    bool importCppcheckGuiProject(std::istream &istr, Settings &settings, Suppressions &supprs);
    static std::string collectArgs(const std::string &cmd, std::vector<std::string> &args);

    struct SharedItemsProject {
        bool successful = false;
        std::string pathToProjectFile;
        std::vector<std::string> includePaths;
        std::vector<std::string> sourceFiles;
    };

    bool importVcxproj(const std::string &filename, std::map<std::string, std::string, cppcheck::stricmp> &variables, const std::string &additionalIncludeDirectories, const std::vector<std::string> &fileFilters, std::vector<SharedItemsProject> &cache);
    bool importVcxproj(const std::string &filename, const tinyxml2::XMLDocument &doc, std::map<std::string, std::string, cppcheck::stricmp> &variables, const std::string &additionalIncludeDirectories, const std::vector<std::string> &fileFilters, std::vector<SharedItemsProject> &cache);

private:
    static void parseArgs(FileSettings &fs, const std::vector<std::string> &args);
    void setRelativePaths(const std::string &filename);

    bool importSln(std::istream &istr, const std::string &path, const std::vector<std::string> &fileFilters);
    bool importSlnx(const std::string& filename, const std::vector<std::string>& fileFilters);
    SharedItemsProject importVcxitems(const std::string &filename, const std::vector<std::string> &fileFilters, std::vector<SharedItemsProject> &cache);
    bool importBcb6Prj(const std::string &projectFilename);

    std::string mPath;
    std::set<std::string> mAllVSConfigs;
};


namespace CppcheckXml {
    static constexpr char ProjectElementName[] = "project";
    static constexpr char ProjectVersionAttrib[] = "version";
    static constexpr char ProjectFileVersion[] = "1";
    static constexpr char BuildDirElementName[] = "builddir";
    static constexpr char ImportProjectElementName[] = "importproject";
    static constexpr char AnalyzeAllVsConfigsElementName[] = "analyze-all-vs-configs";
    static constexpr char Parser[] = "parser";
    static constexpr char IncludeDirElementName[] = "includedir";
    static constexpr char DirElementName[] = "dir";
    static constexpr char DirNameAttrib[] = "name";
    static constexpr char DefinesElementName[] = "defines";
    static constexpr char DefineName[] = "define";
    static constexpr char DefineNameAttrib[] = "name";
    static constexpr char UndefinesElementName[] = "undefines";
    static constexpr char UndefineName[] = "undefine";
    static constexpr char PathsElementName[] = "paths";
    static constexpr char PathName[] = "dir";
    static constexpr char PathNameAttrib[] = "name";
    static constexpr char RootPathName[] = "root";
    static constexpr char RootPathNameAttrib[] = "name";
    static constexpr char IgnoreElementName[] = "ignore";
    static constexpr char IgnorePathName[] = "path";
    static constexpr char IgnorePathNameAttrib[] = "name";
    static constexpr char ExcludeElementName[] = "exclude";
    static constexpr char ExcludePathName[] = "path";
    static constexpr char ExcludePathNameAttrib[] = "name";
    static constexpr char FunctionContracts[] = "function-contracts";
    static constexpr char VariableContractsElementName[] = "variable-contracts";
    static constexpr char LibrariesElementName[] = "libraries";
    static constexpr char LibraryElementName[] = "library";
    static constexpr char PlatformElementName[] = "platform";
    static constexpr char SuppressionsElementName[] = "suppressions";
    static constexpr char SuppressionElementName[] = "suppression";
    static constexpr char AddonElementName[] = "addon";
    static constexpr char AddonsElementName[] = "addons";
    static constexpr char ToolElementName[] = "tool";
    static constexpr char ToolsElementName[] = "tools";
    static constexpr char TagsElementName[] = "tags";
    static constexpr char TagElementName[] = "tag";
    static constexpr char TagWarningsElementName[] = "tag-warnings";
    static constexpr char TagAttributeName[] = "tag";
    static constexpr char WarningElementName[] = "warning";
    static constexpr char HashAttributeName[] = "hash";
    static constexpr char CheckLevelExhaustiveElementName[] = "check-level-exhaustive";
    static constexpr char CheckLevelNormalElementName[] = "check-level-normal";
    static constexpr char CheckLevelReducedElementName[] = "check-level-reduced";
    static constexpr char CheckHeadersElementName[] = "check-headers";
    static constexpr char CheckUnusedTemplatesElementName[] = "check-unused-templates";
    static constexpr char MaxCtuDepthElementName[] = "max-ctu-depth";
    static constexpr char MaxTemplateRecursionElementName[] = "max-template-recursion";
    static constexpr char CheckUnknownFunctionReturn[] = "check-unknown-function-return-values";
    static constexpr char InlineSuppression[] = "inline-suppression";
    static constexpr char ClangTidy[] = "clang-tidy";
    static constexpr char Name[] = "name";
    static constexpr char VSConfigurationElementName[] = "vs-configurations";
    static constexpr char VSConfigurationName[] = "config";
    // Cppcheck Premium
    static constexpr char BughuntingElementName[] = "bug-hunting";
    static constexpr char CodingStandardsElementName[] = "coding-standards";
    static constexpr char CodingStandardElementName[] = "coding-standard";
    static constexpr char CertIntPrecisionElementName[] = "cert-c-int-precision";
    static constexpr char LicenseFileElementName[] = "license-file";
    static constexpr char ProjectNameElementName[] = "project-name";
}

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
