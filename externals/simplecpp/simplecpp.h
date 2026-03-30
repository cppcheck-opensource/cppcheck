/* -*- C++ -*-
 * simplecpp - A simple and high-fidelity C/C++ preprocessor library
 * Copyright (C) 2016-2023 simplecpp team
 */

#ifndef simplecppH
#define simplecppH

#include <cctype>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#if __cplusplus >= 202002L
#  include <version>
#endif

#if defined(__cpp_lib_string_view) && !defined(__cpp_lib_span)
#include <string_view>
#endif
#ifdef __cpp_lib_span
#include <span>
#endif

#ifdef _WIN32
#  ifdef SIMPLECPP_EXPORT
#    define SIMPLECPP_LIB __declspec(dllexport)
#  elif defined(SIMPLECPP_IMPORT)
#    define SIMPLECPP_LIB __declspec(dllimport)
#  else
#    define SIMPLECPP_LIB
#  endif
#else
#  define SIMPLECPP_LIB
#endif

#ifndef _WIN32
#  include <sys/types.h>
#endif

#if defined(_MSC_VER)
#  pragma warning(push)
// suppress warnings about "conversion from 'type1' to 'type2', possible loss of data"
#  pragma warning(disable : 4267)
#  pragma warning(disable : 4244)
#endif

// provide legacy (i.e. raw pointer) API for TokenList
// note: std::istream has an overhead compared to raw pointers
#ifndef SIMPLECPP_TOKENLIST_ALLOW_PTR
// still provide the legacy API in case we lack the performant wrappers
#  if !defined(__cpp_lib_string_view) && !defined(__cpp_lib_span)
#    define SIMPLECPP_TOKENLIST_ALLOW_PTR 1
#  else
#    define SIMPLECPP_TOKENLIST_ALLOW_PTR 0
#  endif
#endif

namespace simplecpp {
    /** C code standard */
    enum cstd_t : std::int8_t { CUnknown=-1, C89, C99, C11, C17, C23, C2Y };

    /** C++ code standard */
    enum cppstd_t : std::int8_t { CPPUnknown=-1, CPP03, CPP11, CPP14, CPP17, CPP20, CPP23, CPP26 };

    using TokenString = std::string;

#if defined(__cpp_lib_string_view) && !defined(__cpp_lib_span)
    using View = std::string_view;
#else
    struct View
    {
        // cppcheck-suppress noExplicitConstructor
        View(const char* data)
            : mData(data)
            , mSize(strlen(data))
        {}

        // only provide when std::span is not available so using untyped initialization won't use View
#if !defined(__cpp_lib_span)
        View(const char* data, std::size_t size)
            : mData(data)
            , mSize(size)
        {}

        // cppcheck-suppress noExplicitConstructor
        View(const std::string& str)
            : mData(str.data())
            , mSize(str.size())
        {}
#endif // !defined(__cpp_lib_span)

        const char* data() const {
            return mData;
        }

        std::size_t size() const {
            return mSize;
        }

    private:
        const char* mData;
        std::size_t mSize;
    };
#endif // defined(__cpp_lib_string_view) && !defined(__cpp_lib_span)

    class Macro;

    /**
     * Location in source code
     */
    struct SIMPLECPP_LIB Location {
        Location() = default;
        Location(unsigned int fileIndex, unsigned int line, unsigned int col)
            : fileIndex(fileIndex)
            , line(line)
            , col(col)
        {}

        Location(const Location &loc) = default;
        Location &operator=(const Location &other) = default;

        /** increment this location by string */
        void adjust(const std::string &str);

        bool operator<(const Location &rhs) const {
            if (fileIndex != rhs.fileIndex)
                return fileIndex < rhs.fileIndex;
            if (line != rhs.line)
                return line < rhs.line;
            return col < rhs.col;
        }

        bool sameline(const Location &other) const {
            return fileIndex == other.fileIndex && line == other.line;
        }

        unsigned int fileIndex{};
        unsigned int line{};
        unsigned int col{};
    };

