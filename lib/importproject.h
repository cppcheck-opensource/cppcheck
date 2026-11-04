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

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

class Settings;
struct Suppressions;

namespace tinyxml2 {
    class XMLElement;
}

/// @addtogroup Core
/// @{

namespace cppcheck {
    struct stricmp {
        bool operator()(const std::string &lhs, const std::string &rhs) const {
            return caseInsensitiveStringCompare(lhs,rhs) < 0;
        }
    };

    namespace testing
    {
        CPPCHECKLIB bool evaluateVcxprojCondition(const std::string& condition, const std::string& configuration, const std::string& platform);
        CPPCHECKLIB std::string expandMSBuildExpression(const std::string& expr);
        CPPCHECKLIB std::string expandMSBuildProperties(const std::string& expr, const std::string& configuration, const std::string& platform);
        CPPCHECKLIB std::string resolveVcxitemsFilename(const std::string& items, const std::string& projectDir);
    }
}

using PropertiesMap = std::map<std::string, std::string, cppcheck::stricmp>;
using MetadataMap = std::map<std::string, std::string, cppcheck::stricmp>;

/**
 * @brief Importing project settings.
 */
class CPPCHECKLIB WARN_UNUSED ImportProject {
public:
    friend CPPCHECKLIB bool cppcheck::testing::evaluateVcxprojCondition(const std::string &condition, const std::string &configuration, const std::string &platform);
    friend CPPCHECKLIB std::string cppcheck::testing::expandMSBuildExpression(const std::string &expr);
    friend CPPCHECKLIB std::string cppcheck::testing::expandMSBuildProperties(const std::string &expr, const std::string &configuration, const std::string &platform);
    friend CPPCHECKLIB std::string cppcheck::testing::resolveVcxitemsFilename(const std::string &items, const std::string &projectDir);

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
    enum class ImportResult : std::uint8_t {
        Ok,
        Cycle,         // Visual Studio/MSBuild reports a circular import; continue safely
        NotResolvable,
        NotFound,
        NotValid,
    };

    /// Visual Studio (like MSBuild) evaluates a project in separate passes over the
    /// whole resolved import graph: every PropertyGroup/Import first (so every
    /// property, wherever in the graph it is set, is final before anything else
    /// runs), then every ItemDefinitionGroup, then every ItemGroup. A PropertyGroup
    /// positioned after an ItemDefinitionGroup -- or reached through a file imported
    /// later in the document -- is still visible to that earlier ItemDefinitionGroup.
    enum class EvalPhase : std::uint8_t {
        Properties, ///< Pass 1: PropertyGroup + Import/ImportGroup only; decides and records the import graph
        ItemDefs,   ///< Pass 2: ItemDefinitionGroup only; replays the recorded import graph
        Items,      ///< Pass 3: ItemGroup only; replays the recorded import graph
        Discover,   ///< Structural discovery bootstrap scan: PropertyGroup/ImportGroup/
                    ///< Import/Choose are walked like Properties, but every Condition is
                    ///< treated as satisfied and Choose explores every branch instead of
                    ///< selecting one (see mDiscovering), and any errors/debugs generated
                    ///< are discarded rather than surfaced.
    };

protected:
    static void fsSetDefines(FileSettings& fs, std::string defs);
    void fsSetIncludePaths(FileSettings& fs, const std::string &basepath, const std::list<std::string> &in, const PropertiesMap &properties);
    /** Set the project path prefix used to resolve relative paths in the project file.
     *  Normally set automatically by import(); exposed here so unit tests can exercise
     *  path-joining without needing a real file on disk. */
    // cppcheck-suppress unusedFunction
    void setProjectPath(const std::string& p) {
        mPath = p;
    }

public:
    std::list<FileSettings> fileSettings;
    std::vector<std::string> errors;
    std::vector<std::string> debugs;

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

    static const std::string &importResultStr(ImportResult result);

protected:
    bool processCompileCommands(std::istream &istr);
    bool importCppcheckGuiProject(std::istream &istr, Settings &settings, Suppressions &supprs);
    static std::string collectArgs(const std::string &cmd, std::vector<std::string> &args);
    void setRelativePaths(const std::string &filename);

private:
    struct PropertyValueExpander;
    class ConditionParser;

    static void parseArgs(FileSettings &fs, const std::vector<std::string> &args);

    bool importBcb6Prj(const std::string &projectFilename);

    struct ProjectConfiguration {
        explicit ProjectConfiguration(const tinyxml2::XMLElement *cfg);

