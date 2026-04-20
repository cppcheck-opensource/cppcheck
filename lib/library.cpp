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

#include "library.h"

#include "astutils.h"
#include "errortypes.h"
#include "mathlib.h"
#include "path.h"
#include "symboldatabase.h"
#include "token.h"
#include "tokenlist.h"
#include "utils.h"
#include "vfvalue.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>

#include "xml.h"

struct Library::LibraryData
{
    struct Platform {
        const PlatformType *platform_type(const std::string &name) const {
            const auto it = mPlatformTypes.find(name);
            return (it != mPlatformTypes.end()) ? &(it->second) : nullptr;
        }
        std::unordered_map<std::string, PlatformType> mPlatformTypes;
    };

    class ExportedFunctions {
    public:
        void addPrefix(std::string prefix) {
            mPrefixes.insert(std::move(prefix));
        }
        void addSuffix(std::string suffix) {
            mSuffixes.insert(std::move(suffix));
        }
        bool isPrefix(const std::string& prefix) const {
            return (mPrefixes.find(prefix) != mPrefixes.end());
        }
        bool isSuffix(const std::string& suffix) const {
            return (mSuffixes.find(suffix) != mSuffixes.end());
        }

    private:
        std::set<std::string> mPrefixes;
        std::set<std::string> mSuffixes;
    };

    class CodeBlock {
    public:
        CodeBlock() = default;

        void setStart(const char* s) {
            mStart = s;
        }
        void setEnd(const char* e) {
            mEnd = e;
        }
        void setOffset(const int o) {
            mOffset = o;
        }
        void addBlock(const char* blockName) {
            mBlocks.insert(blockName);
        }
        const std::string& start() const {
            return mStart;
        }
        const std::string& end() const {
            return mEnd;
        }
        int offset() const {
            return mOffset;
        }
        bool isBlock(const std::string& blockName) const {
            return mBlocks.find(blockName) != mBlocks.end();
        }

    private:
        std::string mStart;
        std::string mEnd;
        int mOffset{};
        std::set<std::string> mBlocks;
    };

    enum class FalseTrueMaybe : std::uint8_t { False, True, Maybe };

    std::map<std::string, WarnInfo> mFunctionwarn;
    std::set<std::string> mDefines;

    std::unordered_map<std::string, Container> mContainers;
    std::unordered_map<std::string, Function> mFunctions;
    std::unordered_map<std::string, SmartPointer> mSmartPointers;

    int mAllocId{};
    std::set<std::string> mFiles;
    std::map<std::string, AllocFunc> mAlloc; // allocation functions
    std::map<std::string, AllocFunc> mDealloc; // deallocation functions
    std::map<std::string, AllocFunc> mRealloc; // reallocation functions
    std::unordered_map<std::string, FalseTrueMaybe> mNoReturn; // is function noreturn?
    std::unordered_map<std::string, std::string> mReturnValue;
    std::unordered_map<std::string, std::string> mReturnValueType;
    std::unordered_map<std::string, int> mReturnValueContainer;
    std::map<std::string, std::vector<MathLib::bigint>> mUnknownReturnValues;
    std::map<std::string, bool> mReportErrors;
    std::map<std::string, bool> mProcessAfterCode;
    std::set<std::string> mMarkupExtensions; // file extensions of markup files
    std::map<std::string, std::set<std::string>> mKeywords;  // keywords for code in the library
    std::unordered_map<std::string, CodeBlock> mExecutableBlocks; // keywords for blocks of executable code
    std::map<std::string, ExportedFunctions> mExporters; // keywords that export variables/functions to libraries (meta-code/macros)
    std::map<std::string, std::set<std::string>> mImporters;  // keywords that import variables/functions
    std::map<std::string, int> mReflection; // invocation of reflection
    std::unordered_map<std::string, struct PodType> mPodTypes; // pod types
    std::map<std::string, PlatformType> mPlatformTypes; // platform independent typedefs
    std::unordered_map<std::string, Platform> mPlatforms; // platform dependent typedefs
    std::map<std::pair<std::string,std::string>, TypeCheck> mTypeChecks;
    std::unordered_map<std::string, NonOverlappingData> mNonOverlappingData;
    std::unordered_set<std::string> mEntrypoints;
};

const std::string Library::mEmptyString;

Library::Library()
    : mData(new LibraryData())
{}

Library::~Library() = default;

Library::Library(const Library& other)
    : mData(new LibraryData(*other.mData))
{}

Library& Library::operator=(const Library& other) &
{
    if (this == &other)
        return *this;

    mData.reset(new LibraryData(*other.mData));
    return *this;
}

static std::vector<std::string> getnames(const char *names)
{
    std::vector<std::string> ret;
    while (const char *p = std::strchr(names,',')) {
        ret.emplace_back(names, p-names);
        names = p + 1;
    }
    ret.emplace_back(names);
    return ret;
}

static void gettokenlistfromvalid(const std::string& valid, TokenList& tokenList)
{
    const std::string str(valid + ',');
    tokenList.createTokensFromBuffer(str.data(), str.size()); // TODO: check result?
    for (Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok,"- %num%")) {
            tok->str("-" + tok->strAt(1));
            tok->deleteNext();
        }
    }
}