    /**
     * token class.
     * @todo don't use std::string representation - for both memory and performance reasons
     */
    class SIMPLECPP_LIB Token {
    public:
        Token(const TokenString &s, const Location &loc, bool wsahead = false) :
            whitespaceahead(wsahead), location(loc), string(s) {
            flags();
        }

        Token(const Token &tok) :
            macro(tok.macro), op(tok.op), comment(tok.comment), name(tok.name), number(tok.number), whitespaceahead(tok.whitespaceahead), location(tok.location), string(tok.string), mExpandedFrom(tok.mExpandedFrom) {}

        Token &operator=(const Token &tok) = delete;

        const TokenString& str() const {
            return string;
        }
        void setstr(const std::string &s) {
            string = s;
            flags();
        }

        bool isOneOf(const char ops[]) const;
        bool startsWithOneOf(const char c[]) const;
        bool endsWithOneOf(const char c[]) const;
        static bool isNumberLike(const std::string& str) {
            return std::isdigit(static_cast<unsigned char>(str[0])) ||
                   (str.size() > 1U && (str[0] == '-' || str[0] == '+') && std::isdigit(static_cast<unsigned char>(str[1])));
        }

        TokenString macro;
        char op;
        bool comment;
        bool name;
        bool number;
        bool whitespaceahead;
        Location location;
        Token *previous{};
        Token *next{};
        mutable const Token *nextcond{};

        const Token *previousSkipComments() const {
            const Token *tok = this->previous;
            while (tok && tok->comment)
                tok = tok->previous;
            return tok;
        }

        const Token *nextSkipComments() const {
            const Token *tok = this->next;
            while (tok && tok->comment)
                tok = tok->next;
            return tok;
        }

        void setExpandedFrom(const Token *tok, const Macro* m) {
            mExpandedFrom = tok->mExpandedFrom;
            mExpandedFrom.insert(m);
            if (tok->whitespaceahead)
                whitespaceahead = true;
        }
        bool isExpandedFrom(const Macro* m) const {
            return mExpandedFrom.find(m) != mExpandedFrom.end();
        }

        void printAll() const;
        void printOut() const;
    private:
        void flags() {
            name = (std::isalpha(static_cast<unsigned char>(string[0])) || string[0] == '_' || string[0] == '$')
                   && (std::memchr(string.c_str(), '\'', string.size()) == nullptr);
            comment = string.size() > 1U && string[0] == '/' && (string[1] == '/' || string[1] == '*');
            number = isNumberLike(string);
            op = (string.size() == 1U && !name && !comment && !number) ? string[0] : '\0';
        }

        TokenString string;

        std::set<const Macro*> mExpandedFrom;
    };

    /** Output from preprocessor */
    struct SIMPLECPP_LIB Output {
        enum Type : std::uint8_t {
            ERROR, /* #error */
            WARNING, /* #warning */
            MISSING_HEADER,
            INCLUDE_NESTED_TOO_DEEPLY,
            SYNTAX_ERROR,
            PORTABILITY_BACKSLASH,
            UNHANDLED_CHAR_ERROR,
            EXPLICIT_INCLUDE_NOT_FOUND,
            FILE_NOT_FOUND,
            DUI_ERROR
        } type;
        Output(Type type, const Location& loc, std::string msg) : type(type), location(loc), msg(std::move(msg)) {}
        Location location;
        std::string msg;
    };

    using OutputList = std::list<Output>;

    /** List of tokens. */
    class SIMPLECPP_LIB TokenList {
    public:
        class Stream;