        std::string name;
        std::string configuration;
        enum : std::uint8_t { Win32, x64, ARM64, ARM64EC, ARM, Unknown } platform = Unknown;
        std::string platformStr;
    };

    struct ItemGroupClCompile {
        explicit ItemGroupClCompile(std::string filename) : filename(std::move(filename)) {}
        std::string filename;
        MetadataMap metadata;
        const std::string &get(const std::string &key) const {
            static const std::string empty;
            const auto it = metadata.find(key);
            return (it != metadata.end()) ? it->second : empty;
        }
    };

    bool importSln(std::istream &istr, const std::string &filename, const std::vector<std::string> &fileFilters);
    bool importSlnx(const std::string& filename, const std::vector<std::string>& fileFilters);
    bool importVcxproj(const std::string &filename, PropertiesMap &properties, const std::vector<std::string> &fileFilters);

    ImportResult processImport(const std::string &file,
                               PropertiesMap &properties,
                               MetadataMap &metadata,
                               std::list<ItemGroupClCompile> &compileList,
                               std::list<ProjectConfiguration> &projectConfigurationList,
                               std::unordered_set<std::string> &importStack,
                               EvalPhase phase);
    ImportResult processImportProject(const tinyxml2::XMLElement *node,
                                      const std::string &projectDir,
                                      PropertiesMap &properties,
                                      MetadataMap &metadata,
                                      std::list<ItemGroupClCompile> &compileList,
                                      std::list<ProjectConfiguration> &projectConfigurationList,
                                      std::unordered_set<std::string> &importStack,
                                      EvalPhase phase);
    ImportResult processImportGroup(const tinyxml2::XMLElement *node,
                                    const std::string &baseDir,
                                    PropertiesMap &properties,
                                    MetadataMap &metadata,
                                    std::list<ItemGroupClCompile> &compileList,
                                    std::list<ProjectConfiguration> &projectConfigurationList,
                                    std::unordered_set<std::string> &importStack,
                                    EvalPhase phase);
    // Decide (Properties pass) or replay (ItemDefs/Items pass) which single
    // <When>/<Otherwise> child of a <Choose> is taken, then process that child's
    // own children (PropertyGroup/ItemDefinitionGroup/ItemGroup/ImportGroup/nested
    // Choose) exactly as processElementChildren() would process them inline.
    // During the discovery bootstrap scan (mDiscovering set) this selection is
    // skipped entirely: every <When> and any <Otherwise> is processed instead of
    // choosing one -- see the mDiscovering branch at the top of the definition.
    ImportResult processChoose(const tinyxml2::XMLElement *node,
                               const std::string &baseDir,
                               PropertiesMap &properties,
                               MetadataMap &metadata,
                               std::list<ItemGroupClCompile> &compileList,
                               std::list<ProjectConfiguration> &projectConfigurationList,
                               std::unordered_set<std::string> &importStack,
                               EvalPhase phase);
    ImportResult processCompile(const tinyxml2::XMLElement *node,
                                const std::string &projectDir,
                                const PropertiesMap &properties,
                                const MetadataMap &metadata,
                                std::list<ItemGroupClCompile> &compileList);
    ImportResult processElementChildren(const tinyxml2::XMLElement *parent,
                                        const std::string &baseDir,
                                        PropertiesMap &properties,
                                        MetadataMap &metadata,
                                        std::list<ItemGroupClCompile> &compileList,
                                        std::list<ProjectConfiguration> &projectConfigurationList,
                                        std::unordered_set<std::string> &importStack,
                                        EvalPhase phase);
    void applyClCompileUpdate(const tinyxml2::XMLElement *node,
                              const std::string &baseDir,
                              const PropertiesMap &properties,
                              std::list<ItemGroupClCompile> &compileList);
    void applyClCompileRemove(const tinyxml2::XMLElement *node,
                              const std::string &baseDir,
                              const PropertiesMap &properties,
                              std::list<ItemGroupClCompile> &compileList);
    // Returns (original-segment, absolute-path) pairs.  The original segment is
    // the spec after property expansion but before toAbsolute(), preserving the
    // relative form needed to compute %(RelativeDir) in processCompile().
    std::pair<std::string, std::string> expandItemSpec(const std::string &spec,
                                                       const std::string &projectDir,
                                                       const PropertiesMap &properties);
    std::string applyMSBuildStaticFunction(const std::string &className,
                                           const std::string &member,
                                           const std::vector<std::string> &args,
                                           const PropertiesMap *properties = nullptr);
    void applyClCompileChild(const tinyxml2::XMLElement *e1,
                             const PropertiesMap &properties,
                             MetadataMap &metadata);
    void expandMSBuildVariables(std::string &s, const PropertiesMap &properties);
    bool evalCondition(const std::string &condition, const PropertiesMap &properties);
    bool conditionIsTrue(const tinyxml2::XMLElement *node, const PropertiesMap &properties);
    bool hasName(const tinyxml2::XMLElement *node, const char *nodeName, const PropertiesMap &properties);
    bool hasNameAndLabel(const tinyxml2::XMLElement *node, const char *nodeName, const char *nodeAttr, const PropertiesMap &properties);
    bool hasNameAndNotLabel(const tinyxml2::XMLElement * node, const char *nodeName, const char *nodeAttr, const PropertiesMap & properties);
    // Decide (Properties pass) or replay (ItemDefs/Items pass) whether one import-graph
    // branch point -- an <Import>, an <ImportGroup>, or a synthetic ForceImportXxx /
    // Directory.Build.* import attempt performed by attemptSyntheticImport() -- is
    // taken. During the Properties pass `conditionHolds` (already evaluated by the
    // caller) is recorded and returned as-is, and `file` (the target path the caller
    // has already resolved, if any) is recorded alongside it. During ItemDefs/Items,
    // both the condition and the file the caller just (re)computed are ignored, and
    // the recorded pair from the Properties pass is replayed instead: the resolved
    // file's own $(...) references may include a property that was still unset (or
    // held a different value) at the point Properties visited this branch, so
    // recomputing it against the now-final properties could resolve to a different
    // file than Properties actually walked, which would desync the properties
    // Properties built from the items/definitions ItemDefs/Items evaluate. Outside
    // the three-pass evaluation (mImportGraph inactive) this simply returns
    // `conditionHolds` unchanged.
    bool importGraphDecision(bool conditionHolds, std::string &file);
    // Same decide/replay discipline as importGraphDecision(), for a <Choose>'s
    // branch selection instead of an <Import>'s target file. During the
    // Properties pass `matched` (already computed by the caller: did any
    // <When> match, or is there an <Otherwise> to fall back to) and `branch`
    // (the 0-based index, among the Choose's <When>/<Otherwise> children in
    // document order, of the one that matched) are recorded and returned/left
    // as-is. During ItemDefs/Items, both are ignored and the recorded pair is
    // replayed instead: a <When>'s Condition may reference a property that was
    // still unset (or held a different value) when the Properties pass reached
    // this Choose, so recomputing it against the now-final properties could
    // select a different branch than Properties actually walked -- desyncing
    // the properties Properties built from the items/definitions ItemDefs/Items
    // evaluate, exactly like an Import resolving to a different file would.
    // Outside the three-pass evaluation (mImportGraph inactive) this simply
    // returns `matched` unchanged.
    bool importGraphChooseDecision(bool matched, std::size_t &branch);
    // Attempt one of the synthetic (non-XML) imports the Microsoft.Cpp.Default.props /
    // .props / .targets emulation performs implicitly: the ForceImportBeforeXxx /
    // ForceImportAfterXxx hook properties, and the Directory.Build.props/.targets
    // auto-import. `file` is the already-resolved target path, or empty if there is
    // nothing to import. Participates in the same decide/replay discipline as a
    // regular <Import> element via importGraphDecision().
    ImportResult attemptSyntheticImport(std::string file,
                                        PropertiesMap &properties,
                                        MetadataMap &metadata,
                                        std::list<ItemGroupClCompile> &compileList,
                                        std::list<ProjectConfiguration> &projectConfigurationList,
                                        std::unordered_set<std::string> &importStack,
                                        EvalPhase phase);
    void checkUnexpandedExpressions(const std::string &text, const char *context);
    bool simplifyPathWithVariables(std::string &s, const PropertiesMap &properties);
    void addProperty(const tinyxml2::XMLElement *node, PropertiesMap &properties);
    void addMetadata(const tinyxml2::XMLElement *node, const PropertiesMap &properties, MetadataMap &metadata);
    std::string getMetadata(const tinyxml2::XMLElement *node, const PropertiesMap &properties, const MetadataMap &metadata, const std::string &original);
    std::string toAbsolute(const std::string &filename, const std::string &baseDir, const PropertiesMap &properties);
    static std::string toAbsoluteExpanded(const std::string &filename, const std::string &baseDir);
    static std::string toAbsolute(const std::string &path);
    static void setSolution(const std::string &filename, PropertiesMap &properties);
    void addDebug(const std::string &msg);