Library::Error Library::load(const char exename[], const char path[], bool debug)
{
    if (std::strchr(path,',') != nullptr) {
        throw std::runtime_error("handling of multiple libraries not supported");
    }

    const bool is_abs_path = Path::isAbsolute(path);
    const bool is_rel_path = Path::isRelative(path);

    std::string fullfilename(path);

    // TODO: what if the extension is not .cfg?
    // only append extension when we provide the library name and not a path
    if (!is_abs_path && !is_rel_path && Path::getFilenameExtension(fullfilename).empty())
        fullfilename += ".cfg";

    std::string absolute_path;
    // open file..
    tinyxml2::XMLDocument doc;
    if (debug)
        std::cout << "looking for library '" + fullfilename + "'" << std::endl;
    tinyxml2::XMLError error = xml_LoadFile(doc, fullfilename.c_str());
    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
        // only perform further lookups when the given path was not absolute
        if (!is_abs_path)
        {
            std::list<std::string> cfgfolders;
    #ifdef FILESDIR
            cfgfolders.emplace_back(FILESDIR "/cfg");
    #endif
            if (exename) {
                std::string exepath(Path::fromNativeSeparators(Path::getPathFromFilename(Path::getCurrentExecutablePath(exename))));
                cfgfolders.push_back(exepath + "cfg");
                cfgfolders.push_back(std::move(exepath));
            }

            while (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND && !cfgfolders.empty()) {
                const std::string cfgfolder(cfgfolders.back());
                cfgfolders.pop_back();
                const char *sep = (!cfgfolder.empty() && endsWith(cfgfolder,'/') ? "" : "/");
                const std::string filename(cfgfolder + sep + fullfilename);
                if (debug)
                    std::cout << "looking for library '" + std::string(filename) + "'" << std::endl;
                error = xml_LoadFile(doc, filename.c_str());
                if (error != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
                    absolute_path = Path::getAbsoluteFilePath(filename);
            }
        }
    } else
        absolute_path = Path::getAbsoluteFilePath(fullfilename);

    if (error == tinyxml2::XML_SUCCESS) {
        if (mData->mFiles.find(absolute_path) == mData->mFiles.end()) {
            Error err = load(doc);
            if (err.errorcode == ErrorCode::OK)
                mData->mFiles.insert(std::move(absolute_path));
            return err;
        }

        return Error(ErrorCode::OK); // ignore duplicates
    }

    if (debug)
        std::cout << "library not found: '" + std::string(path) + "'" << std::endl;

    if (error == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
        return Error(ErrorCode::FILE_NOT_FOUND);

    doc.PrintError(); // TODO: do not print stray messages
    return Error(ErrorCode::BAD_XML);
}

Library::Container::Yield Library::Container::yieldFrom(const std::string& yieldName)
{
    if (yieldName == "at_index")
        return Container::Yield::AT_INDEX;
    if (yieldName == "item")
        return Container::Yield::ITEM;
    if (yieldName == "buffer")
        return Container::Yield::BUFFER;
    if (yieldName == "buffer-nt")
        return Container::Yield::BUFFER_NT;
    if (yieldName == "start-iterator")
        return Container::Yield::START_ITERATOR;
    if (yieldName == "end-iterator")
        return Container::Yield::END_ITERATOR;
    if (yieldName == "iterator")
        return Container::Yield::ITERATOR;
    if (yieldName == "size")
        return Container::Yield::SIZE;
    if (yieldName == "empty")
        return Container::Yield::EMPTY;
    return Container::Yield::NO_YIELD;
}
Library::Container::Action Library::Container::actionFrom(const std::string& actionName)
{
    if (actionName == "resize")
        return Container::Action::RESIZE;
    if (actionName == "clear")
        return Container::Action::CLEAR;
    if (actionName == "push")
        return Container::Action::PUSH;
    if (actionName == "pop")
        return Container::Action::POP;
    if (actionName == "find")
        return Container::Action::FIND;
    if (actionName == "find-const")
        return Container::Action::FIND_CONST;
    if (actionName == "insert")
        return Container::Action::INSERT;
    if (actionName == "erase")
        return Container::Action::ERASE;
    if (actionName == "append")
        return Container::Action::APPEND;
    if (actionName == "change-content")
        return Container::Action::CHANGE_CONTENT;
    if (actionName == "change-internal")
        return Container::Action::CHANGE_INTERNAL;
    if (actionName == "change")
        return Container::Action::CHANGE;
    return Container::Action::NO_ACTION;
}

std::string Library::Container::toString(Library::Container::Yield yield)
{
    switch (yield) {
    case Library::Container::Yield::AT_INDEX:
        return "at_index";
    case Library::Container::Yield::ITEM:
        return "item";
    case Library::Container::Yield::BUFFER:
        return "buffer";
    case Library::Container::Yield::BUFFER_NT:
        return "buffer-nt";
    case Library::Container::Yield::START_ITERATOR:
        return "start-iterator";
    case Library::Container::Yield::END_ITERATOR:
        return "end-iterator";
    case Library::Container::Yield::ITERATOR:
        return "iterator";
    case Library::Container::Yield::SIZE:
        return "size";
    case Library::Container::Yield::EMPTY:
        return "empty";
    case Library::Container::Yield::NO_YIELD:
        break;
    }
    return "";
}

std::string Library::Container::toString(Library::Container::Action action)
{
    switch (action) {
    case Library::Container::Action::RESIZE:
        return "resize";
    case Library::Container::Action::CLEAR:
        return "clear";
    case Library::Container::Action::PUSH:
        return "push";
    case Library::Container::Action::POP:
        return "pop";
    case Library::Container::Action::FIND:
        return "find";
    case Library::Container::Action::FIND_CONST:
        return "find-const";
    case Library::Container::Action::INSERT:
        return "insert";
    case Library::Container::Action::ERASE:
        return "erase";
    case Library::Container::Action::APPEND:
        return "append";
    case Library::Container::Action::CHANGE_CONTENT:
        return "change-content";
    case Library::Container::Action::CHANGE:
        return "change";
    case Library::Container::Action::CHANGE_INTERNAL:
        return "change-internal";
    case Library::Container::Action::NO_ACTION:
        break;
    }
    return "";
}

Library::Error Library::load(const tinyxml2::XMLDocument &doc)
{
    const tinyxml2::XMLElement * const rootnode = doc.FirstChildElement();

    if (rootnode == nullptr) {
        doc.PrintError();
        return Error(ErrorCode::BAD_XML);
    }

    if (strcmp(rootnode->Name(),"def") != 0)
        return Error(ErrorCode::UNSUPPORTED_FORMAT, rootnode->Name());

    const int format = rootnode->IntAttribute("format", 1); // Assume format version 1 if nothing else is specified (very old .cfg files had no 'format' attribute)

    if (format > 2 || format <= 0)
        return Error(ErrorCode::UNSUPPORTED_FORMAT);

    std::set<std::string> unknown_elements;

    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        const std::string nodename = node->Name();
        if (nodename == "memory" || nodename == "resource") {
            // get allocationId to use..
            int allocationId = 0;
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                if (strcmp(memorynode->Name(),"dealloc")==0) {
                    const auto names = getnames(memorynode->GetText());
                    for (const auto& n : names) {
                        const auto it = utils::as_const(mData->mDealloc).find(n);
                        if (it != mData->mDealloc.end()) {
                            allocationId = it->second.groupId;
                            break;
                        }
                    }
                    if (allocationId != 0)
                        break;
                }
            }
            if (allocationId == 0) {
                if (nodename == "memory") {
                    while (!ismemory(++mData->mAllocId)) {}
                }
                else {
                    while (!isresource(++mData->mAllocId)) {}
                }
                allocationId = mData->mAllocId;
            }

            // add alloc/dealloc/use functions..
            for (const tinyxml2::XMLElement *memorynode = node->FirstChildElement(); memorynode; memorynode = memorynode->NextSiblingElement()) {
                const std::string memorynodename = memorynode->Name();
                const auto names = getnames(memorynode->GetText());
                if (memorynodename == "alloc" || memorynodename == "realloc") {
                    AllocFunc temp;
                    temp.groupId = allocationId;

                    temp.noFail = memorynode->BoolAttribute("no-fail", false);
                    temp.initData = memorynode->BoolAttribute("init", true);
                    temp.arg = memorynode->IntAttribute("arg", -1);

                    const char *bufferSize = memorynode->Attribute("buffer-size");
                    if (!bufferSize)
                        temp.bufferSize = AllocFunc::BufferSize::none;
                    else {
                        if (std::strncmp(bufferSize, "malloc", 6) == 0)
                            temp.bufferSize = AllocFunc::BufferSize::malloc;
                        else if (std::strncmp(bufferSize, "calloc", 6) == 0)
                            temp.bufferSize = AllocFunc::BufferSize::calloc;
                        else if (std::strncmp(bufferSize, "strdup", 6) == 0)
                            temp.bufferSize = AllocFunc::BufferSize::strdup;
                        else
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, bufferSize);
                        temp.bufferSizeArg1 = 1;
                        temp.bufferSizeArg2 = 2;
                        if (bufferSize[6] == 0) {
                            // use default values
                        } else if (bufferSize[6] == ':' && bufferSize[7] >= '1' && bufferSize[7] <= '5') {
                            temp.bufferSizeArg1 = bufferSize[7] - '0';
                            if (bufferSize[8] == ',' && bufferSize[9] >= '1' && bufferSize[9] <= '5')
                                temp.bufferSizeArg2 = bufferSize[9] - '0';
                        } else
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, bufferSize);
                    }

                    if (memorynodename == "realloc")
                        temp.reallocArg = memorynode->IntAttribute("realloc-arg", 1);

                    auto& map = (memorynodename == "realloc") ? mData->mRealloc : mData->mAlloc;
                    for (const auto& n : names)
                        map[n] = temp;
                } else if (memorynodename == "dealloc") {
                    AllocFunc temp;
                    temp.groupId = allocationId;
                    temp.arg = memorynode->IntAttribute("arg", 1);
                    for (const auto& n : names)
                        mData->mDealloc[n] = temp;
                } else if (memorynodename == "use")
                    for (const auto& n : names)
                        mData->mFunctions[n].use = true;
                else
                    unknown_elements.insert(memorynodename);
            }
        }

        else if (nodename == "define") {
            const char *name = node->Attribute("name");
            if (name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            const char *value = node->Attribute("value");
            if (value == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "value");
            auto result = mData->mDefines.insert(std::string(name) + " " + value);
            if (!result.second)
                return Error(ErrorCode::DUPLICATE_DEFINE, name);
        }

        else if (nodename == "function") {
            const char *name = node->Attribute("name");
            if (name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            for (const std::string &s : getnames(name)) {
                const Error &err = loadFunction(node, s, unknown_elements);
                if (err.errorcode != ErrorCode::OK)
                    return err;
            }
        }

        else if (nodename == "reflection") {
            for (const tinyxml2::XMLElement *reflectionnode = node->FirstChildElement(); reflectionnode; reflectionnode = reflectionnode->NextSiblingElement()) {
                if (strcmp(reflectionnode->Name(), "call") != 0) {
                    unknown_elements.insert(reflectionnode->Name());
                    continue;
                }

                const char * const argString = reflectionnode->Attribute("arg");
                if (!argString)
                    return Error(ErrorCode::MISSING_ATTRIBUTE, "arg");

                mData->mReflection[reflectionnode->GetText()] = strToInt<int>(argString);
            }
        }

        else if (nodename == "markup") {
            const char * const extension = node->Attribute("ext");
            if (!extension)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "ext");
            mData->mMarkupExtensions.insert(extension);

            mData->mReportErrors[extension] = (node->Attribute("reporterrors", "true") != nullptr);
            mData->mProcessAfterCode[extension] = (node->Attribute("aftercode", "true") != nullptr);

            for (const tinyxml2::XMLElement *markupnode = node->FirstChildElement(); markupnode; markupnode = markupnode->NextSiblingElement()) {
                const std::string markupnodename = markupnode->Name();
                if (markupnodename == "keywords") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "keyword") == 0) {
                            const char* nodeName = librarynode->Attribute("name");
                            if (nodeName == nullptr)
                                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
                            mData->mKeywords[extension].insert(nodeName);
                        } else
                            unknown_elements.insert(librarynode->Name());
                    }
                }

                else if (markupnodename == "exported") {
                    for (const tinyxml2::XMLElement *exporter = markupnode->FirstChildElement(); exporter; exporter = exporter->NextSiblingElement()) {
                        if (strcmp(exporter->Name(), "exporter") != 0) {
                            unknown_elements.insert(exporter->Name());
                            continue;
                        }

                        const char * const prefix = exporter->Attribute("prefix");
                        if (!prefix)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "prefix");

                        for (const tinyxml2::XMLElement *e = exporter->FirstChildElement(); e; e = e->NextSiblingElement()) {
                            const std::string ename = e->Name();
                            if (ename == "prefix")
                                mData->mExporters[prefix].addPrefix(e->GetText());
                            else if (ename == "suffix")
                                mData->mExporters[prefix].addSuffix(e->GetText());
                            else
                                unknown_elements.insert(ename);
                        }
                    }
                }

                else if (markupnodename == "imported") {
                    for (const tinyxml2::XMLElement *librarynode = markupnode->FirstChildElement(); librarynode; librarynode = librarynode->NextSiblingElement()) {
                        if (strcmp(librarynode->Name(), "importer") == 0)
                            mData->mImporters[extension].insert(librarynode->GetText());
                        else
                            unknown_elements.insert(librarynode->Name());
                    }
                }

                else if (markupnodename == "codeblocks") {
                    for (const tinyxml2::XMLElement *blocknode = markupnode->FirstChildElement(); blocknode; blocknode = blocknode->NextSiblingElement()) {
                        const std::string blocknodename = blocknode->Name();
                        if (blocknodename == "block") {
                            const char * blockName = blocknode->Attribute("name");
                            if (blockName)
                                mData->mExecutableBlocks[extension].addBlock(blockName);
                        } else if (blocknodename == "structure") {
                            const char * start = blocknode->Attribute("start");
                            if (start)
                                mData->mExecutableBlocks[extension].setStart(start);
                            const char * end = blocknode->Attribute("end");
                            if (end)
                                mData->mExecutableBlocks[extension].setEnd(end);
                            const char * offset = blocknode->Attribute("offset");
                            if (offset) {
                                // cppcheck-suppress templateInstantiation - TODO: fix this - see #11631
                                mData->mExecutableBlocks[extension].setOffset(strToInt<int>(offset));
                            }
                        }

                        else
                            unknown_elements.insert(blocknodename);
                    }
                }

                else
                    unknown_elements.insert(markupnodename);
            }
        }

        else if (nodename == "container") {
            const char* const id = node->Attribute("id");
            if (!id)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "id");

            Container& container = mData->mContainers[id];

            const char* const inherits = node->Attribute("inherits");
            if (inherits) {
                const auto i = utils::as_const(mData->mContainers).find(inherits);
                if (i != mData->mContainers.end())
                    container = i->second; // Take values from parent and overwrite them if necessary
                else
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, inherits);
            }

            const char* const startPattern = node->Attribute("startPattern");
            if (startPattern) {
                container.startPattern = startPattern;
                container.startPattern2 = startPattern;
                if (!endsWith(container.startPattern, '<'))
                    container.startPattern2 += " !!::";
                container.startPatternHasStd = startsWith(container.startPattern2, "std :: ");
            }
            const char* const endPattern = node->Attribute("endPattern");
            if (endPattern)
                container.endPattern = endPattern;
            const char* const itEndPattern = node->Attribute("itEndPattern");
            if (itEndPattern)
                container.itEndPattern = itEndPattern;
            const char* const opLessAllowed = node->Attribute("opLessAllowed");
            if (opLessAllowed)
                container.opLessAllowed = strcmp(opLessAllowed, "true") == 0;
            const char* const hasInitializerListConstructor = node->Attribute("hasInitializerListConstructor");
            if (hasInitializerListConstructor)
                container.hasInitializerListConstructor = strcmp(hasInitializerListConstructor, "true") == 0;
            const char* const view = node->Attribute("view");
            if (view)
                container.view = strcmp(view, "true") == 0;

            for (const tinyxml2::XMLElement *containerNode = node->FirstChildElement(); containerNode; containerNode = containerNode->NextSiblingElement()) {
                const std::string containerNodeName = containerNode->Name();
                if (containerNodeName == "size" || containerNodeName == "access" || containerNodeName == "other") {
                    for (const tinyxml2::XMLElement *functionNode = containerNode->FirstChildElement(); functionNode; functionNode = functionNode->NextSiblingElement()) {
                        if (strcmp(functionNode->Name(), "function") != 0) {
                            unknown_elements.insert(functionNode->Name());
                            continue;
                        }

                        const char* const functionName = functionNode->Attribute("name");
                        if (!functionName)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "name");

                        const char* const action_ptr = functionNode->Attribute("action");
                        Container::Action action = Container::Action::NO_ACTION;
                        if (action_ptr) {
                            std::string actionName = action_ptr;
                            action = Container::actionFrom(actionName);
                            if (action == Container::Action::NO_ACTION)
                                return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, actionName);
                        }

                        const char* const yield_ptr = functionNode->Attribute("yields");
                        Container::Yield yield = Container::Yield::NO_YIELD;
                        if (yield_ptr) {
                            std::string yieldName = yield_ptr;
                            yield = Container::yieldFrom(yieldName);
                            if (yield == Container::Yield::NO_YIELD)
                                return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, yieldName);
                        }

                        const char* const returnType = functionNode->Attribute("returnType");
                        if (returnType)
                            container.functions[functionName].returnType = returnType;

                        container.functions[functionName].action = action;
                        container.functions[functionName].yield = yield;
                    }

                    if (containerNodeName == "size") {
                        const char* const templateArg = containerNode->Attribute("templateParameter");
                        if (templateArg)
                            container.size_templateArgNo = strToInt<int>(templateArg);
                    } else if (containerNodeName == "access") {
                        const char* const indexArg = containerNode->Attribute("indexOperator");
                        if (indexArg)
                            container.arrayLike_indexOp = strcmp(indexArg, "array-like") == 0;
                    }
                } else if (containerNodeName == "type") {
                    const char* const templateArg = containerNode->Attribute("templateParameter");
                    if (templateArg)
                        container.type_templateArgNo = strToInt<int>(templateArg);

                    const char* const string = containerNode->Attribute("string");
                    if (string)
                        container.stdStringLike = strcmp(string, "std-like") == 0;
                    const char* const associative = containerNode->Attribute("associative");
                    if (associative)
                        container.stdAssociativeLike = strcmp(associative, "std-like") == 0;
                    const char* const unstable = containerNode->Attribute("unstable");
                    if (unstable) {
                        std::string unstableType = unstable;
                        if (unstableType.find("erase") != std::string::npos)
                            container.unstableErase = true;
                        if (unstableType.find("insert") != std::string::npos)
                            container.unstableInsert = true;
                    }
                } else if (containerNodeName == "rangeItemRecordType") {
                    for (const tinyxml2::XMLElement* memberNode = node->FirstChildElement(); memberNode; memberNode = memberNode->NextSiblingElement()) {
                        const char *memberName = memberNode->Attribute("name");
                        const char *memberTemplateParameter = memberNode->Attribute("templateParameter");
                        Container::RangeItemRecordTypeItem member;
                        member.name = memberName ? memberName : "";
                        member.templateParameter = memberTemplateParameter ? strToInt<int>(memberTemplateParameter) : -1;
                        container.rangeItemRecordType.emplace_back(std::move(member));
                    }
                } else
                    unknown_elements.insert(containerNodeName);
            }
        }

        else if (nodename == "smart-pointer") {
            const char *className = node->Attribute("class-name");
            if (!className)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "class-name");
            SmartPointer& smartPointer = mData->mSmartPointers[className];
            smartPointer.name = className;
            for (const tinyxml2::XMLElement* smartPointerNode = node->FirstChildElement(); smartPointerNode;
                 smartPointerNode = smartPointerNode->NextSiblingElement()) {
                const std::string smartPointerNodeName = smartPointerNode->Name();
                if (smartPointerNodeName == "unique")
                    smartPointer.unique = true;
            }
        }

        else if (nodename == "type-checks") {
            for (const tinyxml2::XMLElement *checkNode = node->FirstChildElement(); checkNode; checkNode = checkNode->NextSiblingElement()) {
                const std::string &checkName = checkNode->Name();
                for (const tinyxml2::XMLElement *checkTypeNode = checkNode->FirstChildElement(); checkTypeNode; checkTypeNode = checkTypeNode->NextSiblingElement()) {
                    const std::string checkTypeName = checkTypeNode->Name();
                    const char *typeName = checkTypeNode->GetText();
                    if (!typeName)
                        continue;
                    if (checkTypeName == "check")
                        mData->mTypeChecks[std::pair<std::string,std::string>(checkName, typeName)] = TypeCheck::check;
                    else if (checkTypeName == "suppress")
                        mData->mTypeChecks[std::pair<std::string,std::string>(checkName, typeName)] = TypeCheck::suppress;
                    else if (checkTypeName == "checkFiniteLifetime")
                        mData->mTypeChecks[std::pair<std::string,std::string>(checkName, typeName)] = TypeCheck::checkFiniteLifetime;
                }
            }
        }

        else if (nodename == "podtype") {
            const char * const name = node->Attribute("name");
            if (!name)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            PodType podType;
            podType.stdtype = PodType::Type::NO;
            const char * const stdtype = node->Attribute("stdtype");
            if (stdtype) {
                if (std::strcmp(stdtype, "bool") == 0)
                    podType.stdtype = PodType::Type::BOOL;
                else if (std::strcmp(stdtype, "char") == 0)
                    podType.stdtype = PodType::Type::CHAR;
                else if (std::strcmp(stdtype, "short") == 0)
                    podType.stdtype = PodType::Type::SHORT;
                else if (std::strcmp(stdtype, "int") == 0)
                    podType.stdtype = PodType::Type::INT;
                else if (std::strcmp(stdtype, "long") == 0)
                    podType.stdtype = PodType::Type::LONG;
                else if (std::strcmp(stdtype, "long long") == 0)
                    podType.stdtype = PodType::Type::LONGLONG;
            }
            const char * const size = node->Attribute("size");
            if (size)
                podType.size = strToInt<unsigned int>(size);
            const char * const sign = node->Attribute("sign");
            if (sign)
                podType.sign = *sign;
            for (const std::string &s : getnames(name))
                mData->mPodTypes[s] = podType;
        }

        else if (nodename == "platformtype") {
            const char * const type_name = node->Attribute("name");
            if (type_name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            const char *value = node->Attribute("value");
            if (value == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "value");
            PlatformType type;
            type.mType = value;
            std::set<std::string> platform;
            for (const tinyxml2::XMLElement *typenode = node->FirstChildElement(); typenode; typenode = typenode->NextSiblingElement()) {
                const std::string typenodename = typenode->Name();
                if (typenodename == "platform") {
                    const char * const type_attribute = typenode->Attribute("type");
                    if (type_attribute == nullptr)
                        return Error(ErrorCode::MISSING_ATTRIBUTE, "type");
                    platform.insert(type_attribute);
                } else if (typenodename == "signed")
                    type.mSigned = true;
                else if (typenodename == "unsigned")
                    type.mUnsigned = true;
                else if (typenodename == "long")
                    type.mLong = true;
                else if (typenodename == "pointer")
                    type.mPointer= true;
                else if (typenodename == "ptr_ptr")
                    type.mPtrPtr = true;
                else if (typenodename == "const_ptr")
                    type.mConstPtr = true;
                else
                    unknown_elements.insert(typenodename);
            }
            if (platform.empty()) {
                const PlatformType * const type_ptr = platform_type(type_name, "");
                if (type_ptr) {
                    if (*type_ptr == type)
                        return Error(ErrorCode::DUPLICATE_PLATFORM_TYPE, type_name);
                    return Error(ErrorCode::PLATFORM_TYPE_REDEFINED, type_name);
                }
                mData->mPlatformTypes[type_name] = std::move(type);
            } else {
                for (const std::string &p : platform) {
                    const PlatformType * const type_ptr = platform_type(type_name, p);
                    if (type_ptr) {
                        if (*type_ptr == type)
                            return Error(ErrorCode::DUPLICATE_PLATFORM_TYPE, type_name);
                        return Error(ErrorCode::PLATFORM_TYPE_REDEFINED, type_name);
                    }
                    mData->mPlatforms[p].mPlatformTypes[type_name] = type;
                }
            }
        }

        else if (nodename == "entrypoint") {
            const char * const type_name = node->Attribute("name");
            if (type_name == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "name");
            mData->mEntrypoints.emplace(type_name);
        }

        else
            unknown_elements.insert(nodename);
    }
    if (!unknown_elements.empty()) {
        std::string str;
        for (auto i = unknown_elements.cbegin(); i != unknown_elements.cend();) {
            str += *i;
            if (++i != unknown_elements.end())
                str += ", ";
        }
        return Error(ErrorCode::UNKNOWN_ELEMENT, str);
    }
    return Error(ErrorCode::OK);
}

