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
#ifndef libraryH
#define libraryH
//---------------------------------------------------------------------------

#include "config.h"
#include "mathlib.h"
#include "standards.h"

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Token;
class Settings; // TODO: remove dependency on settings
enum class Severity : std::uint8_t;

namespace tinyxml2 {
    class XMLDocument;
    class XMLElement;
}

/// @addtogroup Core
/// @{

/**
 * @brief Library definitions handling
 */
class CPPCHECKLIB Library {
    friend struct LibraryHelper; // for testing

    static const std::string mEmptyString;

public:
    Library();
    ~Library();

    Library(const Library& other);
    Library& operator=(const Library& other) &;

    enum class ErrorCode : std::uint8_t {
        OK,
        FILE_NOT_FOUND, BAD_XML, UNKNOWN_ELEMENT, MISSING_ATTRIBUTE, BAD_ATTRIBUTE_VALUE,
        UNSUPPORTED_FORMAT, DUPLICATE_PLATFORM_TYPE, PLATFORM_TYPE_REDEFINED, DUPLICATE_DEFINE
    };

    class Error {
    public:
        Error() : errorcode(ErrorCode::OK) {}
        explicit Error(ErrorCode e) : errorcode(e) {}
        template<typename T>
        Error(ErrorCode e, T&& r) : errorcode(e), reason(r) {}
        ErrorCode errorcode;
        std::string reason;
    };

    Error load(const char exename[], const char path[], bool debug = false);

    struct AllocFunc {
        int groupId{};
        int arg{};
        enum class BufferSize : std::uint8_t {none,malloc,calloc,strdup};
        BufferSize bufferSize{BufferSize::none};
        int bufferSizeArg1{};
        int bufferSizeArg2{};
        int reallocArg{};
        bool initData{};
        bool noFail{};
    };

    /** get allocation info for function */
    const AllocFunc* getAllocFuncInfo(const Token *tok) const;

    /** get deallocation info for function */
    const AllocFunc* getDeallocFuncInfo(const Token *tok) const;

    /** get reallocation info for function */
    const AllocFunc* getReallocFuncInfo(const Token *tok) const;

    /** get allocation id for function */
    int getAllocId(const Token *tok, int arg) const;

    /** get deallocation id for function */
    int getDeallocId(const Token *tok, int arg) const;

    /** get reallocation id for function */
    int getReallocId(const Token *tok, int arg) const;

    // TODO: get rid of this
    /** get allocation info for function by name (deprecated, use other alloc) */
    const AllocFunc* getAllocFuncInfo(const char name[]) const;

    // TODO: get rid of this
    /** get deallocation info for function by name (deprecated, use other alloc) */
    const AllocFunc* getDeallocFuncInfo(const char name[]) const;

    // TODO: get rid of this
    /** get allocation id for function by name (deprecated, use other alloc) */
    int allocId(const char name[]) const;

    // TODO: get rid of this
    /** get deallocation id for function by name (deprecated, use other alloc) */
    int deallocId(const char name[]) const;

    static bool isCompliantValidationExpression(const char* p);

    /** is allocation type memory? */
    static bool ismemory(const int id) {
        return ((id > 0) && ((id & 1) == 0));
    }
    static bool ismemory(const AllocFunc* const func) {
        return func && (func->groupId > 0) && ((func->groupId & 1) == 0);
    }

    /** is allocation type resource? */
    static bool isresource(const int id) {
        return ((id > 0) && ((id & 1) == 1));
    }
    static bool isresource(const AllocFunc* const func) {
        return func && (func->groupId > 0) && ((func->groupId & 1) == 1);
    }

    bool formatstr_function(const Token* ftok) const;
    int formatstr_argno(const Token* ftok) const;
    bool formatstr_scan(const Token* ftok) const;
    bool formatstr_secure(const Token* ftok) const;

    struct NonOverlappingData {
        int ptr1Arg;
        int ptr2Arg;
        int sizeArg;
        int strlenArg;
        int countArg;
    };
    const NonOverlappingData* getNonOverlappingData(const Token *ftok) const;

    struct WarnInfo {
        std::string message;
        Standards standards;
        Severity severity;
    };
    const std::map<std::string, WarnInfo>& functionwarn() const;

    const WarnInfo* getWarnInfo(const Token* ftok) const;

    struct Function;

    // returns true if ftok is not a library function
    bool isNotLibraryFunction(const Token *ftok, const Function **func = nullptr) const;
    bool matchArguments(const Token *ftok, const std::string &functionName, const Function **func = nullptr) const;

    enum class UseRetValType : std::uint8_t { NONE, DEFAULT, ERROR_CODE };
    UseRetValType getUseRetValType(const Token* ftok) const;

    const std::string& returnValue(const Token *ftok) const;
    const std::string& returnValueType(const Token *ftok) const;
    int returnValueContainer(const Token *ftok) const;
    std::vector<MathLib::bigint> unknownReturnValues(const Token *ftok) const;

    bool isnoreturn(const Token *ftok) const;
    bool isnotnoreturn(const Token *ftok) const;

    bool isScopeNoReturn(const Token *end, std::string *unknownFunc) const;