    /// Tracks the state of the current three-pass (Properties/ItemDefs/Items)
    /// evaluation for one project configuration. During the Properties pass every
    /// import-graph branch point is decided and its outcome recorded; during the
    /// ItemDefs and Items passes those decisions are replayed in the same order
    /// instead of being re-evaluated, so all three passes walk the identical
    /// resolved graph -- exactly what real MSBuild/Visual Studio evaluation does by
    /// resolving imports once, during property evaluation, and reusing that graph
    /// for item definitions and items.
    struct ImportGraph {
        /// One import-graph branch point's outcome, as decided during the
        /// Properties pass: whether it was taken and, if so, either the target
        /// file it resolved to at that time (Import/ImportGroup) or the index of
        /// the <When>/<Otherwise> child it selected (Choose) -- whichever applies
        /// to this branch point's kind -- frozen so later passes reuse it
        /// verbatim instead of recomputing it against possibly-different state.
        struct Decision {
            bool taken = false;
            std::string file;      ///< Import/ImportGroup only; unused for Choose.
            std::size_t branch = 0; ///< Choose only; unused for Import/ImportGroup.
        };
        /// Files already imported during the current pass. An imported file is
        /// processed at most once per pass; a repeated <Import> of it is ignored
        /// (MSB4011). Reset at the start of each pass; which files end up in it is
        /// an automatic consequence of replaying the same decisions in the same
        /// order, so it does not itself need to be recorded/replayed.
        std::unordered_set<std::string> imported;
        /// Per container file (by import-file-key): the decision for each
        /// import-graph branch point encountered while walking that file's
        /// children, in traversal order. Populated during the Properties pass;
        /// read-only afterwards.
        std::map<std::string, std::vector<Decision>> decisions;
        /// Per container file: how far ItemDefs/Items replay has consumed that
        /// file's decisions vector. Reset at the start of each replay pass.
        std::map<std::string, std::size_t> cursor;
        /// Import-file-key of the file whose children are currently being walked;
        /// keys 'decisions'/'cursor'. Maintained by CurrentFileGuard in processImport().
        std::string currentFile;
        bool active = false;  ///< true only inside importVcxproj's per-configuration loop
        bool replay = false;  ///< true during ItemDefs/Items: replay recorded decisions, don't decide
    };