Library::Error Library::loadFunction(const tinyxml2::XMLElement * const node, const std::string &name, std::set<std::string> &unknown_elements)
{
    if (name.empty())
        return Error(ErrorCode::OK);

    // TODO: write debug warning if we modify an existing entry
    Function& func = mData->mFunctions[name];

    for (const tinyxml2::XMLElement *functionnode = node->FirstChildElement(); functionnode; functionnode = functionnode->NextSiblingElement()) {
        const std::string functionnodename = functionnode->Name();
        if (functionnodename == "noreturn") {
            const char * const text = functionnode->GetText();
            if (strcmp(text, "false") == 0)
                mData->mNoReturn[name] = LibraryData::FalseTrueMaybe::False;
            else if (strcmp(text, "maybe") == 0)
                mData->mNoReturn[name] = LibraryData::FalseTrueMaybe::Maybe;
            else
                mData->mNoReturn[name] = LibraryData::FalseTrueMaybe::True; // Safe
        } else if (functionnodename == "pure")
            func.ispure = true;
        else if (functionnodename == "const") {
            func.ispure = true;
            func.isconst = true; // a constant function is pure
        } else if (functionnodename == "leak-ignore")
            func.leakignore = true;
        else if (functionnodename == "not-overlapping-data") {
            NonOverlappingData nonOverlappingData;
            nonOverlappingData.ptr1Arg = functionnode->IntAttribute("ptr1-arg", -1);
            nonOverlappingData.ptr2Arg = functionnode->IntAttribute("ptr2-arg", -1);
            nonOverlappingData.sizeArg = functionnode->IntAttribute("size-arg", -1);
            nonOverlappingData.strlenArg = functionnode->IntAttribute("strlen-arg", -1);
            nonOverlappingData.countArg = functionnode->IntAttribute("count-arg", -1);
            mData->mNonOverlappingData[name] = nonOverlappingData;
        } else if (functionnodename == "use-retval") {
            func.useretval = Library::UseRetValType::DEFAULT;
            if (const char *type = functionnode->Attribute("type"))
                if (std::strcmp(type, "error-code") == 0)
                    func.useretval = Library::UseRetValType::ERROR_CODE;
        } else if (functionnodename == "returnValue") {
            if (const char *expr = functionnode->GetText())
                mData->mReturnValue[name] = expr;
            if (const char *type = functionnode->Attribute("type"))
                mData->mReturnValueType[name] = type;
            if (const char *container = functionnode->Attribute("container"))
                mData->mReturnValueContainer[name] = strToInt<int>(container);
            // cppcheck-suppress shadowFunction - TODO: fix this
            if (const char *unknownReturnValues = functionnode->Attribute("unknownValues")) {
                if (std::strcmp(unknownReturnValues, "all") == 0) {
                    std::vector<MathLib::bigint> values{LLONG_MIN, LLONG_MAX};
                    mData->mUnknownReturnValues[name] = std::move(values);
                }
            }
        } else if (functionnodename == "arg") {
            const char* argNrString = functionnode->Attribute("nr");
            if (!argNrString)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "nr");
            const bool bAnyArg = strcmp(argNrString, "any") == 0;
            const bool bVariadicArg = strcmp(argNrString, "variadic") == 0;
            const int nr = (bAnyArg || bVariadicArg) ? -1 : strToInt<int>(argNrString);
            ArgumentChecks &ac = func.argumentChecks[nr];
            ac.optional  = functionnode->Attribute("default") != nullptr;
            ac.variadic = bVariadicArg;
            const char * const argDirection = functionnode->Attribute("direction");
            if (argDirection) {
                const size_t argDirLen = strlen(argDirection);
                ArgumentChecks::Direction dir = ArgumentChecks::Direction::DIR_UNKNOWN;
                if (!strncmp(argDirection, "in", argDirLen)) {
                    dir = ArgumentChecks::Direction::DIR_IN;
                } else if (!strncmp(argDirection, "out", argDirLen)) {
                    dir = ArgumentChecks::Direction::DIR_OUT;
                } else if (!strncmp(argDirection, "inout", argDirLen)) {
                    dir = ArgumentChecks::Direction::DIR_INOUT;
                }
                if (const char* const argIndirect = functionnode->Attribute("indirect")) {
                    const int indirect = strToInt<int>(argIndirect);
                    ac.direction[indirect] = dir; // TODO: handle multiple directions/indirect levels
                }
                else
                    ac.direction.fill(dir);
            }
            for (const tinyxml2::XMLElement *argnode = functionnode->FirstChildElement(); argnode; argnode = argnode->NextSiblingElement()) {
                const std::string argnodename = argnode->Name();
                int indirect = 0;
                const char * const indirectStr = argnode->Attribute("indirect");
                if (indirectStr)
                    indirect = strToInt<int>(indirectStr);
                if (argnodename == "not-bool")
                    ac.notbool = true;
                else if (argnodename == "not-null")
                    ac.notnull = true;
                else if (argnodename == "not-uninit")
                    ac.notuninit = indirect;
                else if (argnodename == "formatstr")
                    ac.formatstr = true;
                else if (argnodename == "strz")
                    ac.strz = true;
                else if (argnodename == "valid") {
                    // Validate the validation expression
                    const char *p = argnode->GetText();
                    if (!isCompliantValidationExpression(p))
                        return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, (!p ? "\"\"" : p));
                    // Set validation expression
                    ac.valid = p;
                }
                else if (argnodename == "minsize") {
                    const char *typeattr = argnode->Attribute("type");
                    if (!typeattr)
                        return Error(ErrorCode::MISSING_ATTRIBUTE, "type");

                    ArgumentChecks::MinSize::Type type;
                    if (strcmp(typeattr,"strlen")==0)
                        type = ArgumentChecks::MinSize::Type::STRLEN;
                    else if (strcmp(typeattr,"argvalue")==0)
                        type = ArgumentChecks::MinSize::Type::ARGVALUE;
                    else if (strcmp(typeattr,"sizeof")==0)
                        type = ArgumentChecks::MinSize::Type::SIZEOF;
                    else if (strcmp(typeattr,"mul")==0)
                        type = ArgumentChecks::MinSize::Type::MUL;
                    else if (strcmp(typeattr,"value")==0)
                        type = ArgumentChecks::MinSize::Type::VALUE;
                    else
                        return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, typeattr);

                    if (type == ArgumentChecks::MinSize::Type::VALUE) {
                        const char *valueattr = argnode->Attribute("value");
                        if (!valueattr)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "value");
                        long long minsizevalue = 0;
                        try {
                            minsizevalue = strToInt<long long>(valueattr);
                        } catch (const std::runtime_error&) {
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, valueattr);
                        }
                        if (minsizevalue <= 0)
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, valueattr);
                        ac.minsizes.emplace_back(type, 0);
                        ac.minsizes.back().value = minsizevalue;
                    } else {
                        const char *argattr = argnode->Attribute("arg");
                        if (!argattr)
                            return Error(ErrorCode::MISSING_ATTRIBUTE, "arg");
                        if (strlen(argattr) != 1 || argattr[0]<'0' || argattr[0]> '9')
                            return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, argattr);

                        ac.minsizes.reserve(type == ArgumentChecks::MinSize::Type::MUL ? 2 : 1);
                        ac.minsizes.emplace_back(type, argattr[0] - '0');
                        if (type == ArgumentChecks::MinSize::Type::MUL) {
                            const char *arg2attr = argnode->Attribute("arg2");
                            if (!arg2attr)
                                return Error(ErrorCode::MISSING_ATTRIBUTE, "arg2");
                            if (strlen(arg2attr) != 1 || arg2attr[0]<'0' || arg2attr[0]> '9')
                                return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, arg2attr);
                            ac.minsizes.back().arg2 = arg2attr[0] - '0';
                        }
                    }
                    const char* baseTypeAttr = argnode->Attribute("baseType"); // used by VALUE, ARGVALUE
                    if (baseTypeAttr)
                        ac.minsizes.back().baseType = baseTypeAttr;
                }

                else if (argnodename == "iterator") {
                    ac.iteratorInfo.it = true;
                    const char* str = argnode->Attribute("type");
                    ac.iteratorInfo.first = (str && std::strcmp(str, "first") == 0);
                    ac.iteratorInfo.last = (str && std::strcmp(str, "last") == 0);
                    ac.iteratorInfo.container = argnode->IntAttribute("container", 0);
                }

                else
                    unknown_elements.insert(argnodename);
            }
            if (ac.notuninit == 0)
                ac.notuninit = ac.notnull ? 1 : 0;
        } else if (functionnodename == "ignorefunction") {
            func.ignore = true;
        } else if (functionnodename == "formatstr") {
            func.formatstr = true;
            const tinyxml2::XMLAttribute* scan = functionnode->FindAttribute("scan");
            const tinyxml2::XMLAttribute* secure = functionnode->FindAttribute("secure");
            func.formatstr_scan = scan && scan->BoolValue();
            func.formatstr_secure = secure && secure->BoolValue();
        } else if (functionnodename == "warn") {
            WarnInfo wi;
            const char* const severity = functionnode->Attribute("severity");
            if (severity == nullptr)
                return Error(ErrorCode::MISSING_ATTRIBUTE, "severity");
            wi.severity = severityFromString(severity);

            const char* const cstd = functionnode->Attribute("cstd");
            if (cstd) {
                if (!wi.standards.setC(cstd))
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, cstd);
            } else
                wi.standards.c = Standards::C89;

            const char* const cppstd = functionnode->Attribute("cppstd");
            if (cppstd) {
                if (!wi.standards.setCPP(cppstd))
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, cppstd);
            } else
                wi.standards.cpp = Standards::CPP03;

            const char* const reason = functionnode->Attribute("reason");
            const char* const alternatives = functionnode->Attribute("alternatives");
            if (reason && alternatives) {
                // Construct message
                wi.message = std::string(reason) + " function '" + name + "' called. It is recommended to use ";
                std::vector<std::string> alt = getnames(alternatives);
                for (std::size_t i = 0; i < alt.size(); ++i) {
                    wi.message += "'" + alt[i] + "'";
                    if (i == alt.size() - 1)
                        wi.message += " instead.";
                    else if (i == alt.size() - 2)
                        wi.message += " or ";
                    else
                        wi.message += ", ";
                }
            } else {
                const char * const message = functionnode->GetText();
                if (!message)
                    return Error(ErrorCode::MISSING_ATTRIBUTE, "\"reason\" and \"alternatives\" or some text.");

                wi.message = message;
            }

            mData->mFunctionwarn[name] = std::move(wi);
        } else if (functionnodename == "container") {
            const char* const action_ptr = functionnode->Attribute("action");
            Container::Action action = Container::Action::NO_ACTION;
            if (action_ptr) {
                std::string actionName = action_ptr;
                action = Container::actionFrom(actionName);
                if (action == Container::Action::NO_ACTION)
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, actionName);
            }
            func.containerAction = action;

            const char* const yield_ptr = functionnode->Attribute("yields");
            Container::Yield yield = Container::Yield::NO_YIELD;
            if (yield_ptr) {
                std::string yieldName = yield_ptr;
                yield = Container::yieldFrom(yieldName);
                if (yield == Container::Yield::NO_YIELD)
                    return Error(ErrorCode::BAD_ATTRIBUTE_VALUE, yieldName);
            }
            func.containerYield = yield;

            const char* const returnType = functionnode->Attribute("returnType");
            if (returnType)
                func.returnType = returnType;
        } else
            unknown_elements.insert(functionnodename);
    }
    return Error(ErrorCode::OK);
}