        explicit TokenList(std::vector<std::string> &filenames);
        /** generates a token list from the given std::istream parameter */
        TokenList(std::istream &istr, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr);
        /** generates a token list from the given buffer */
        template<size_t size>
        TokenList(const char (&data)[size], std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
            : TokenList(reinterpret_cast<const unsigned char*>(data), size-1, filenames, filename, outputList, 0)
        {}
        /** generates a token list from the given buffer */
        template<size_t size>
        TokenList(const unsigned char (&data)[size], std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
            : TokenList(data, size-1, filenames, filename, outputList, 0)
        {}
#if SIMPLECPP_TOKENLIST_ALLOW_PTR
        /** generates a token list from the given buffer */
        TokenList(const unsigned char* data, std::size_t size, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
            : TokenList(data, size, filenames, filename, outputList, 0)
        {}
        /** generates a token list from the given buffer */
        TokenList(const char* data, std::size_t size, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
            : TokenList(reinterpret_cast<const unsigned char*>(data), size, filenames, filename, outputList, 0)
        {}
#endif // SIMPLECPP_TOKENLIST_ALLOW_PTR
        /** generates a token list from the given buffer */
        TokenList(View data, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
            : TokenList(reinterpret_cast<const unsigned char*>(data.data()), data.size(), filenames, filename, outputList, 0)
        {}
#ifdef __cpp_lib_span
        /** generates a token list from the given buffer */
        TokenList(std::span<const char> data, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
            : TokenList(reinterpret_cast<const unsigned char*>(data.data()), data.size(), filenames, filename, outputList, 0)
        {}

        /** generates a token list from the given buffer */
        TokenList(std::span<const unsigned char> data, std::vector<std::string> &filenames, const std::string &filename=std::string(), OutputList *outputList = nullptr)
            : TokenList(data.data(), data.size(), filenames, filename, outputList, 0)
        {}
#endif // __cpp_lib_span

        /** generates a token list from the given filename parameter */
        TokenList(const std::string &filename, std::vector<std::string> &filenames, OutputList *outputList = nullptr);
        TokenList(const TokenList &other);
        TokenList(TokenList &&other);
        ~TokenList();
        TokenList &operator=(const TokenList &other);
        TokenList &operator=(TokenList &&other);

        void clear();
        bool empty() const {
            return !frontToken;
        }
        void push_back(Token *tok);

        void dump(bool linenrs = false) const;
        std::string stringify(bool linenrs = false) const;

        void readfile(Stream &stream, const std::string &filename=std::string(), OutputList *outputList = nullptr);
        /**
         * @throws std::overflow_error thrown on overflow or division by zero
         * @throws std::runtime_error thrown on invalid expressions
         */
        void constFold();

        void removeComments();

        Token *front() {
            return frontToken;
        }

        const Token *cfront() const {
            return frontToken;
        }

        Token *back() {
            return backToken;
        }

        const Token *cback() const {
            return backToken;
        }

        void deleteToken(Token *tok) {
            if (!tok)
                return;
            Token * const prev = tok->previous;
            Token * const next = tok->next;
            if (prev)
                prev->next = next;
            if (next)
                next->previous = prev;
            if (frontToken == tok)
                frontToken = next;
            if (backToken == tok)
                backToken = prev;
            delete tok;
        }

        void takeTokens(TokenList &other) {
            if (!other.frontToken)
                return;
            if (!frontToken) {
                frontToken = other.frontToken;
            } else {
                backToken->next = other.frontToken;
                other.frontToken->previous = backToken;
            }
            backToken = other.backToken;
            other.frontToken = other.backToken = nullptr;
        }

        /** sizeof(T) */
        std::map<std::string, std::size_t> sizeOfType;

        const std::vector<std::string>& getFiles() const {
            return files;
        }

        const std::string& file(const Location& loc) const;

    private:
        TokenList(const unsigned char* data, std::size_t size, std::vector<std::string> &filenames, const std::string &filename, OutputList *outputList, int unused);

        void combineOperators();

        void constFoldUnaryNotPosNeg(Token *tok);
        /**
         * @throws std::overflow_error thrown on overflow or division by zero
         */
        void constFoldMulDivRem(Token *tok);
        void constFoldAddSub(Token *tok);
        void constFoldShift(Token *tok);
        void constFoldComparison(Token *tok);
        void constFoldBitwise(Token *tok);
        void constFoldLogicalOp(Token *tok);
        /**
         * @throws std::runtime_error thrown on invalid expressions
         */
        void constFoldQuestionOp(Token *&tok1);

        std::string readUntil(Stream &stream, const Location &location, char start, char end, OutputList *outputList);
        void lineDirective(unsigned int fileIndex, unsigned int line, Location &location);

        const Token* lastLineTok(int maxsize=1000) const;
        const Token* isLastLinePreprocessor(int maxsize=1000) const;

        unsigned int fileIndex(const std::string &filename);

        Token *frontToken;
        Token *backToken;
        std::vector<std::string> &files;
    };

    /** Tracking how macros are used */
    struct SIMPLECPP_LIB MacroUsage {
        explicit MacroUsage(bool macroValueKnown_) : macroValueKnown(macroValueKnown_) {}
        std::string macroName;
        Location macroLocation;
        Location useLocation;
        bool macroValueKnown;
    };

    /** Tracking #if/#elif expressions */
    struct SIMPLECPP_LIB IfCond {
        explicit IfCond(const Location& location, const std::string &E, long long result) : location(location), E(E), result(result) {}
        Location location; // location of #if/#elif
        std::string E; // preprocessed condition
        long long result; // condition result
    };

    /**
     * Command line preprocessor settings.
     * On the command line these are configured by -D, -U, -I, --include, -std
     */
    struct SIMPLECPP_LIB DUI {
        DUI() = default;
        std::list<std::string> defines;
        std::set<std::string> undefined;
        std::list<std::string> includePaths;
        std::list<std::string> includes;
        std::string std;
        bool clearIncludeCache{};
        bool removeComments{}; /** remove comment tokens from included files */
    };

    struct SIMPLECPP_LIB FileData {
        /** The canonical filename associated with this data */
        std::string filename;
        /** The tokens associated with this file */
        TokenList tokens;
    };

    class SIMPLECPP_LIB FileDataCache {
    public:
        FileDataCache() = default;

        FileDataCache(const FileDataCache &) = delete;
        FileDataCache(FileDataCache &&) = default;

        FileDataCache &operator=(const FileDataCache &) = delete;
        FileDataCache &operator=(FileDataCache &&) = default;

        /** Get the cached data for a file, or load and then return it if it isn't cached.
         *  returns the file data and true if the file was loaded, false if it was cached. */
        std::pair<FileData *, bool> get(const std::string &sourcefile, const std::string &header, const DUI &dui, bool systemheader, std::vector<std::string> &filenames, OutputList *outputList);

        void insert(FileData data) {
            // NOLINTNEXTLINE(misc-const-correctness) - FP
            auto *const newdata = new FileData(std::move(data));

            mData.emplace_back(newdata);
            mNameMap.emplace(newdata->filename, newdata);
        }

        void clear() {
            mNameMap.clear();
            mIdMap.clear();
            mData.clear();
        }

        using container_type = std::vector<std::unique_ptr<FileData>>;
        using iterator = container_type::iterator;
        using const_iterator = container_type::const_iterator;
        using size_type = container_type::size_type;

        size_type size() const {
            return mData.size();
        }
        iterator begin() {
            return mData.begin();
        }
        iterator end() {
            return mData.end();
        }
        const_iterator begin() const {
            return mData.begin();
        }
        const_iterator end() const {
            return mData.end();
        }
        const_iterator cbegin() const {
            return mData.cbegin();
        }
        const_iterator cend() const {
            return mData.cend();
        }

    private:
        struct FileID {
#ifdef _WIN32
            struct {
                std::uint64_t VolumeSerialNumber;
                struct {
                    std::uint64_t IdentifierHi;
                    std::uint64_t IdentifierLo;
                } FileId;
            } fileIdInfo;

            bool operator==(const FileID &that) const noexcept {
                return fileIdInfo.VolumeSerialNumber == that.fileIdInfo.VolumeSerialNumber &&
                       fileIdInfo.FileId.IdentifierHi == that.fileIdInfo.FileId.IdentifierHi &&
                       fileIdInfo.FileId.IdentifierLo == that.fileIdInfo.FileId.IdentifierLo;
            }
#else
            dev_t dev;
            ino_t ino;

            bool operator==(const FileID& that) const noexcept {
                return dev == that.dev && ino == that.ino;
            }
#endif
            struct Hasher {
                std::size_t operator()(const FileID &id) const {
#ifdef _WIN32
                    return static_cast<std::size_t>(id.fileIdInfo.FileId.IdentifierHi ^ id.fileIdInfo.FileId.IdentifierLo ^
                                                    id.fileIdInfo.VolumeSerialNumber);
#else
                    return static_cast<std::size_t>(id.dev) ^ static_cast<std::size_t>(id.ino);
#endif
                }
            };
        };

        using name_map_type = std::unordered_map<std::string, FileData *>;
        using id_map_type = std::unordered_map<FileID, FileData *, FileID::Hasher>;

        static bool getFileId(const std::string &path, FileID &id);

        std::pair<FileData *, bool> tryload(name_map_type::iterator &name_it, const DUI &dui, std::vector<std::string> &filenames, OutputList *outputList);

        container_type mData;
        name_map_type mNameMap;
        id_map_type mIdMap;
    };

    /** Converts character literal (including prefix, but not ud-suffix) to long long value.
     *
     * Assumes ASCII-compatible single-byte encoded str for narrow literals
     * and UTF-8 otherwise.
     *
     * For target assumes
     * - execution character set encoding matching str
     * - UTF-32 execution wide-character set encoding
     * - requirements for __STDC_UTF_16__, __STDC_UTF_32__ and __STDC_ISO_10646__ satisfied
     * - char16_t is 16bit wide
     * - char32_t is 32bit wide
     * - wchar_t is 32bit wide and unsigned
     * - matching char signedness to host
     * - matching sizeof(int) to host
     *
     * For host assumes
     * - ASCII-compatible execution character set
     *
     * For host and target assumes
     * - CHAR_BIT == 8
     * - two's complement
     *
     * Implements multi-character narrow literals according to GCC's behavior,
     * except multi code unit universal character names are not supported.
     * Multi-character wide literals are not supported.
     * Limited support of universal character names for non-UTF-8 execution character set encodings.
     * @throws std::runtime_error thrown on invalid literal
     */
    SIMPLECPP_LIB long long characterLiteralToLL(const std::string& str);

    SIMPLECPP_LIB FileDataCache load(const TokenList &rawtokens, std::vector<std::string> &filenames, const DUI &dui, OutputList *outputList = nullptr, FileDataCache cache = {});

    /**
     * Preprocess
     * @todo simplify interface
     * @param output TokenList that receives the preprocessing output
     * @param rawtokens Raw tokenlist for top sourcefile
     * @param files internal data of simplecpp
     * @param cache output from simplecpp::load()
     * @param dui defines, undefs, and include paths
     * @param outputList output: list that will receive output messages
     * @param macroUsage output: macro usage
     * @param ifCond output: #if/#elif expressions
     */
    SIMPLECPP_LIB void preprocess(TokenList &output, const TokenList &rawtokens, std::vector<std::string> &files, FileDataCache &cache, const DUI &dui, OutputList *outputList = nullptr, std::list<MacroUsage> *macroUsage = nullptr, std::list<IfCond> *ifCond = nullptr);

    /**
     * Deallocate data
     */
    SIMPLECPP_LIB void cleanup(FileDataCache &cache);

    /** Simplify path */
    SIMPLECPP_LIB std::string simplifyPath(std::string path);

    /** Convert Cygwin path to Windows path */
    SIMPLECPP_LIB std::string convertCygwinToWindowsPath(const std::string &cygwinPath);

    /** Returns the C version a given standard */
    SIMPLECPP_LIB cstd_t getCStd(const std::string &std);

    /** Returns the C++ version a given standard */
    SIMPLECPP_LIB cppstd_t getCppStd(const std::string &std);

    /** Returns the __STDC_VERSION__ value for a given standard */
    SIMPLECPP_LIB std::string getCStdString(const std::string &std);
    SIMPLECPP_LIB std::string getCStdString(cstd_t std);

    /** Returns the __cplusplus value for a given standard */
    SIMPLECPP_LIB std::string getCppStdString(const std::string &std);
    SIMPLECPP_LIB std::string getCppStdString(cppstd_t std);

    /** Checks if given path is absolute */
    SIMPLECPP_LIB bool isAbsolutePath(const std::string &path);
}

#undef SIMPLECPP_TOKENLIST_ALLOW_PTR

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

#undef SIMPLECPP_LIB

#endif