    std::string mPath;
    std::set<std::string> mAllVSConfigs;
    /// Names of properties that resolve to the SAME value in every one of this
    /// project's configurations, computed once per project file by a priming
    /// walk over projectConfigurationList (see importVcxproj()) before the
    /// real per-configuration passes run. A macro is unsafe in a project item
    /// path only if its value could differ by configuration -- see
    /// expandItemSpec()'s use of this alongside invariantItemPathProperties()
    /// -- so this set, not just that fixed name list, is what a macro's name
    /// is checked against. Cleared and repopulated at the top of each
    /// importVcxproj() call; empty (and therefore inert) before the first one.
    /// MSBuild property names are case-insensitive (PropertyGroup and
    /// PropertiesMap itself, above, agree on this), so this uses the same
    /// cppcheck::stricmp comparator PropertiesMap does -- otherwise "MyRoot"
    /// set in one configuration's PropertyGroup and "myroot" set in another's
    /// would be tracked as two unrelated names instead of one property, and
    /// each could wrongly look config-invariant on its own even when the
    /// property's real value differs by configuration. The priming walk
    /// (see its comment in importVcxproj()) also tracks a property that only
    /// SOME configurations set at all -- a value in one configuration and no
    /// PropertyGroup for it whatsoever in another is itself a difference,
    /// exactly like real MSBuild resolving it to "" wherever nothing sets
    /// it -- rather than considering the property only where it happens to
    /// be present.
    std::set<std::string, cppcheck::stricmp> mConfigInvariantProperties;
    ImportGraph mImportGraph;
    /// True only while importVcxproj()'s discovery bootstrap scan (used when a
    /// project has no inline ProjectConfigurations) is walking the document. While
    /// set, conditionIsTrue() treats every Condition as satisfied and processChoose()
    /// explores every <When>/<Otherwise> instead of selecting one: discovery's only
    /// goal is to enumerate every ProjectConfiguration reachable through ANY
    /// combination of property values, and it cannot correctly predict which single
    /// branch a real build would take for values -- Configuration/Platform above
    /// all -- that are themselves what is being discovered. See DiscoveringGuard.
    bool mDiscovering = false;
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
    static constexpr char UserIncludeElementName[] = "user-include";
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
    static constexpr char ProjectNameElementName[] = "project-name";
}

/// @}
//---------------------------------------------------------------------------
#endif // importprojectH