bool Library::isIntArgValid(const Token *ftok, int argnr, const MathLib::bigint argvalue, const Settings& settings) const
{
    const ArgumentChecks *ac = getarg(ftok, argnr);
    if (!ac || ac->valid.empty())
        return true;
    if (ac->valid.find('.') != std::string::npos)
        return isFloatArgValid(ftok, argnr, static_cast<double>(argvalue), settings);
    TokenList tokenList(settings, ftok->isCpp() ? Standards::Language::CPP : Standards::Language::C);
    gettokenlistfromvalid(ac->valid, tokenList);
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (tok->isNumber() && argvalue == MathLib::toBigNumber(tok))
            return true;
        if (Token::Match(tok, "%num% : %num%") && argvalue >= MathLib::toBigNumber(tok) && argvalue <= MathLib::toBigNumber(tok->tokAt(2)))
            return true;
        if (Token::Match(tok, "%num% : ,") && argvalue >= MathLib::toBigNumber(tok))
            return true;
        if ((!tok->previous() || tok->strAt(-1) == ",") && Token::Match(tok,": %num%") && argvalue <= MathLib::toBigNumber(tok->tokAt(1)))
            return true;
    }
    return false;
}

bool Library::isFloatArgValid(const Token *ftok, int argnr, double argvalue, const Settings& settings) const
{
    const ArgumentChecks *ac = getarg(ftok, argnr);
    if (!ac || ac->valid.empty())
        return true;
    TokenList tokenList(settings, ftok->isCpp() ? Standards::Language::CPP : Standards::Language::C);
    gettokenlistfromvalid(ac->valid, tokenList);
    for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
        if (Token::Match(tok, "%num% : %num%") && argvalue >= MathLib::toDoubleNumber(tok) && argvalue <= MathLib::toDoubleNumber(tok->tokAt(2)))
            return true;
        if (Token::Match(tok, "%num% : ,") && argvalue >= MathLib::toDoubleNumber(tok))
            return true;
        if ((!tok->previous() || tok->strAt(-1) == ",") && Token::Match(tok,": %num%") && argvalue <= MathLib::toDoubleNumber(tok->tokAt(1)))
            return true;
        if (Token::Match(tok, "%num%") && MathLib::isFloat(tok->str()) && MathLib::isEqual(tok->str(), MathLib::toString(argvalue)))
            return true;
        if (Token::Match(tok, "! %num%") && MathLib::isFloat(tok->strAt(1)))
            return MathLib::isNotEqual(tok->strAt(1), MathLib::toString(argvalue));
    }
    return false;
}