    class Container {
    public:
        Container() = default;

        enum class Action : std::uint8_t {
            RESIZE,
            CLEAR,
            PUSH,
            POP,
            FIND,
            FIND_CONST,
            INSERT,
            ERASE,
            APPEND,
            CHANGE_CONTENT,
            CHANGE,
            CHANGE_INTERNAL,
            NO_ACTION
        };
        enum class Yield : std::uint8_t {
            AT_INDEX,
            ITEM,
            BUFFER,
            BUFFER_NT,
            START_ITERATOR,
            END_ITERATOR,
            ITERATOR,
            SIZE,
            EMPTY,
            NO_YIELD
        };
        struct Function {
            Action action;
            Yield yield;
            std::string returnType;
        };
        struct RangeItemRecordTypeItem {
            std::string name;
            int templateParameter; // TODO: use this
        };
        std::string startPattern, startPattern2, endPattern, itEndPattern;
        std::map<std::string, Function> functions;
        int type_templateArgNo = -1;
        std::vector<RangeItemRecordTypeItem> rangeItemRecordType;
        int size_templateArgNo = -1;
        bool arrayLike_indexOp{};
        bool stdStringLike{};
        bool stdAssociativeLike{};
        bool opLessAllowed = true;
        bool hasInitializerListConstructor{};
        bool unstableErase{};
        bool unstableInsert{};
        bool view{};
        bool startPatternHasStd{};

        Action getAction(const std::string& function) const {
            const auto i = functions.find(function);
            if (i != functions.end())
                return i->second.action;
            return Action::NO_ACTION;
        }

        Yield getYield(const std::string& function) const {
            const auto i = functions.find(function);
            if (i != functions.end())
                return i->second.yield;
            return Yield::NO_YIELD;
        }

        const std::string& getReturnType(const std::string& function) const {
            const auto i = functions.find(function);
            return (i != functions.end()) ? i->second.returnType : mEmptyString;
        }

        CPPCHECKLIB static Yield yieldFrom(const std::string& yieldName);
        CPPCHECKLIB static Action actionFrom(const std::string& actionName);
        CPPCHECKLIB static std::string toString(Yield yield);
        CPPCHECKLIB static std::string toString(Action action);
    };
    const std::unordered_map<std::string, Container>& containers() const;
    const Container* detectContainer(const Token* typeStart) const;
    const Container* detectIterator(const Token* typeStart) const;
    const Container* detectContainerOrIterator(const Token* typeStart, bool* isIterator = nullptr, bool withoutStd = false) const;

    struct ArgumentChecks {
        bool notbool{};
        bool notnull{};
        int notuninit = -1;
        bool formatstr{};
        bool strz{};
        bool optional{};
        bool variadic{};
        std::string valid;

        struct IteratorInfo {
            int container{};
            bool it{};
            bool first{};
            bool last{};
        };
        IteratorInfo iteratorInfo;

        struct MinSize {
            enum class Type : std::uint8_t { NONE, STRLEN, ARGVALUE, SIZEOF, MUL, VALUE };
            MinSize(Type t, int a) : type(t), arg(a) {}
            Type type;
            int arg;
            int arg2 = 0;
            long long value = 0;
            std::string baseType;
        };
        std::vector<MinSize> minsizes;

        enum class Direction : std::uint8_t {
            DIR_IN,     ///< Input to called function. Data is treated as read-only.
            DIR_OUT,    ///< Output to caller. Data is passed by reference or address and is potentially written.
            DIR_INOUT,  ///< Input to called function, and output to caller. Data is passed by reference or address and is potentially modified.
            DIR_UNKNOWN ///< direction not known / specified
        };
        // argument directions up to ** indirect level (only one can be configured explicitly at the moment)
        std::array<Direction, 3> direction = { { Direction::DIR_UNKNOWN, Direction::DIR_UNKNOWN, Direction::DIR_UNKNOWN } };
    };

    struct Function {
        std::map<int, ArgumentChecks> argumentChecks; // argument nr => argument data
        bool use{};
        bool leakignore{};
        bool isconst{};
        bool ispure{};
        UseRetValType useretval = UseRetValType::NONE;
        bool ignore{};  // ignore functions/macros from a library (gtk, qt etc)
        bool formatstr{};
        bool formatstr_scan{};
        bool formatstr_secure{};
        Container::Action containerAction = Container::Action::NO_ACTION;
        Container::Yield containerYield = Container::Yield::NO_YIELD;
        std::string returnType;
    };

    const Function *getFunction(const Token *ftok) const;
    const std::unordered_map<std::string, Function>& functions() const;
    bool isUse(const std::string& functionName) const;
    bool isLeakIgnore(const std::string& functionName) const;
    bool isFunctionConst(const std::string& functionName, bool pure) const;
    bool isFunctionConst(const Token *ftok) const;

    bool isboolargbad(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->notbool;
    }

    bool isnullargbad(const Token *ftok, int argnr) const;
    bool isuninitargbad(const Token *ftok, int argnr, int indirect = 0, bool *hasIndirect=nullptr) const;

    bool isargformatstr(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->formatstr;
    }