std::string Library::getFunctionName(const Token *ftok, bool &error) const
{
    if (!ftok) {
        error = true;
        return "";
    }
    if (ftok->isName()) {
        if (Token::simpleMatch(ftok->astParent(), "::"))
            return ftok->str();
        for (const Scope *scope = ftok->scope(); scope; scope = scope->nestedIn) {
            if (!scope->isClassOrStruct())
                continue;
            const std::vector<Type::BaseInfo> &derivedFrom = scope->definedType->derivedFrom;
            for (const Type::BaseInfo & baseInfo : derivedFrom) {
                std::string name;
                const Token* tok = baseInfo.nameTok; // baseInfo.name still contains template parameters, but is missing namespaces
                if (tok->str() == "::")
                    tok = tok->next();
                while (Token::Match(tok, "%name%|::")) {
                    name += tok->str();
                    tok = tok->next();
                }
                name += "::" + ftok->str();
                if (mData->mFunctions.find(name) != mData->mFunctions.end() && matchArguments(ftok, name))
                    return name;
            }
        }
        return ftok->str();
    }
    if (ftok->str() == "::") {
        if (!ftok->astOperand2())
            return getFunctionName(ftok->astOperand1(), error);
        return getFunctionName(ftok->astOperand1(),error) + "::" + getFunctionName(ftok->astOperand2(),error);
    }
    if (ftok->str() == "." && ftok->astOperand1()) {
        const std::string type = astCanonicalType(ftok->astOperand1(), ftok->originalName() == "->");
        if (type.empty()) {
            error = true;
            return "";
        }

        return type + "::" + getFunctionName(ftok->astOperand2(),error);
    }
    error = true;
    return "";
}

std::string Library::getFunctionName(const Token *ftok) const
{
    if (!Token::Match(ftok, "%name% )| (") && (ftok->strAt(-1) != "&" || ftok->previous()->astOperand2()))
        return "";

    // Lookup function name using AST..
    if (ftok->astParent()) {
        bool error = false;
        const Token * tok = ftok->astParent()->isUnaryOp("&") ? ftok->astParent()->astOperand1() : ftok->next()->astOperand1();
        std::string ret = getFunctionName(tok, error);
        if (error)
            return {};
        if (startsWith(ret, "::"))
            ret.erase(0, 2);
        return ret;
    }

    // Lookup function name without using AST..
    if (Token::simpleMatch(ftok->previous(), "."))
        return "";
    if (!Token::Match(ftok->tokAt(-2), "%name% ::"))
        return ftok->str();
    std::string ret(ftok->str());
    ftok = ftok->tokAt(-2);
    while (Token::Match(ftok, "%name% ::")) {
        ret = ftok->str() + "::" + ret;
        ftok = ftok->tokAt(-2);
    }
    return ret;
}

bool Library::isnullargbad(const Token *ftok, int argnr) const
{
    const ArgumentChecks *arg = getarg(ftok, argnr);
    if (!arg) {
        // scan format string argument should not be null
        const std::string funcname = getFunctionName(ftok);
        const auto it = utils::as_const(mData->mFunctions).find(funcname);
        if (it != mData->mFunctions.cend() && it->second.formatstr && it->second.formatstr_scan)
            return true;
    }
    return arg && arg->notnull;
}

bool Library::isuninitargbad(const Token *ftok, int argnr, int indirect, bool *hasIndirect) const
{
    const ArgumentChecks *arg = getarg(ftok, argnr);
    if (!arg) {
        // non-scan format string argument should not be uninitialized
        const std::string funcname = getFunctionName(ftok);
        const auto it = utils::as_const(mData->mFunctions).find(funcname);
        if (it != mData->mFunctions.cend() && it->second.formatstr && !it->second.formatstr_scan)
            return true;
    }
    if (hasIndirect && arg && arg->notuninit >= 1)
        *hasIndirect = true;
    return arg && arg->notuninit >= indirect;
}


/** get allocation info for function */
const Library::AllocFunc* Library::getAllocFuncInfo(const Token *tok) const
{
    while (Token::simpleMatch(tok, "::"))
        tok = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
    if (!tok)
        return nullptr;
    const std::string funcname = getFunctionName(tok);
    return isNotLibraryFunction(tok) && mData->mFunctions.find(funcname) != mData->mFunctions.end() ? nullptr : getAllocDealloc(mData->mAlloc, funcname);
}

/** get deallocation info for function */
const Library::AllocFunc* Library::getDeallocFuncInfo(const Token *tok) const
{
    while (Token::simpleMatch(tok, "::"))
        tok = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
    if (!tok)
        return nullptr;
    const std::string funcname = getFunctionName(tok);
    return isNotLibraryFunction(tok) && mData->mFunctions.find(funcname) != mData->mFunctions.end() ? nullptr : getAllocDealloc(mData->mDealloc, funcname);
}

/** get reallocation info for function */
const Library::AllocFunc* Library::getReallocFuncInfo(const Token *tok) const
{
    while (Token::simpleMatch(tok, "::"))
        tok = tok->astOperand2() ? tok->astOperand2() : tok->astOperand1();
    if (!tok)
        return nullptr;
    const std::string funcname = getFunctionName(tok);
    return isNotLibraryFunction(tok) && mData->mFunctions.find(funcname) != mData->mFunctions.end() ? nullptr : getAllocDealloc(mData->mRealloc, funcname);
}

/** get allocation id for function */
int Library::getAllocId(const Token *tok, int arg) const
{
    const Library::AllocFunc* af = getAllocFuncInfo(tok);
    return (af && af->arg == arg) ? af->groupId : 0;
}

/** get deallocation id for function */
int Library::getDeallocId(const Token *tok, int arg) const
{
    const Library::AllocFunc* af = getDeallocFuncInfo(tok);
    return (af && af->arg == arg) ? af->groupId : 0;
}

/** get reallocation id for function */
int Library::getReallocId(const Token *tok, int arg) const
{
    const Library::AllocFunc* af = getReallocFuncInfo(tok);
    return (af && af->arg == arg) ? af->groupId : 0;
}


const Library::ArgumentChecks * Library::getarg(const Token *ftok, int argnr) const
{
    const Function* func = nullptr;
    if (isNotLibraryFunction(ftok, &func))
        return nullptr;
    const auto it2 = func->argumentChecks.find(argnr);
    if (it2 != func->argumentChecks.cend())
        return &it2->second;
    const auto it3 = func->argumentChecks.find(-1);
    if (it3 != func->argumentChecks.cend())
        return &it3->second;
    return nullptr;
}

bool Library::isScopeNoReturn(const Token *end, std::string *unknownFunc) const
{
    if (unknownFunc)
        unknownFunc->clear();

    if (Token::Match(end->tokAt(-2), "!!{ ; }")) {
        const Token *lastTop = end->tokAt(-2)->astTop();
        if (Token::simpleMatch(lastTop, "<<") &&
            Token::simpleMatch(lastTop->astOperand1(), "(") &&
            Token::Match(lastTop->astOperand1()->previous(), "%name% ("))
            return isnoreturn(lastTop->astOperand1()->previous());
    }

    if (!Token::simpleMatch(end->tokAt(-2), ") ; }"))
        return false;

    const Token *funcname = end->linkAt(-2)->previous();
    if (funcname->isCpp() && funcname->astTop()->str() == "throw")
        return true;
    const Token *start = funcname;
    if (Token::Match(funcname->tokAt(-3),"( * %name% )")) {
        funcname = funcname->previous();
        start = funcname->tokAt(-3);
    } else if (funcname->isName()) {
        while (Token::Match(start, "%name%|.|::"))
            start = start->previous();
    } else {
        return false;
    }
    if (Token::Match(start,"[;{}]") && Token::Match(funcname, "%name% )| (")) {
        if (funcname->isKeyword())
            return false;
        if (funcname->str() == "exit")
            return true;
        if (!isnotnoreturn(funcname)) {
            if (unknownFunc && !isnoreturn(funcname))
                *unknownFunc = funcname->str();
            return true;
        }
    }
    return false;
}

// cppcheck-suppress unusedFunction - used in tests only
const std::unordered_map<std::string, Library::Container>& Library::containers() const
{
    return mData->mContainers;
}

const Library::Container* Library::detectContainerInternal(const Token* const typeStart, DetectContainer detect, bool* isIterator, bool withoutStd) const
{
    if (!typeStart)
        return nullptr;
    if (typeStart->isKeyword())
        return nullptr;
    const Token* firstLinkedTok = nullptr;
    for (const Token* tok = typeStart; tok && !tok->varId(); tok = tok->next()) {
        if (!tok->link())
            continue;

        firstLinkedTok = tok;
        break;
    }

    const bool hasScope = typeStart->strAt(1) == "::";
    const bool bailIfHasStd = !withoutStd && !hasScope;

    for (const std::pair<const std::string, Library::Container> & c : mData->mContainers) {
        const Container& container = c.second;
        if (container.startPattern.empty())
            continue;

        if (bailIfHasStd && container.startPatternHasStd)
            continue;

        const int offset = (withoutStd && container.startPatternHasStd) ? 7 : 0;

        // If endPattern is undefined, it will always match, but itEndPattern has to be defined.
        if (detect != IteratorOnly && container.endPattern.empty()) {
            if (!Token::Match(typeStart, container.startPattern2.c_str() + offset))
                continue;

            if (isIterator)
                *isIterator = false;
            return &container;
        }

        if (!firstLinkedTok)
            continue;

        const bool matchedStartPattern = Token::Match(typeStart, container.startPattern2.c_str() + offset);
        if (!matchedStartPattern)
            continue;

        if (detect != ContainerOnly && Token::Match(firstLinkedTok->link(), container.itEndPattern.c_str())) {
            if (isIterator)
                *isIterator = true;
            return &container;
        }
        if (detect != IteratorOnly && Token::Match(firstLinkedTok->link(), container.endPattern.c_str())) {
            if (isIterator)
                *isIterator = false;
            return &container;
        }
    }
    return nullptr;
}

const Library::Container* Library::detectContainer(const Token* typeStart) const
{
    return detectContainerInternal(typeStart, ContainerOnly);
}

const Library::Container* Library::detectIterator(const Token* typeStart) const
{
    return detectContainerInternal(typeStart, IteratorOnly);
}

const Library::Container* Library::detectContainerOrIterator(const Token* typeStart, bool* isIterator, bool withoutStd) const
{
    bool res;
    const Library::Container* c = detectContainerInternal(typeStart, Both, &res, withoutStd);
    if (c && isIterator)
        *isIterator = res;
    return c;
}

bool Library::isContainerYield(const Token * const cond, Library::Container::Yield y, const std::string& fallback)
{
    if (!cond)
        return false;
    if (cond->str() == "(") {
        const Token* tok = cond->astOperand1();
        if (tok && tok->str() == ".") {
            if (tok->astOperand1() && tok->astOperand1()->valueType()) {
                if (const Library::Container *container = tok->astOperand1()->valueType()->container) {
                    return tok->astOperand2() && y == container->getYield(tok->astOperand2()->str());
                }
            } else if (!fallback.empty()) {
                return Token::simpleMatch(cond, "( )") && cond->strAt(-1) == fallback;
            }
        }
    }
    return false;
}

Library::Container::Yield Library::getContainerYield(const Token* const cond)
{
    if (Token::simpleMatch(cond, "(")) {
        const Token* tok = cond->astOperand1();
        if (tok && tok->str() == ".") {
            if (tok->astOperand1() && tok->astOperand1()->valueType()) {
                if (const Library::Container *container = tok->astOperand1()->valueType()->container) {
                    if (tok->astOperand2())
                        return container->getYield(tok->astOperand2()->str());
                }
            }
        }
    }
    return Library::Container::Yield::NO_YIELD;
}

// returns true if ftok is not a library function
bool Library::isNotLibraryFunction(const Token *ftok, const Function **func) const
{
    if (ftok->isKeyword() || ftok->isStandardType())
        return true;

    if (ftok->function() && ftok->function()->nestedIn && ftok->function()->nestedIn->type != ScopeType::eGlobal)
        return true;

    // variables are not library functions.
    if (ftok->varId())
        return true;

    return !matchArguments(ftok, getFunctionName(ftok), func);
}

bool Library::matchArguments(const Token *ftok, const std::string &functionName, const Function **func) const
{
    if (functionName.empty())
        return false;
    const auto it = utils::as_const(mData->mFunctions).find(functionName);
    if (it == mData->mFunctions.cend())
        return false;
    const int callargs = numberOfArgumentsWithoutAst(ftok);
    int args = 0;
    int firstOptionalArg = -1;
    for (const std::pair<const int, Library::ArgumentChecks> & argCheck : it->second.argumentChecks) {
        args = std::max(argCheck.first, args);
        if (argCheck.second.optional && (firstOptionalArg == -1 || firstOptionalArg > argCheck.first))
            firstOptionalArg = argCheck.first;

        if (argCheck.second.formatstr || argCheck.second.variadic) {
            const bool b = args <= callargs;
            if (b && func)
                *func = &it->second;
            return b;
        }
    }
    const bool b = (firstOptionalArg < 0) ? args == callargs : (callargs >= firstOptionalArg-1 && callargs <= args);
    if (b && func)
        *func = &it->second;
    return b;
}