    bool isargstrz(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->strz;
    }

    bool isIntArgValid(const Token *ftok, int argnr, MathLib::bigint argvalue, const Settings& settings) const;
    bool isFloatArgValid(const Token *ftok, int argnr, double argvalue, const Settings& settings) const;

    const std::string& validarg(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg ? arg->valid : mEmptyString;
    }

    const ArgumentChecks::IteratorInfo *getArgIteratorInfo(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg && arg->iteratorInfo.it ? &arg->iteratorInfo : nullptr;
    }

    bool hasminsize(const Token *ftok) const;

    const std::vector<ArgumentChecks::MinSize> *argminsizes(const Token *ftok, int argnr) const {
        const ArgumentChecks *arg = getarg(ftok, argnr);
        return arg ? &arg->minsizes : nullptr;
    }

    ArgumentChecks::Direction getArgDirection(const Token* ftok, int argnr, int indirect = 0) const;

    bool markupFile(const std::string &path) const;

    bool processMarkupAfterCode(const std::string &path) const;

    const std::set<std::string> &markupExtensions() const;

    bool reportErrors(const std::string &path) const;

    bool ignorefunction(const std::string &functionName) const;

    bool isexecutableblock(const std::string &file, const std::string &token) const;

    int blockstartoffset(const std::string &file) const;

    const std::string& blockstart(const std::string &file) const;
    const std::string& blockend(const std::string &file) const;

    bool iskeyword(const std::string &file, const std::string &keyword) const;

    bool isexporter(const std::string &prefix) const;

    bool isexportedprefix(const std::string &prefix, const std::string &token) const;

    bool isexportedsuffix(const std::string &prefix, const std::string &token) const;

    bool isimporter(const std::string& file, const std::string &importer) const;

    const Token* getContainerFromYield(const Token* tok, Container::Yield yield) const;
    const Token* getContainerFromAction(const Token* tok, Container::Action action) const;

    static bool isContainerYield(const Token* cond, Library::Container::Yield y, const std::string& fallback = "");
    static Library::Container::Yield getContainerYield(const Token* cond);

    bool isreflection(const std::string &token) const;

    int reflectionArgument(const std::string &token) const;

    bool isentrypoint(const std::string &func) const;

    const std::set<std::string>& defines() const; // to provide some library defines

    struct SmartPointer {
        std::string name;
        bool unique = false;
    };

    const std::unordered_map<std::string, SmartPointer>& smartPointers() const;
    bool isSmartPointer(const Token *tok) const;
    const SmartPointer* detectSmartPointer(const Token* tok, bool withoutStd = false) const;

    struct PodType {
        unsigned int size{};
        char sign{};
        enum class Type : std::uint8_t { NO, BOOL, CHAR, SHORT, INT, LONG, LONGLONG } stdtype = Type::NO;
    };
    const PodType *podtype(const std::string &name) const;

    struct PlatformType {
        bool operator == (const PlatformType & type) const {
            return (mSigned == type.mSigned &&
                    mUnsigned == type.mUnsigned &&
                    mLong == type.mLong &&
                    mPointer == type.mPointer &&
                    mPtrPtr == type.mPtrPtr &&
                    mConstPtr == type.mConstPtr &&
                    mType == type.mType);
        }
        bool operator != (const PlatformType & type) const {
            return !(*this == type);
        }
        std::string mType;
        bool mSigned{};
        bool mUnsigned{};
        bool mLong{};
        bool mPointer{};
        bool mPtrPtr{};
        bool mConstPtr{};
    };

    const PlatformType *platform_type(const std::string &name, const std::string & platform) const;

    /**
     * Get function name for function call
     */
    std::string getFunctionName(const Token *ftok) const;

    /** Suppress/check a type */
    enum class TypeCheck : std::uint8_t {
        def,
        check,
        suppress,
        checkFiniteLifetime, // (unusedvar) object has side effects, but immediate destruction is wrong
    };
    TypeCheck getTypeCheck(std::string check, std::string typeName) const;
    bool hasAnyTypeCheck(const std::string& typeName) const;

private:
    Error load(const tinyxml2::XMLDocument &doc);

    // load a <function> xml node
    Error loadFunction(const tinyxml2::XMLElement * node, const std::string &name, std::set<std::string> &unknown_elements);

    struct LibraryData;
    std::unique_ptr<LibraryData> mData;

    const ArgumentChecks * getarg(const Token *ftok, int argnr) const;

    std::string getFunctionName(const Token *ftok, bool &error) const;

    static const AllocFunc* getAllocDealloc(const std::map<std::string, AllocFunc> &data, const std::string &name) {
        const auto it = data.find(name);
        return (it == data.end()) ? nullptr : &it->second;
    }

    enum DetectContainer : std::uint8_t { ContainerOnly, IteratorOnly, Both };
    const Library::Container* detectContainerInternal(const Token* typeStart, DetectContainer detect, bool* isIterator = nullptr, bool withoutStd = false) const;
};

CPPCHECKLIB const Library::Container * getLibraryContainer(const Token * tok);

/// @}
//---------------------------------------------------------------------------
#endif // libraryH