const std::map<std::string, Library::WarnInfo>& Library::functionwarn() const
{
    return mData->mFunctionwarn;
}

const Library::WarnInfo* Library::getWarnInfo(const Token* ftok) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    const auto i = utils::as_const(mData->mFunctionwarn).find(getFunctionName(ftok));
    if (i ==  mData->mFunctionwarn.cend())
        return nullptr;
    return &i->second;
}

bool Library::isCompliantValidationExpression(const char* p)
{
    if (!p || !*p)
        return false;

    bool error = false;
    bool range = false;
    bool has_dot = false;
    bool has_E = false;

    error = *p == '.';
    for (; *p; p++) {
        if (std::isdigit(*p)) {
            error |= (*(p + 1) == '-');
        }
        else if (*p == ':') {
            error |= range || (*(p + 1) == '.');
            range = true;
            has_dot = false;
            has_E = false;
        }
        else if ((*p == '-') || (*p == '+')) {
            error |= (!std::isdigit(*(p + 1)));
        }
        else if (*p == ',') {
            range = false;
            error |= *(p + 1) == '.';
            has_dot = false;
            has_E = false;
        } else if (*p == '.') {
            error |= has_dot || (!std::isdigit(*(p + 1)));
            has_dot = true;
        } else if (*p == 'E' || *p == 'e') {
            error |= has_E;
            has_E = true;
        } else if (*p == '!') {
            error |= !((*(p+1) == '-') || (*(p+1) == '+') || (std::isdigit(*(p + 1))));
        } else
            return false;
    }
    return !error;
}

bool Library::formatstr_function(const Token* ftok) const
{
    if (isNotLibraryFunction(ftok))
        return false;

    const auto it = utils::as_const(mData->mFunctions).find(getFunctionName(ftok));
    if (it != mData->mFunctions.cend())
        return it->second.formatstr;
    return false;
}

int Library::formatstr_argno(const Token* ftok) const
{
    const std::map<int, Library::ArgumentChecks>& argumentChecksFunc = mData->mFunctions.at(getFunctionName(ftok)).argumentChecks;
    auto it = std::find_if(argumentChecksFunc.cbegin(), argumentChecksFunc.cend(), [](const std::pair<const int, Library::ArgumentChecks>& a) {
        return a.second.formatstr;
    });
    return it == argumentChecksFunc.cend() ? -1 : it->first - 1;
}

bool Library::formatstr_scan(const Token* ftok) const
{
    return mData->mFunctions.at(getFunctionName(ftok)).formatstr_scan;
}

bool Library::formatstr_secure(const Token* ftok) const
{
    return mData->mFunctions.at(getFunctionName(ftok)).formatstr_secure;
}

const Library::NonOverlappingData* Library::getNonOverlappingData(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    const auto it = utils::as_const(mData->mNonOverlappingData).find(getFunctionName(ftok));
    return (it != mData->mNonOverlappingData.cend()) ? &it->second : nullptr;
}

Library::UseRetValType Library::getUseRetValType(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".")) {
            const Token* contTok = ftok->astParent()->astOperand1();
            using Yield = Library::Container::Yield;
            const Yield yield = astContainerYield(contTok, *this);
            if (yield == Yield::START_ITERATOR || yield == Yield::END_ITERATOR || yield == Yield::AT_INDEX ||
                yield == Yield::SIZE || yield == Yield::EMPTY || yield == Yield::BUFFER || yield == Yield::BUFFER_NT ||
                ((yield == Yield::ITEM || yield == Yield::ITERATOR) && astContainerAction(contTok, *this) == Library::Container::Action::NO_ACTION))
                return Library::UseRetValType::DEFAULT;
        }
        return Library::UseRetValType::NONE;
    }
    const auto it = utils::as_const(mData->mFunctions).find(getFunctionName(ftok));
    if (it != mData->mFunctions.cend())
        return it->second.useretval;
    return Library::UseRetValType::NONE;
}

const std::string& Library::returnValue(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return mEmptyString;
    const auto it = utils::as_const(mData->mReturnValue).find(getFunctionName(ftok));
    return it != mData->mReturnValue.cend() ? it->second : mEmptyString;
}

const std::string& Library::returnValueType(const Token *ftok) const
{
    while (Token::simpleMatch(ftok, "::"))
        ftok = ftok->astOperand2() ? ftok->astOperand2() : ftok->astOperand1();
    if (!ftok)
        return mEmptyString;
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".") && ftok->astParent()->astOperand1()) {
            const Token* contTok = ftok->astParent()->astOperand1();
            if (contTok->valueType() && contTok->valueType()->container)
                return contTok->valueType()->container->getReturnType(ftok->str());
        }
        return mEmptyString;
    }
    const auto it = utils::as_const(mData->mReturnValueType).find(getFunctionName(ftok));
    return it != mData->mReturnValueType.cend() ? it->second : mEmptyString;
}

int Library::returnValueContainer(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return -1;
    const auto it = utils::as_const(mData->mReturnValueContainer).find(getFunctionName(ftok));
    return it != mData->mReturnValueContainer.cend() ? it->second : -1;
}

std::vector<MathLib::bigint> Library::unknownReturnValues(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return std::vector<MathLib::bigint>();
    const auto it = utils::as_const(mData->mUnknownReturnValues).find(getFunctionName(ftok));
    return (it == mData->mUnknownReturnValues.cend()) ? std::vector<MathLib::bigint>() : it->second;
}

const Library::Function *Library::getFunction(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return nullptr;
    const auto it1 = utils::as_const(mData->mFunctions).find(getFunctionName(ftok));
    if (it1 == mData->mFunctions.cend())
        return nullptr;
    return &it1->second;
}


bool Library::hasminsize(const Token *ftok) const
{
    if (isNotLibraryFunction(ftok))
        return false;
    const auto it = utils::as_const(mData->mFunctions).find(getFunctionName(ftok));
    if (it == mData->mFunctions.cend())
        return false;
    return std::any_of(it->second.argumentChecks.cbegin(), it->second.argumentChecks.cend(), [](const std::pair<const int, Library::ArgumentChecks>& a) {
        return !a.second.minsizes.empty();
    });
}

Library::ArgumentChecks::Direction Library::getArgDirection(const Token* ftok, int argnr, int indirect) const
{
    const ArgumentChecks* arg = getarg(ftok, argnr);
    if (arg) {
        if (indirect < 0 || indirect >= arg->direction.size())
            return ArgumentChecks::Direction::DIR_UNKNOWN; // TODO: don't generate bad indirect values
        return arg->direction[indirect];
    }
    if (formatstr_function(ftok)) {
        const int fs_argno = formatstr_argno(ftok);
        if (fs_argno >= 0 && argnr >= fs_argno) {
            if (formatstr_scan(ftok))
                return ArgumentChecks::Direction::DIR_OUT;
            return ArgumentChecks::Direction::DIR_IN;
        }
    }
    return ArgumentChecks::Direction::DIR_UNKNOWN;
}

bool Library::ignorefunction(const std::string& functionName) const
{
    const auto it = utils::as_const(mData->mFunctions).find(functionName);
    if (it != mData->mFunctions.cend())
        return it->second.ignore;
    return false;
}
const std::unordered_map<std::string, Library::Function>& Library::functions() const
{
    return mData->mFunctions;
}
bool Library::isUse(const std::string& functionName) const
{
    const auto it = utils::as_const(mData->mFunctions).find(functionName);
    if (it != mData->mFunctions.cend())
        return it->second.use;
    return false;
}
bool Library::isLeakIgnore(const std::string& functionName) const
{
    const auto it = utils::as_const(mData->mFunctions).find(functionName);
    if (it != mData->mFunctions.cend())
        return it->second.leakignore;
    return false;
}
bool Library::isFunctionConst(const std::string& functionName, bool pure) const
{
    const auto it = utils::as_const(mData->mFunctions).find(functionName);
    if (it != mData->mFunctions.cend())
        return pure ? it->second.ispure : it->second.isconst;
    return false;
}
bool Library::isFunctionConst(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isConst())
        return true;
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".")) {
            using Yield = Library::Container::Yield;
            const Yield yield = astContainerYield(ftok->astParent()->astOperand1(), *this);
            if (yield == Yield::EMPTY || yield == Yield::SIZE || yield == Yield::BUFFER_NT)
                return true;
        }
        return false;
    }
    const auto it = utils::as_const(mData->mFunctions).find(getFunctionName(ftok));
    return (it != mData->mFunctions.cend() && it->second.isconst);
}

bool Library::isnoreturn(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isAttributeNoreturn())
        return true;
    if (ftok->variable() && ftok->variable()->nameToken()->isAttributeNoreturn())
        return true;
    if (isNotLibraryFunction(ftok)) {
        if (Token::simpleMatch(ftok->astParent(), ".")) {
            const Token* contTok = ftok->astParent()->astOperand1();
            if (astContainerAction(contTok, *this) != Library::Container::Action::NO_ACTION ||
                astContainerYield(contTok, *this) != Library::Container::Yield::NO_YIELD)
                return false;
        }
        return false;
    }
    const auto it = utils::as_const(mData->mNoReturn).find(getFunctionName(ftok));
    if (it == mData->mNoReturn.end())
        return false;
    if (it->second == LibraryData::FalseTrueMaybe::Maybe)
        return true;
    return it->second == LibraryData::FalseTrueMaybe::True;
}

bool Library::isnotnoreturn(const Token *ftok) const
{
    if (ftok->function() && ftok->function()->isAttributeNoreturn())
        return false;
    if (isNotLibraryFunction(ftok))
        return hasAnyTypeCheck(getFunctionName(ftok));
    const auto it = utils::as_const(mData->mNoReturn).find(getFunctionName(ftok));
    if (it == mData->mNoReturn.end())
        return false;
    if (it->second == LibraryData::FalseTrueMaybe::Maybe)
        return false;
    return it->second == LibraryData::FalseTrueMaybe::False;
}

bool Library::markupFile(const std::string &path) const
{
    return mData->mMarkupExtensions.find(Path::getFilenameExtensionInLowerCase(path)) != mData->mMarkupExtensions.end();
}

bool Library::processMarkupAfterCode(const std::string &path) const
{
    const auto it = utils::as_const(mData->mProcessAfterCode).find(Path::getFilenameExtensionInLowerCase(path));
    return (it == mData->mProcessAfterCode.cend() || it->second);
}

bool Library::reportErrors(const std::string &path) const
{
    const auto it = utils::as_const(mData->mReportErrors).find(Path::getFilenameExtensionInLowerCase(path));
    return (it == mData->mReportErrors.cend() || it->second);
}

bool Library::isexecutableblock(const std::string &file, const std::string &token) const
{
    const auto it = utils::as_const(mData->mExecutableBlocks).find(Path::getFilenameExtensionInLowerCase(file));
    return (it != mData->mExecutableBlocks.cend() && it->second.isBlock(token));
}

int Library::blockstartoffset(const std::string &file) const
{
    int offset = -1;
    const auto map_it
        = utils::as_const(mData->mExecutableBlocks).find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != mData->mExecutableBlocks.end()) {
        offset = map_it->second.offset();
    }
    return offset;
}

const std::string& Library::blockstart(const std::string &file) const
{
    const auto map_it
        = utils::as_const(mData->mExecutableBlocks).find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != mData->mExecutableBlocks.end()) {
        return map_it->second.start();
    }
    return mEmptyString;
}

const std::string& Library::blockend(const std::string &file) const
{
    const auto map_it
        = utils::as_const(mData->mExecutableBlocks).find(Path::getFilenameExtensionInLowerCase(file));

    if (map_it != mData->mExecutableBlocks.end()) {
        return map_it->second.end();
    }
    return mEmptyString;
}

bool Library::iskeyword(const std::string &file, const std::string &keyword) const
{
    const auto it =
        utils::as_const(mData->mKeywords).find(Path::getFilenameExtensionInLowerCase(file));
    return (it != mData->mKeywords.end() && it->second.count(keyword));
}

bool Library::isimporter(const std::string& file, const std::string &importer) const
{
    const auto it =
        utils::as_const(mData->mImporters).find(Path::getFilenameExtensionInLowerCase(file));
    return (it != mData->mImporters.end() && it->second.count(importer) > 0);
}

const Token* Library::getContainerFromYield(const Token* tok, Library::Container::Yield yield) const
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok->tokAt(-2), ". %name% (")) {
        const Token* containerTok = tok->tokAt(-2)->astOperand1();
        if (!astIsContainer(containerTok))
            return nullptr;
        if (containerTok->valueType()->container &&
            containerTok->valueType()->container->getYield(tok->strAt(-1)) == yield)
            return containerTok;
        if (yield == Library::Container::Yield::EMPTY && Token::simpleMatch(tok->tokAt(-1), "empty ( )"))
            return containerTok;
        if (yield == Library::Container::Yield::SIZE && Token::Match(tok->tokAt(-1), "size|length ( )"))
            return containerTok;
    } else if (Token::Match(tok->previous(), "%name% (")) {
        if (const Library::Function* f = this->getFunction(tok->previous())) {
            if (f->containerYield == yield) {
                return tok->astOperand2();
            }
        }
    }
    return nullptr;
}

// cppcheck-suppress unusedFunction
const Token* Library::getContainerFromAction(const Token* tok, Library::Container::Action action) const
{
    if (!tok)
        return nullptr;
    if (Token::Match(tok->tokAt(-2), ". %name% (")) {
        const Token* containerTok = tok->tokAt(-2)->astOperand1();
        if (!astIsContainer(containerTok))
            return nullptr;
        if (containerTok->valueType()->container &&
            containerTok->valueType()->container->getAction(tok->strAt(-1)) == action)
            return containerTok;
        if (Token::simpleMatch(tok->tokAt(-1), "empty ( )"))
            return containerTok;
    } else if (Token::Match(tok->previous(), "%name% (")) {
        if (const Library::Function* f = this->getFunction(tok->previous())) {
            if (f->containerAction == action) {
                return tok->astOperand2();
            }
        }
    }
    return nullptr;
}

const std::unordered_map<std::string, Library::SmartPointer>& Library::smartPointers() const
{
    return mData->mSmartPointers;
}

bool Library::isSmartPointer(const Token* tok) const
{
    return detectSmartPointer(tok);
}

const Library::SmartPointer* Library::detectSmartPointer(const Token* tok, bool withoutStd) const
{
    if (!tok)
        return nullptr;
    if (tok->isKeyword())
        return nullptr;
    std::string typestr = withoutStd ? "std::" : "";
    if (tok->str() == "::")
        tok = tok->next();
    while (Token::Match(tok, "%name% ::")) {
        typestr += tok->str();
        typestr += "::";
        tok = tok->tokAt(2);
    }
    if (tok && tok->isName()) {
        typestr += tok->str();
    }
    auto it = mData->mSmartPointers.find(typestr);
    if (it == mData->mSmartPointers.end())
        return nullptr;
    return &it->second;
}

const Library::Container * getLibraryContainer(const Token * tok)
{
    if (!tok)
        return nullptr;
    // TODO: Support dereferencing iterators
    // TODO: Support dereferencing with ->
    if (tok->isUnaryOp("*") && astIsPointer(tok->astOperand1())) {
        for (const ValueFlow::Value& v:tok->astOperand1()->values()) {
            if (!v.isLocalLifetimeValue())
                continue;
            if (v.lifetimeKind != ValueFlow::Value::LifetimeKind::Address)
                continue;
            return getLibraryContainer(v.tokvalue);
        }
    }
    if (!tok->valueType())
        return nullptr;
    return tok->valueType()->container;
}

Library::TypeCheck Library::getTypeCheck(std::string check,  std::string typeName) const
{
    auto it = mData->mTypeChecks.find(std::pair<std::string, std::string>(std::move(check), std::move(typeName)));
    return it == mData->mTypeChecks.end() ? TypeCheck::def : it->second;
}

bool Library::hasAnyTypeCheck(const std::string& typeName) const
{
    return std::any_of(mData->mTypeChecks.begin(), mData->mTypeChecks.end(), [&](const std::pair<const std::pair<std::string, std::string>, Library::TypeCheck>& tc) {
        return tc.first.second == typeName;
    });
}

const Library::AllocFunc* Library::getAllocFuncInfo(const char name[]) const
{
    return getAllocDealloc(mData->mAlloc, name);
}

const Library::AllocFunc* Library::getDeallocFuncInfo(const char name[]) const
{
    return getAllocDealloc(mData->mDealloc, name);
}

// cppcheck-suppress unusedFunction
int Library::allocId(const char name[]) const
{
    const AllocFunc* af = getAllocDealloc(mData->mAlloc, name);
    return af ? af->groupId : 0;
}

int Library::deallocId(const char name[]) const
{
    const AllocFunc* af = getAllocDealloc(mData->mDealloc, name);
    return af ? af->groupId : 0;
}

const std::set<std::string> &Library::markupExtensions() const
{
    return mData->mMarkupExtensions;
}

bool Library::isexporter(const std::string &prefix) const
{
    return mData->mExporters.find(prefix) != mData->mExporters.end();
}

bool Library::isexportedprefix(const std::string &prefix, const std::string &token) const
{
    const auto it = utils::as_const(mData->mExporters).find(prefix);
    return (it != mData->mExporters.end() && it->second.isPrefix(token));
}

bool Library::isexportedsuffix(const std::string &prefix, const std::string &token) const
{
    const auto it = utils::as_const(mData->mExporters).find(prefix);
    return (it != mData->mExporters.end() && it->second.isSuffix(token));
}

bool Library::isreflection(const std::string &token) const
{
    return mData->mReflection.find(token) != mData->mReflection.end();
}

int Library::reflectionArgument(const std::string &token) const
{
    const auto it = utils::as_const(mData->mReflection).find(token);
    if (it != mData->mReflection.end())
        return it->second;
    return -1;
}

bool Library::isentrypoint(const std::string &func) const
{
    return func == "main" || mData->mEntrypoints.find(func) != mData->mEntrypoints.end();
}

const std::set<std::string>& Library::defines() const
{
    return mData->mDefines;
}

const Library::PodType *Library::podtype(const std::string &name) const
{
    const auto it = utils::as_const(mData->mPodTypes).find(name);
    return (it != mData->mPodTypes.end()) ? &(it->second) : nullptr;
}

const Library::PlatformType *Library::platform_type(const std::string &name, const std::string & platform) const
{
    const auto it = utils::as_const(mData->mPlatforms).find(platform);
    if (it != mData->mPlatforms.end()) {
        const PlatformType * const type = it->second.platform_type(name);
        if (type)
            return type;
    }

    const auto it2 = utils::as_const(mData->mPlatformTypes).find(name);
    return (it2 != mData->mPlatformTypes.end()) ? &(it2->second) : nullptr;
}
