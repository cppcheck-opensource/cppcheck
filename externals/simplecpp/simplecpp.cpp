/*
 * simplecpp - A simple and high-fidelity C/C++ preprocessor library
 * Copyright (C) 2016-2023 simplecpp team
 */

// needs to be specified here otherwise _mingw.h will define it as 0x0601
// causing FileIdInfo not to be available
#if defined(_WIN32)
#  ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0602
#  endif
#endif

#include "simplecpp.h"

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#  define SIMPLECPP_WINDOWS
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <cstddef> // IWYU pragma: keep
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#ifdef SIMPLECPP_WINDOWS
#  include <mutex>
#endif
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(_WIN32)
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  undef ERROR
#  include <direct.h>
#else
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

static bool isHex(const std::string &s)
{
    return s.size()>2 && (s.compare(0,2,"0x")==0 || s.compare(0,2,"0X")==0);
}

static bool isOct(const std::string &s)
{
    return s.size()>1 && (s[0]=='0') && (s[1] >= '0') && (s[1] < '8');
}

static bool isStringLiteral(const std::string &s)
{
    return s.size() > 1 && (s[0]=='\"') && (*s.rbegin()=='\"');
}

static bool isCharLiteral(const std::string &s)
{
    // char literal patterns can include 'a', '\t', '\000', '\xff', 'abcd', and maybe ''
    // This only checks for the surrounding '' but doesn't parse the content.
    return s.size() > 1 && (s[0]=='\'') && (*s.rbegin()=='\'');
}

static const simplecpp::TokenString DEFINE("define");
static const simplecpp::TokenString UNDEF("undef");

static const simplecpp::TokenString INCLUDE("include");

static const simplecpp::TokenString ERROR("error");
static const simplecpp::TokenString WARNING("warning");

static const simplecpp::TokenString IF("if");
static const simplecpp::TokenString IFDEF("ifdef");
static const simplecpp::TokenString IFNDEF("ifndef");
static const simplecpp::TokenString DEFINED("defined");
static const simplecpp::TokenString ELSE("else");
static const simplecpp::TokenString ELIF("elif");
static const simplecpp::TokenString ELIFDEF("elifdef");
static const simplecpp::TokenString ELIFNDEF("elifndef");
static const simplecpp::TokenString ENDIF("endif");

static const simplecpp::TokenString PRAGMA("pragma");
static const simplecpp::TokenString ONCE("once");

static const simplecpp::TokenString HAS_INCLUDE("__has_include");

template<class T> static std::string toString(T t)
{
    // NOLINTNEXTLINE(misc-const-correctness) - false positive
    std::ostringstream ostr;
    ostr << t;
    return ostr.str();
}

#ifdef SIMPLECPP_DEBUG_MACRO_EXPANSION
static std::string locstring(const simplecpp::Location &loc)
{
    std::ostringstream ostr;
    ostr << '[' << loc.file() << ':' << loc.line << ':' << loc.col << ']';
    return ostr.str();
}
#endif

static long long stringToLL(const std::string &s)
{
    long long ret;
    const bool hex = isHex(s);
    const bool oct = isOct(s);
    std::istringstream istr(hex ? s.substr(2) : oct ? s.substr(1) : s);
    if (hex)
        istr >> std::hex;
    else if (oct)
        istr >> std::oct;
    istr >> ret;
    return ret;
}

static unsigned long long stringToULL(const std::string &s)
{
    unsigned long long ret;
    const bool hex = isHex(s);
    const bool oct = isOct(s);
    std::istringstream istr(hex ? s.substr(2) : oct ? s.substr(1) : s);
    if (hex)
        istr >> std::hex;
    else if (oct)
        istr >> std::oct;
    istr >> ret;
    return ret;
}

static bool endsWith(const std::string &s, const std::string &e)
{
    return (s.size() >= e.size()) && std::equal(e.rbegin(), e.rend(), s.rbegin());
}

static bool sameline(const simplecpp::Token *tok1, const simplecpp::Token *tok2)
{
    return tok1 && tok2 && tok1->location.sameline(tok2->location);
}

/** is s a punctuator (C11 6.4.6, C++ [lex.operators]) */
static bool isPunctuator(const std::string &s)
{
    static const std::set<std::string> punctuators{
        "[", "]", "(", ")", "{", "}", ".", "->", "++", "--", "&", "*", "+", "-", "~", "!",
        "/", "%", "<<", ">>", "<", ">", "<=", ">=", "==", "!=", "^", "|", "&&", "||",
        "?", ":", ";", "...", "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=",
        ",", "#", "##", "<:", ":>", "<%", "%>", "%:", "%:%:", "::", ".*", "->*", "<=>"
    };
    return punctuators.find(s) != punctuators.end();
}

/** is s a valid result when a punctuator is pasted with ## */
static bool isValidPunctuatorPaste(const std::string &s)
{
    // a pp-number can start with '.'
    return isPunctuator(s) || (s.size() >= 2 && s[0] == '.' && std::isdigit(static_cast<unsigned char>(s[1])));
}

/*
 * When an operation can't be evaluated (division by zero), its result is an
 * error value. The error is only reported if the error value is used, so an
 * operand that is not evaluated, as in "0 && (1/0)", does not cause an error.
 */
static const std::string ERROR_VALUE("#error:");

static bool isErrorValue(const simplecpp::Token *tok)
{
    return tok && tok->str().compare(0, ERROR_VALUE.size(), ERROR_VALUE) == 0;
}

static bool isValue(const simplecpp::Token *tok)
{
    return tok && (tok->number || isErrorValue(tok));
}

static bool isAlternativeBinaryOp(const simplecpp::Token *tok, const std::string &alt)
{
    return (tok->name &&
            tok->str() == alt &&
            tok->previous &&
            tok->next &&
            (isValue(tok->previous) || tok->previous->name || tok->previous->op == ')') &&
            (isValue(tok->next) || tok->next->name || tok->next->op == '('));
}

static bool isAlternativeOpName(const simplecpp::Token *tok)
{
    static const std::set<std::string> altop{"and","or","bitand","bitor","compl","not","not_eq","xor"};
    return tok->name && altop.find(tok->str()) != altop.end();
}

static bool isAlternativeUnaryOp(const simplecpp::Token *tok, const std::string &alt)
{
    if (!tok->name || tok->str() != alt)
        return false;
    // a unary operator comes first or after another operator, but not after an operand
    const simplecpp::Token * const prev = tok->previous;
    if (prev && (isValue(prev) || prev->op == ')' || (prev->name && !isAlternativeOpName(prev))))
        return false;
    // it is followed by the start of an operand
    const simplecpp::Token * const next = tok->next;
    return next && (next->name || isValue(next) || next->isOneOf("(!~+-"));
}

static std::string replaceAll(std::string s, const std::string& from, const std::string& to)
{
    for (size_t pos = s.find(from); pos != std::string::npos; pos = s.find(from, pos + to.size()))
        s.replace(pos, from.size(), to);
    return s;
}

void simplecpp::Location::adjust(const std::string &str)
{
    if (strpbrk(str.c_str(), "\r\n") == nullptr) {
        col += str.size();
        return;
    }

    for (std::size_t i = 0U; i < str.size(); ++i) {
        col++;
        if (str[i] == '\n' || str[i] == '\r') {
            col = 1;
            line++;
            if (str[i] == '\r' && (i+1)<str.size() && str[i+1]=='\n')
                ++i;
        }
    }
}

bool simplecpp::Token::isOneOf(const char ops[]) const
{
    return (op != '\0') && (std::strchr(ops, op) != nullptr);
}

bool simplecpp::Token::startsWithOneOf(const char c[]) const
{
    return std::strchr(c, string[0]) != nullptr;
}

bool simplecpp::Token::endsWithOneOf(const char c[]) const
{
    return std::strchr(c, string[string.size() - 1U]) != nullptr;
}

void simplecpp::Token::printAll() const
{
    const Token *tok = this;
    while (tok->previous)
        tok = tok->previous;
    for (; tok; tok = tok->next) {
        if (tok->previous) {
            std::cout << (sameline(tok, tok->previous) ? ' ' : '\n');
        }
        std::cout << tok->str();
    }
    std::cout << std::endl;
}

void simplecpp::Token::printOut() const
{
    for (const Token *tok = this; tok; tok = tok->next) {
        if (tok != this) {
            std::cout << (sameline(tok, tok->previous) ? ' ' : '\n');
        }
        std::cout << tok->str();
    }
    std::cout << std::endl;
}

class simplecpp::TokenList::Stream {
public:
    virtual ~Stream() = default;

    /**
     * Read the next character. CR and CR LF are returned as LF.
     * At the end of the input, EOF converted to unsigned char is returned
     * and good() becomes false.
     */
    unsigned char readChar() {
        int ch = getUnit();
        if (ch == EOF) {
            atEnd = true;
            lastChar = EOF;
            return static_cast<unsigned char>(EOF);
        }

        // Handling of newlines..
        if (ch == '\r') {
            ch = '\n';
            const int ch2 = getUnit();
            if (ch2 != '\n' && ch2 != EOF)
                buffer.push_back(static_cast<char>(ch2));
        }

        lastChar = ch;
        return static_cast<unsigned char>(ch);
    }

    /** The next character, without reading it. EOF at the end of the input. */
    int peekChar() {
        const int ch = getUnit();
        if (ch == EOF)
            return EOF;
        buffer.push_back(static_cast<char>(ch));
        return (ch == '\r') ? '\n' : ch;
    }

    /** Put back the character that was last read with readChar() */
    void ungetChar() {
        if (lastChar != EOF)
            buffer.push_back(static_cast<char>(lastChar));
        lastChar = EOF;
    }

    /**
     * false when readChar() has reached the end of the input.
     * Unlike std::istream::good(), peekChar() at the end of the input does not make it false.
     */
    bool good() const {
        return !atEnd;
    }

protected:
    /** Raw input, used only by the Stream class. Call readChar(), peekChar() and ungetChar() instead. */
    virtual int get() = 0;
    virtual int peek() = 0;
    virtual void unget() = 0;

    void init() {
        // initialize since we use peek() in getAndSkipBOM()
        isUtf16 = false;
        bom = getAndSkipBOM();
        isUtf16 = (bom == 0xfeff || bom == 0xfffe);
    }

private:
    /**
     * Read the next character from the input, or take it from the buffer.
     * Newlines are not converted.
     */
    int getUnit() {
        if (!buffer.empty()) {
            const auto ch = static_cast<unsigned char>(buffer.back());
            buffer.pop_back();
            return ch;
        }
        if (inputEnd)
            return EOF;
        const int ch = get();
        if (ch == EOF) {
            inputEnd = true;
            return EOF;
        }
        if (!isUtf16)
            return ch;

        // For UTF-16 encoded files the BOM is 0xfeff/0xfffe. If the
        // character is non-ASCII character then replace it with 0xff
        const int ch2 = get();
        if (ch2 == EOF) {
            // a dangling last byte is dropped
            inputEnd = true;
            return EOF;
        }
        const int ch16 = makeUtf16Char(static_cast<unsigned char>(ch), static_cast<unsigned char>(ch2));
        return (ch16 >= 0x80) ? 0xff : ch16;
    }

    inline int makeUtf16Char(const unsigned char ch, const unsigned char ch2) const {
        return (bom == 0xfeff) ? (ch<<8 | ch2) : (ch2<<8 | ch);
    }

    unsigned short getAndSkipBOM() {
        const int ch1 = peek();

        // The UTF-16 BOM is 0xfffe or 0xfeff.
        if (ch1 >= 0xfe) {
            (void)get();
            const unsigned short byte = (static_cast<unsigned char>(ch1) << 8);
            if (peek() >= 0xfe)
                return byte | static_cast<unsigned char>(get());
            unget();
            return 0;
        }

        // Skip UTF-8 BOM 0xefbbbf
        if (ch1 == 0xef) {
            (void)get();
            if (peek() == 0xbb) {
                (void)get();
                if (peek() == 0xbf) {
                    (void)get();
                    return 0;
                }
                unget();
            }
            unget();
        }

        return 0;
    }

    unsigned short bom{0};

    /** characters that are read but not used yet. The next character is at the back. */
    std::string buffer;
    /** the last character returned by readChar(), EOF if ungetChar() has nothing to put back */
    int lastChar{EOF};
    /** readChar() has returned EOF */
    bool atEnd{false};
    /** get() has returned EOF */
    bool inputEnd{false};
protected:
    bool isUtf16{false};
};

namespace {
    class StdIStream final : public simplecpp::TokenList::Stream {
    public:
        explicit StdIStream(std::istream &istr)
            : istr(istr) {
            assert(istr.good());
            init();
        }

    protected:
        int get() override {
            return istr.get();
        }
        int peek() override {
            return istr.peek();
        }
        void unget() override {
            istr.unget();
        }

    private:
        std::istream &istr;
    };

    class StdCharBufStream final : public simplecpp::TokenList::Stream {
    public:
        StdCharBufStream(const unsigned char* str, std::size_t size)
            : str(str)
            , size(size)
        {
            init();
        }

    protected:
        int get() override {
            if (pos >= size)
                return EOF;
            return str[pos++];
        }
        int peek() override {
            if (pos >= size)
                return EOF;
            return str[pos];
        }
        void unget() override {
            --pos;
        }

    private:
        const unsigned char *str;
        const std::size_t size;
        std::size_t pos{};
    };

    class FileStream final : public simplecpp::TokenList::Stream {
    public:
        /**
         * @throws simplecpp::Output thrown if file is not found
         */
        explicit FileStream(const std::string &filename, std::vector<std::string> &files)
            : file(fopen(filename.c_str(), "rb"))
        {
            if (!file) {
                files.emplace_back(filename);
                throw simplecpp::Output(simplecpp::Output::FILE_NOT_FOUND, {}, "File is missing: " + filename);
            }
            init();
        }

        FileStream(const FileStream&) = delete;
        FileStream &operator=(const FileStream&) = delete;

        ~FileStream() override {
            fclose(file);
            file = nullptr;
        }

    protected:
        int get() override {
            lastCh = fgetc(file);
            return lastCh;
        }
        int peek() override {
            // keep lastCh intact
            const int ch = fgetc(file);
            unget_internal(ch);
            return ch;
        }
        void unget() override {
            unget_internal(lastCh);
        }

    private:
        void unget_internal(int ch) {
            if (isUtf16) {
                // TODO: use ungetc() as well
                // UTF-16 has subsequent unget() calls
                fseek(file, -1, SEEK_CUR);
            } else {
                ungetc(ch, file);
            }
        }

        FILE *file;
        int lastCh{};
    };
}

simplecpp::TokenList::TokenList(std::vector<std::string> &filenames) : frontToken(nullptr), backToken(nullptr), files(filenames) {}

simplecpp::TokenList::TokenList(std::istream &istr, std::vector<std::string> &filenames, const std::string &filename, const DUI &dui, OutputList *outputList)
    : frontToken(nullptr), backToken(nullptr), files(filenames)
{
    StdIStream stream(istr);
    readfile(stream,filename,dui,outputList);
}

simplecpp::TokenList::TokenList(const unsigned char* data, std::size_t size, std::vector<std::string> &filenames, const std::string &filename, const DUI &dui, OutputList *outputList, int /*unused*/)
    : frontToken(nullptr), backToken(nullptr), files(filenames)
{
    StdCharBufStream stream(data, size);
    readfile(stream,filename,dui,outputList);
}

simplecpp::TokenList::TokenList(const std::string &filename, std::vector<std::string> &filenames, const DUI &dui, OutputList *outputList)
    : frontToken(nullptr), backToken(nullptr), files(filenames)
{
    try {
        FileStream stream(filename, filenames);
        readfile(stream,filename,dui,outputList);
    } catch (const simplecpp::Output & e) {
        outputList->emplace_back(e);
    }
}

simplecpp::TokenList::TokenList(const TokenList &other) : frontToken(nullptr), backToken(nullptr), files(other.files)
{
    *this = other;
}

simplecpp::TokenList::TokenList(TokenList &&other) : frontToken(nullptr), backToken(nullptr), files(other.files)
{
    *this = std::move(other);
}

simplecpp::TokenList::~TokenList()
{
    clear();
}

simplecpp::TokenList &simplecpp::TokenList::operator=(const TokenList &other)
{
    if (this != &other) {
        clear();
        files = other.files;
        for (const Token *tok = other.cfront(); tok; tok = tok->next)
            push_back(new Token(*tok));
        sizeOfType = other.sizeOfType;
    }
    return *this;
}

simplecpp::TokenList &simplecpp::TokenList::operator=(TokenList &&other)
{
    if (this != &other) {
        clear();
        frontToken = other.frontToken;
        other.frontToken = nullptr;
        backToken = other.backToken;
        other.backToken = nullptr;
        files = other.files;
        sizeOfType = std::move(other.sizeOfType);
    }
    return *this;
}

void simplecpp::TokenList::clear()
{
    backToken = nullptr;
    while (frontToken) {
        Token * const next = frontToken->next;
        delete frontToken;
        frontToken = next;
    }
    sizeOfType.clear();
}

void simplecpp::TokenList::push_back(Token *tok)
{
    if (!frontToken)
        frontToken = tok;
    else
        backToken->next = tok;
    tok->previous = backToken;
    backToken = tok;
}

void simplecpp::TokenList::dump(bool linenrs) const
{
    std::cout << stringify(linenrs) << std::endl;
}

std::string simplecpp::TokenList::stringify(bool linenrs) const
{
    std::ostringstream ret;
    Location loc;
    loc.line = 1;
    bool filechg = true;
    for (const Token *tok = cfront(); tok; tok = tok->next) {
        if (tok->location.line < loc.line || tok->location.fileIndex != loc.fileIndex) {
            ret << "\n#line " << tok->location.line << " \"" << file(tok->location) << "\"\n";
            loc = tok->location;
            filechg = true;
        }

        if (linenrs && filechg) {
            ret << loc.line << ": ";
            filechg = false;
        }

        while (tok->location.line > loc.line) {
            ret << '\n';
            loc.line++;
            if (linenrs)
                ret << loc.line << ": ";
        }

        if (sameline(tok->previous, tok))
            ret << ' ';

        ret << tok->str();

        loc.adjust(tok->str());
    }

    return ret.str();
}

static bool isNameChar(int ch)
{
    return std::isalnum(ch) || ch == '_' || ch == '$';
}

static std::string escapeString(const std::string &str)
{
    std::ostringstream ostr;
    ostr << '\"';
    for (std::size_t i = 1U; i < str.size() - 1; ++i) {
        const char c = str[i];
        if (c == '\\' || c == '\"' || c == '\'')
            ostr << '\\';
        ostr << c;
    }
    ostr << '\"';
    return ostr.str();
}

static void portabilityBackslash(simplecpp::OutputList *outputList, const simplecpp::Location &location)
{
    if (!outputList)
        return;
    simplecpp::Output err{
        simplecpp::Output::PORTABILITY_BACKSLASH,
        location,
        "Combination 'backslash space newline' is not portable."
    };
    outputList->emplace_back(std::move(err));
}

static bool isStringLiteralPrefix(const std::string &str)
{
    return str == "u" || str == "U" || str == "L" || str == "u8" ||
           str == "R" || str == "uR" || str == "UR" || str == "LR" || str == "u8R";
}

/** Spelling of a token in the result of the # operator: a \ is inserted
 *  before each " and \ of a string literal or character constant (C 6.10.3.2p2) */
static std::string stringifyToken(const std::string &str)
{
    const std::string::size_type quote = str.find_first_of("\"\'");
    if (quote == std::string::npos || str.back() != str[quote] ||
        (quote > 0 && !isStringLiteralPrefix(str.substr(0, quote))))
        return str;
    std::string ret;
    for (const char c : str) {
        if (c == '\\' || c == '\"')
            ret += '\\';
        ret += c;
    }
    return ret;
}

/**
 * The result of the # operator: the tokens as a string literal (C 6.10.3.2).
 * The tokens are separated by a space where there was whitespace between them.
 * @param tokens     the argument of #, not macro expanded
 * @param loc        location for the warning
 * @param macroName  the macro that is expanded, for the warning
 * @param outputList if not null, a warning is added when a final backslash is dropped
 * @return the string literal, including the quotes
 */
static std::string stringify(const simplecpp::TokenList &tokens, const simplecpp::Location &loc, const std::string &macroName, simplecpp::OutputList *outputList)
{
    std::string str;
    for (const simplecpp::Token *tok = tokens.cfront(), *next; tok; tok = next) {
        next = tok->next;
        str += stringifyToken(tok->str());
        if (next && tok->whitespaceahead)
            str += ' ';
    }
    // An odd number of trailing backslashes would escape the closing quote and
    // yield an unterminated string literal (undefined behavior, C 6.10.3.2p2).
    // Drop the final backslash, as gcc and clang do.
    std::size_t trailing = 0;
    while (trailing < str.size() && str[str.size() - 1U - trailing] == '\\')
        ++trailing;
    if (trailing % 2U == 1U) {
        str.pop_back();
        if (outputList) {
            outputList->emplace_back(simplecpp::Output::PORTABILITY_STRINGIFY, loc,
                                     "Invalid string literal when expanding \'" + macroName + "\', ignoring final \'\\\'. "
                                     "Undefined behavior according to C standard chapter 6.10.3.2, paragraph 2.");
        }
    }
    return '\"' + str + '\"';
}

void simplecpp::TokenList::lineDirective(unsigned int fileIndex_, unsigned int line, Location &location)
{
    if (fileIndex_ != location.fileIndex || line >= location.line) {
        location.fileIndex = fileIndex_;
        location.line = line;
        return;
    }

    if (line + 2 >= location.line) {
        location.line = line;
        while (cback()->op != '#')
            deleteToken(back());
        deleteToken(back());
        return;
    }
}

static const std::string COMMENT_END("*/");

/**
 * Read a name or a number.
 * In a number, a ' digit separator is removed, so 1'000 gives "1000".
 * The stream is left after the last character of the token.
 * @param stream the stream to read from
 * @param ch     the first character, already read from the stream
 * @return the name or number
 */
static simplecpp::TokenString readNameOrNumber(simplecpp::TokenList::Stream &stream, unsigned char ch)
{
    simplecpp::TokenString currentToken;
    const bool num = !!std::isdigit(ch);
    while (stream.good() && isNameChar(ch)) {
        currentToken += ch;
        ch = stream.readChar();
        if (num && ch=='\'' && isNameChar(stream.peekChar()))
            ch = stream.readChar();
    }
    stream.ungetChar();
    return currentToken;
}

/**
 * Read a // comment, up to but not including the newline.
 * A backslash followed by a newline continues the comment on the next line.
 * The backslash, and any spaces or tabs after it, are then replaced by a
 * newline in the comment text.
 * @param stream     the stream to read from
 * @param ch         the first '/', already read from the stream
 * @param location   the location of the comment, used for warnings
 * @param multiline  incremented for each continued line
 * @param outputList if not null, a portability warning is added when there is
 *                   whitespace between the backslash and the newline
 * @return the comment text, including the leading //
 */
static simplecpp::TokenString readLineComment(simplecpp::TokenList::Stream &stream, unsigned char ch, const simplecpp::Location &location, unsigned int &multiline, simplecpp::OutputList *outputList)
{
    simplecpp::TokenString currentToken;
    while (stream.good() && ch != '\n') {
        currentToken += ch;
        ch = stream.readChar();
        if (ch == '\\') {
            simplecpp::TokenString tmp;
            char tmp_ch = ch;
            while ((stream.good()) && (tmp_ch == '\\' || tmp_ch == ' ' || tmp_ch == '\t')) {
                tmp += tmp_ch;
                tmp_ch = stream.readChar();
            }
            if (!stream.good()) {
                break;
            }

            if (tmp_ch != '\n') {
                currentToken += tmp;
            } else {
                const simplecpp::TokenString check_portability = currentToken + tmp;
                const std::string::size_type pos = check_portability.find_last_not_of(" \t");
                if (pos < check_portability.size() - 1U && check_portability[pos] == '\\')
                    portabilityBackslash(outputList, location);
                ++multiline;
                tmp_ch = stream.readChar();
                currentToken += '\n';
            }
            ch = tmp_ch;
        }
    }
    if (ch == '\n') {
        stream.ungetChar();
    }
    return currentToken;
}

simplecpp::TokenString simplecpp::TokenList::readBlockComment(Stream &stream, unsigned int &multiline) const
{
    TokenString currentToken = "/*";
    (void)stream.readChar();
    unsigned char ch = stream.readChar();
    while (stream.good()) {
        currentToken += ch;
        if (currentToken.size() >= 4U && endsWith(currentToken, COMMENT_END))
            break;
        ch = stream.readChar();
    }
    // multiline..

    std::string::size_type pos = 0;
    while ((pos = currentToken.find("\\\n",pos)) != std::string::npos) {
        currentToken.erase(pos,2);
        ++multiline;
    }
    if (multiline || isLastLinePreprocessor()) {
        pos = 0;
        while ((pos = currentToken.find('\n',pos)) != std::string::npos) {
            currentToken.erase(pos,1);
            ++multiline;
        }
    }
    return currentToken;
}

void simplecpp::TokenList::readErrorText(Stream &stream, unsigned char ch, Location &location)
{
    TokenString currentToken;
    char prev = ' ';
    while (stream.good() && (prev == '\\' || (ch != '\r' && ch != '\n'))) {
        currentToken += ch;
        prev = ch;
        ch = stream.readChar();
    }
    stream.ungetChar();
    std::string::size_type pos = 0;
    unsigned int spliced = 0;
    while ((pos = currentToken.find('\\', pos)) != std::string::npos) {
        if (pos + 1 < currentToken.size() && currentToken[pos + 1] == '\n') {
            currentToken.erase(pos, 2);
            ++spliced;
        } else {
            ++pos;
        }
    }
    if (!currentToken.empty()) {
        push_back(new Token(currentToken, location));
        location.adjust(currentToken);
    }
    location.line += spliced;
}

void simplecpp::TokenList::readfile(Stream &stream, const std::string &filename, const DUI &dui, OutputList *outputList)
{
    unsigned int multiline = 0U;
    bool trailing_nl = true;

    const Token *oldLastToken = nullptr;

    const cstd_t cstd = getCStd(dui.std);
    const cppstd_t cppstd = getCppStd(dui.std);

    Location location(fileIndex(filename), 1, 1);
    while (stream.good()) {
        unsigned char ch = stream.readChar();
        if (!stream.good())
            break;

        trailing_nl = false;

        if (ch >= 0x80) {
            if (outputList) {
                simplecpp::Output err{
                    simplecpp::Output::UNHANDLED_CHAR_ERROR,
                    location,
                    "The code contains unhandled character(s) (character code=" + std::to_string(static_cast<int>(ch)) + "). Neither unicode nor extended ascii is supported."
                };
                outputList->emplace_back(std::move(err));
            }
            clear();
            return;
        }

        if (ch == '\n') {
            if (cback() && cback()->op == '\\') {
                if (location.col > cback()->location.col + 1U)
                    portabilityBackslash(outputList, cback()->location);
                ++multiline;
                deleteToken(back());
            } else {
                location.line += multiline + 1;
                multiline = 0U;
                trailing_nl = true;
            }
            if (!multiline)
                location.col = 1;

            if (oldLastToken != cback()) {
                oldLastToken = cback();

                // #line 3
                // #line 3 "file.c"
                // #3
                // #3 "file.c"
                const Token * ppTok = isLastLinePreprocessor();
                if (!ppTok)
                    continue;

                const auto advanceAndSkipComments = [](const Token* tok) {
                    do {
                        tok = tok->next;
                    } while (tok && tok->comment);
                    return tok;
                };

                // skip #
                ppTok = advanceAndSkipComments(ppTok);
                if (!ppTok)
                    continue;

                if (ppTok->str() == "line")
                    ppTok = advanceAndSkipComments(ppTok);

                if (ppTok && (ppTok->str()[0] == '-' || ppTok->str()[0] == '+')) {
                    if (outputList) {
                        simplecpp::Output err{
                            simplecpp::Output::SYNTAX_ERROR,
                            location,
                            "Invalid character in line directive: '" + ppTok->str() + "'."
                        };
                        outputList->emplace_back(std::move(err));
                    }
                    clear();
                    return;
                }

                if (!ppTok || !ppTok->number)
                    continue;

                constexpr unsigned long line_limit = std::numeric_limits<decltype(Location::line)>::max();
                unsigned long line;
                try {
                    line = std::min(line_limit, std::stoul(ppTok->str()));
                } catch (...) {
                    line = line_limit;
                }

                unsigned long maxline;
                if ((cstd != CUnknown && cstd < C99) || (cppstd != CPPUnknown && cppstd < CPP11))
                    maxline = 32767;
                else
                    maxline = 2147483647;

                if (line == 0 || line > maxline) {
                    if (outputList) {
                        const bool unknown_std = cstd == CUnknown && cppstd == CPPUnknown;
                        std::string msg = "Line number out of range: " + ppTok->str() + ". ";
                        if (line == 0) {
                            msg += "Line number zero is undefined behavior.";
                        } else {
                            msg += "Line numbers above " + std::to_string(maxline) + " are ";
                            if (unknown_std)
                                msg += "undefined behavior or conditionally supported";
                            else if (cppstd >= CPP26)
                                msg += "conditionally supported";
                            else
                                msg += "undefined behavior";
                            if (cstd != CUnknown)
                                msg += std::string(" in ") + getCStdName(cstd);
                            else if (cppstd != CPPUnknown)
                                msg += std::string(" in ") + getCppStdName(cppstd);
                            msg += ".";
                        }
                        simplecpp::Output err{
                            simplecpp::Output::PORTABILITY_LINE_DIRECTIVE,
                            location, msg
                        };
                        outputList->emplace_back(std::move(err));
                    }
                }

                ppTok = advanceAndSkipComments(ppTok);

                unsigned int fileindex;

                if (ppTok && ppTok->str()[0] == '\"')
                    fileindex = fileIndex(replaceAll(ppTok->str().substr(1U, ppTok->str().size() - 2U),"\\\\","\\"));
                else
                    fileindex = location.fileIndex;

                lineDirective(fileindex, line, location);
            }

            continue;
        }

        if (ch <= ' ') {
            location.col++;
            continue;
        }

        TokenString currentToken;

        if (cback() && cback()->location.line == location.line && cback()->previous && cback()->previous->op == '#') {
            const Token* const ppTok = cback()->previous;
            if (ppTok->next && (ppTok->next->str() == "error" || ppTok->next->str() == "warning")) {
                readErrorText(stream, ch, location);
                continue;
            }
        }

        // number or name
        if (isNameChar(ch)) {
            currentToken = readNameOrNumber(stream, ch);
        }

        // comment
        else if (ch == '/' && stream.peekChar() == '/') {
            currentToken = readLineComment(stream, ch, location, multiline, outputList);
        }

        // comment
        else if (ch == '/' && stream.peekChar() == '*') {
            currentToken = readBlockComment(stream, multiline);
        }

        // string / char literal
        else if (ch == '\"' || ch == '\'') {
            std::string prefix;
            if (cback() && cback()->name && isStringLiteralPrefix(cback()->str()) &&
                ((cback()->location.col + cback()->str().size()) == location.col) &&
                (cback()->location.line == location.line)) {
                prefix = cback()->str();
            }
            // C++11 raw string literal
            if (ch == '\"' && !prefix.empty() && *cback()->str().rbegin() == 'R') {
                std::string delim;
                currentToken = ch;
                prefix.resize(prefix.size() - 1);
                ch = stream.readChar();
                while (stream.good() && ch != '(' && ch != '\n') {
                    delim += ch;
                    ch = stream.readChar();
                }
                if (!stream.good() || ch == '\n') {
                    if (outputList) {
                        Output err{
                            Output::SYNTAX_ERROR,
                            location,
                            "Invalid newline in raw string delimiter."
                        };
                        outputList->emplace_back(std::move(err));
                    }
                    return;
                }
                const std::string endOfRawString(')' + delim + currentToken);
                while (stream.good() && (!endsWith(currentToken, endOfRawString) || currentToken.size() <= 1))
                    currentToken += stream.readChar();
                if (!endsWith(currentToken, endOfRawString)) {
                    if (outputList) {
                        Output err{
                            Output::SYNTAX_ERROR,
                            location,
                            "Raw string missing terminating delimiter."
                        };
                        outputList->emplace_back(std::move(err));
                    }
                    return;
                }
                currentToken.erase(currentToken.size() - endOfRawString.size(), endOfRawString.size() - 1U);
                currentToken = escapeString(currentToken);
                currentToken.insert(0, prefix);
                back()->setstr(currentToken);
                location.adjust(currentToken);
                if (currentToken.find_first_of("\r\n") == std::string::npos)
                    location.col += 2 + (2 * delim.size());
                else
                    location.col += 1 + delim.size();

                continue;
            }

            currentToken = readUntil(stream,location,ch,ch,outputList);
            if (currentToken.size() < 2U)
                // Error is reported by readUntil()
                return;

            std::string s = currentToken;
            std::string::size_type pos;
            int newlines = 0;
            while ((pos = s.find_first_of("\r\n")) != std::string::npos) {
                s.erase(pos,1);
                newlines++;
            }

            if (prefix.empty())
                push_back(new Token(s, location, !!std::isspace(stream.peekChar()))); // push string without newlines
            else
                back()->setstr(prefix + s);

            if (newlines > 0) {
                const Token * const llTok = lastLineTok();
                if (llTok && llTok->op == '#' && llTok->next && (llTok->next->str() == "define" || llTok->next->str() == "pragma") && llTok->next->next) {
                    multiline += newlines;
                    location.adjust(s);
                    continue;
                }
            }

            location.adjust(currentToken);
            continue;
        }

        else {
            currentToken += ch;
        }

        if (*currentToken.begin() == '<') {
            const Token * const llTok = lastLineTok();
            if (llTok && llTok->op == '#' && llTok->next && llTok->next->str() == "include") {
                currentToken = readUntil(stream, location, '<', '>', outputList);
                if (currentToken.size() < 2U)
                    return;
            }
        }

        push_back(new Token(currentToken, location, !!std::isspace(stream.peekChar())));

        if (multiline)
            location.col += currentToken.size();
        else
            location.adjust(currentToken);
    }

    if ((cstd != CUnknown || cppstd == CPPUnknown) && !trailing_nl && outputList) {
        Output err{
            Output::PORTABILITY_NO_EOF_NEWLINE,
            location,
            "No newline at end of file is undefined behavior in C."
        };
        outputList->emplace_back(std::move(err));
    }

    combineOperators();
}

/**
 * If one operand of the binary operator tok is an error value then
 * "lhs op rhs" is replaced by that error value.
 * @return the error value token, or nullptr
 */
static simplecpp::Token *foldErrorValue(simplecpp::TokenList &tokens, simplecpp::Token *tok)
{
    simplecpp::Token * const lhs = tok->previous;
    if (!isErrorValue(lhs) && !isErrorValue(tok->next))
        return nullptr;
    if (!isErrorValue(lhs))
        lhs->setstr(tok->next->str());
    tokens.deleteToken(tok->next);
    tokens.deleteToken(tok);
    return lhs;
}

void simplecpp::TokenList::constFold()
{
    while (cfront()) {
        // goto last '('
        Token *tok = back();
        while (tok && tok->op != '(')
            tok = tok->previous;

        // no '(', goto first token
        if (!tok)
            tok = front();

        // Constant fold expression
        constFoldUnaryNotPosNeg(tok);
        constFoldMulDivRem(tok);
        constFoldAddSub(tok);
        constFoldShift(tok);
        constFoldComparison(tok);
        constFoldBitwise(tok);
        constFoldLogicalOp(tok);
        constFoldQuestionOp(tok);

        // If there is no '(' we are done with the constant folding
        if (tok->op != '(')
            break;

        if (!tok->next || !tok->next->next || tok->next->next->op != ')')
            break;

        tok = tok->next;
        deleteToken(tok->previous);
        deleteToken(tok->next);
    }

    for (const Token *tok = cfront(); tok; tok = tok->next) {
        if (isErrorValue(tok))
            throw std::overflow_error(tok->str().substr(ERROR_VALUE.size()));
    }
}

static bool isFloatSuffix(const simplecpp::Token *tok)
{
    if (!tok || tok->str().size() != 1U)
        return false;
    const char c = std::tolower(tok->str()[0]);
    return c == 'f' || c == 'l';
}

static const std::string AND("and");
static const std::string BITAND("bitand");
static const std::string BITOR("bitor");
static bool isAlternativeAndBitandBitor(const simplecpp::Token* tok)
{
    return isAlternativeBinaryOp(tok, AND) || isAlternativeBinaryOp(tok, BITAND) || isAlternativeBinaryOp(tok, BITOR);
}

/**
 * Keep track of whether the code is in a function body. This is not part of
 * preprocessing, it is used by the cppcheck-specific handling of &= in
 * combineOperators().
 * @param executableScope one entry for each open {, true if it is a function
 *                        body or a block in a function body
 * @param tok             the current token
 * @return true if tok is { or }
 */
static bool updateExecutableScope(std::stack<bool, std::vector<bool>> &executableScope, const simplecpp::Token *tok)
{
    if (tok->op == '{') {
        if (executableScope.top()) {
            executableScope.push(true);
            return true;
        }
        const simplecpp::Token *prev = tok->previous;
        while (prev && prev->isOneOf(";{}("))
            prev = prev->previous;
        executableScope.push(prev && prev->op == ')');
        return true;
    }
    if (tok->op == '}') {
        if (executableScope.size() > 1)
            executableScope.pop();
        return true;
    }
    return false;
}

/**
 * Is tok the & of an unnamed reference parameter with a default value in a
 * function declaration, as in "void f(x&=2)"? Then "&" and "=" should not be
 * combined into "&=". This is a cppcheck-specific heuristic, compilers
 * tokenize "&=" as one operator.
 * @param tok a & token that is directly followed by =
 */
static bool isAnonymousReferenceParameter(const simplecpp::Token *tok)
{
    int indentlevel = 0;
    const simplecpp::Token *start = tok;
    while (indentlevel >= 0 && start) {
        if (start->op == ')')
            ++indentlevel;
        else if (start->op == '(')
            --indentlevel;
        else if (start->isOneOf(";{}"))
            break;
        start = start->previous;
    }
    if (indentlevel == -1 && start) {
        const simplecpp::Token * const ftok = start;
        bool isFuncDecl = ftok->name;
        while (isFuncDecl) {
            if (!start->name && start->str() != "::" && start->op != '*' && start->op != '&')
                isFuncDecl = false;
            if (!start->previous)
                break;
            if (start->previous->isOneOf(";{}:"))
                break;
            start = start->previous;
        }
        isFuncDecl &= start != ftok && start->name;
        if (isFuncDecl) {
            // TODO: we could loop through the parameters here and check if they are correct.
            return true;
        }
    }
    return false;
}

void simplecpp::TokenList::combineOperators()
{
    std::stack<bool, std::vector<bool>> executableScope{{false}};
    for (Token *tok = front(); tok; tok = tok->next) {
        if (updateExecutableScope(executableScope, tok))
            continue;

        if (tok->op == '.') {
            // ellipsis ...
            if (tok->next && tok->next->op == '.' && tok->next->location.col == (tok->location.col + 1) &&
                tok->next->next && tok->next->next->op == '.' && tok->next->next->location.col == (tok->location.col + 2)) {
                tok->setstr("...");
                tok->whitespaceahead = tok->next->whitespaceahead;
                deleteToken(tok->next);
                tok->whitespaceahead = tok->next->whitespaceahead;
                deleteToken(tok->next);
                continue;
            }
            // float literals..
            if (tok->previous && tok->previous->number && sameline(tok->previous, tok) && tok->previous->str().find_first_of("._") == std::string::npos) {
                tok->setstr(tok->previous->str() + '.');
                deleteToken(tok->previous);
                if (sameline(tok, tok->next) && (isFloatSuffix(tok->next) || (tok->next && tok->next->startsWithOneOf("AaBbCcDdEeFfPp") && !isAlternativeAndBitandBitor(tok->next)))) {
                    tok->setstr(tok->str() + tok->next->str());
                    tok->whitespaceahead = tok->next->whitespaceahead;
                    deleteToken(tok->next);
                }
            }
            if (tok->next && tok->next->number) {
                tok->setstr(tok->str() + tok->next->str());
                tok->whitespaceahead = tok->next->whitespaceahead;
                deleteToken(tok->next);
            }
        }
        // match: [0-9.]+E [+-] [0-9]+
        const char lastChar = tok->str()[tok->str().size() - 1];
        if (tok->number && !isOct(tok->str()) &&
            ((!isHex(tok->str()) && (lastChar == 'E' || lastChar == 'e')) ||
             (isHex(tok->str()) && (lastChar == 'P' || lastChar == 'p'))) &&
            tok->next && tok->next->isOneOf("+-") && tok->next->next && tok->next->next->number) {
            tok->setstr(tok->str() + tok->next->op + tok->next->next->str());
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
        }

        if (tok->op == '\0' || !tok->next || tok->next->op == '\0')
            continue;
        if (!sameline(tok,tok->next))
            continue;
        if (tok->location.col + 1U != tok->next->location.col)
            continue;

        if (tok->next->op == '=' && tok->isOneOf("=!<>+-*/%&|^")) {
            // don't combine &= if it is a anonymous reference parameter with default value:
            // void f(x&=2)
            if (tok->op == '&' && !executableScope.top() && isAnonymousReferenceParameter(tok))
                continue;
            tok->setstr(tok->str() + "=");
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
        } else if ((tok->op == '|' || tok->op == '&') && tok->op == tok->next->op) {
            tok->setstr(tok->str() + tok->next->str());
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
        } else if (tok->op == ':' && tok->next->op == ':') {
            tok->setstr(tok->str() + tok->next->str());
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
        } else if (tok->op == '-' && tok->next->op == '>') {
            tok->setstr(tok->str() + tok->next->str());
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
        } else if ((tok->op == '<' || tok->op == '>') && tok->op == tok->next->op) {
            tok->setstr(tok->str() + tok->next->str());
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
            if (tok->next && tok->next->op == '=' && tok->next->next && tok->next->next->op != '=') {
                tok->setstr(tok->str() + tok->next->str());
                tok->whitespaceahead = tok->next->whitespaceahead;
                deleteToken(tok->next);
            }
        } else if ((tok->op == '+' || tok->op == '-') && tok->op == tok->next->op) {
            if (tok->location.col + 1U != tok->next->location.col)
                continue;
            if (tok->previous && tok->previous->number)
                continue;
            if (tok->next->next && tok->next->next->number)
                continue;
            tok->setstr(tok->str() + tok->next->str());
            tok->whitespaceahead = tok->next->whitespaceahead;
            deleteToken(tok->next);
        }
    }
}

static const std::string COMPL("compl");
static const std::string NOT("not");
void simplecpp::TokenList::constFoldUnaryNotPosNeg(simplecpp::Token *tok1)
{
    if (!tok1 || tok1->op == ')')
        return;
    Token *last = tok1;
    while (last->next && last->next->op != ')')
        last = last->next;

    // unary operators are right-associative, "!-0" means "!(-0)", so they are folded from right to left
    for (Token *tok = last; tok; tok = (tok == tok1) ? nullptr : tok->previous) {
        // "not" might be !
        if (isAlternativeUnaryOp(tok, NOT))
            tok->op = '!';
        // "compl" might be ~
        else if (isAlternativeUnaryOp(tok, COMPL))
            tok->op = '~';

        if (tok->isOneOf("!~") && isErrorValue(tok->next)) {
            tok->setstr(tok->next->str());
            deleteToken(tok->next);
        } else if (tok->op == '!' && tok->next && tok->next->number) {
            tok->setstr(stringToLL(tok->next->str()) == 0 ? "1" : "0");
            deleteToken(tok->next);
        } else if (tok->op == '~' && tok->next && tok->next->number) {
            tok->setstr(toString(~stringToLL(tok->next->str())));
            deleteToken(tok->next);
        } else {
            if (tok->previous && (isValue(tok->previous) || (tok->previous->name && !isAlternativeOpName(tok->previous))))
                continue;
            if (!isValue(tok->next))
                continue;
            if (isErrorValue(tok->next)) {
                if (tok->isOneOf("+-")) {
                    tok->setstr(tok->next->str());
                    deleteToken(tok->next);
                }
                continue;
            }
            switch (tok->op) {
            case '+':
                tok->setstr(tok->next->str());
                deleteToken(tok->next);
                break;
            case '-':
                if (tok->next->str()[0] == '-')
                    tok->setstr(tok->next->str().substr(1));
                else
                    tok->setstr(tok->op + tok->next->str());
                deleteToken(tok->next);
                break;
            }
        }
    }
}

void simplecpp::TokenList::constFoldMulDivRem(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (!isValue(tok->previous) || !isValue(tok->next))
            continue;
        if (!tok->isOneOf("*/%"))
            continue;
        if (Token * const errorValue = foldErrorValue(*this, tok)) {
            tok = errorValue;
            continue;
        }

        std::string result;
        if (tok->op == '*') {
            result = toString(stringToLL(tok->previous->str()) * stringToLL(tok->next->str()));
        } else {
            const long long rhs = stringToLL(tok->next->str());
            const long long lhs = stringToLL(tok->previous->str());
            if (rhs == 0)
                result = ERROR_VALUE + "division/modulo by zero";
            else if (rhs == -1 && lhs == std::numeric_limits<long long>::min())
                result = ERROR_VALUE + "division overflow";
            else if (tok->op == '/')
                result = toString(lhs / rhs);
            else
                result = toString(lhs % rhs);
        }

        tok = tok->previous;
        tok->setstr(result);
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

void simplecpp::TokenList::constFoldAddSub(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (!isValue(tok->previous) || !isValue(tok->next))
            continue;
        if (!tok->isOneOf("+-"))
            continue;
        if (Token * const errorValue = foldErrorValue(*this, tok)) {
            tok = errorValue;
            continue;
        }

        long long result;
        if (tok->op == '+')
            result = stringToLL(tok->previous->str()) + stringToLL(tok->next->str());
        else if (tok->op == '-')
            result = stringToLL(tok->previous->str()) - stringToLL(tok->next->str());
        else
            continue;

        tok = tok->previous;
        tok->setstr(toString(result));
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

void simplecpp::TokenList::constFoldShift(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (!isValue(tok->previous) || !isValue(tok->next))
            continue;
        if (tok->str() != "<<" && tok->str() != ">>")
            continue;
        if (Token * const errorValue = foldErrorValue(*this, tok)) {
            tok = errorValue;
            continue;
        }

        long long result;
        if (tok->str() == "<<")
            result = stringToLL(tok->previous->str()) << stringToLL(tok->next->str());
        else if (tok->str() == ">>")
            result = stringToLL(tok->previous->str()) >> stringToLL(tok->next->str());
        else
            continue;

        tok = tok->previous;
        tok->setstr(toString(result));
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

static const std::string NOTEQ("not_eq");
void simplecpp::TokenList::constFoldComparison(Token *tok)
{
    for (; tok && tok->op != ')'; tok = tok->next) {
        if (isAlternativeBinaryOp(tok,NOTEQ))
            tok->setstr("!=");

        if (!tok->isOneOf("<>") && tok->str() != "<=" && tok->str() != ">=" && tok->str() != "==" && tok->str() != "!=")
            continue;
        if (!isValue(tok->previous) || !isValue(tok->next))
            continue;
        if (Token * const errorValue = foldErrorValue(*this, tok)) {
            tok = errorValue;
            continue;
        }

        int result;
        if (tok->str() == "==")
            result = (stringToLL(tok->previous->str()) == stringToLL(tok->next->str()));
        else if (tok->str() == "!=")
            result = (stringToLL(tok->previous->str()) != stringToLL(tok->next->str()));
        else if (tok->str() == ">")
            result = (stringToLL(tok->previous->str()) > stringToLL(tok->next->str()));
        else if (tok->str() == ">=")
            result = (stringToLL(tok->previous->str()) >= stringToLL(tok->next->str()));
        else if (tok->str() == "<")
            result = (stringToLL(tok->previous->str()) < stringToLL(tok->next->str()));
        else if (tok->str() == "<=")
            result = (stringToLL(tok->previous->str()) <= stringToLL(tok->next->str()));
        else
            continue;

        tok = tok->previous;
        tok->setstr(toString(result));
        deleteToken(tok->next);
        deleteToken(tok->next);
    }
}

static const std::string XOR("xor");
void simplecpp::TokenList::constFoldBitwise(Token *tok)
{
    Token * const tok1 = tok;
    for (const char *op = "&^|"; *op; op++) {
        const std::string* alternativeOp;
        if (*op == '&')
            alternativeOp = &BITAND;
        else if (*op == '|')
            alternativeOp = &BITOR;
        else
            alternativeOp = &XOR;
        for (tok = tok1; tok && tok->op != ')'; tok = tok->next) {
            if (tok->op != *op && !isAlternativeBinaryOp(tok, *alternativeOp))
                continue;
            if (!isValue(tok->previous) || !isValue(tok->next))
                continue;
            if (Token * const errorValue = foldErrorValue(*this, tok)) {
                tok = errorValue;
                continue;
            }
            long long result;
            if (*op == '&')
                result = (stringToLL(tok->previous->str()) & stringToLL(tok->next->str()));
            else if (*op == '^')
                result = (stringToLL(tok->previous->str()) ^ stringToLL(tok->next->str()));
            else /*if (*op == '|')*/
                result = (stringToLL(tok->previous->str()) | stringToLL(tok->next->str()));
            tok = tok->previous;
            tok->setstr(toString(result));
            deleteToken(tok->next);
            deleteToken(tok->next);
        }
    }
}

static const std::string OR("or");
void simplecpp::TokenList::constFoldLogicalOp(Token *tok)
{
    for (Token *t = tok; t && t->op != ')'; t = t->next) {
        if (t->name) {
            if (isAlternativeBinaryOp(t,AND))
                t->setstr("&&");
            else if (isAlternativeBinaryOp(t,OR))
                t->setstr("||");
        }
    }

    // && has higher precedence than ||
    for (const char * const op : {"&&", "||"}) {
        for (Token *t = tok; t && t->op != ')'; t = t->next) {
            if (t->str() != op)
                continue;
            if (!isValue(t->previous) || !isValue(t->next))
                continue;

            // the right operand is not evaluated if the left operand decides the result
            std::string result;
            if (isErrorValue(t->previous))
                result = t->previous->str();
            else if (t->str() == "||" && stringToLL(t->previous->str()) != 0)
                result = "1";
            else if (t->str() == "&&" && stringToLL(t->previous->str()) == 0)
                result = "0";
            else if (isErrorValue(t->next))
                result = t->next->str();
            else
                result = (stringToLL(t->next->str()) != 0) ? "1" : "0";

            t = t->previous;
            t->setstr(result);
            deleteToken(t->next);
            deleteToken(t->next);
        }
    }
}

void simplecpp::TokenList::constFoldQuestionOp(Token *&tok1)
{
    // ?: is right-associative, "a ? b : c ? d : e" means "a ? b : (c ? d : e)",
    // so the last ? is folded first
    std::vector<const Token *> questionTokens;
    for (const Token *t = tok1; t && t->op != ')'; t = t->next) {
        if (t->str() == "?")
            questionTokens.push_back(t);
    }
    while (!questionTokens.empty()) {
        const Token * const tok = questionTokens.back();
        questionTokens.pop_back();
        if (!tok->previous || !tok->next || !tok->next->next)
            throw std::runtime_error("invalid expression");
        Token * const condTok = tok->previous;
        Token * const trueTok = tok->next;
        Token * const falseTok = trueTok->next->next;
        if (!falseTok)
            throw std::runtime_error("invalid expression");
        if (trueTok->next->op != ':')
            return;
        if (!isValue(condTok) || !isValue(trueTok) || !isValue(falseTok))
            return;
        if (isErrorValue(condTok)) {
            // "error ? x : y" => error
            deleteToken(condTok->next); // ?
            deleteToken(trueTok->next); // :
            deleteToken(trueTok);
            deleteToken(falseTok);
        } else {
            Token * const result = (stringToLL(condTok->str()) != 0 ? trueTok : falseTok);
            if (condTok == tok1)
                tok1 = result;
            deleteToken(condTok->next); // ?
            deleteToken(trueTok->next); // :
            deleteToken(result == falseTok ? trueTok : falseTok); // the operand that is not evaluated
            deleteToken(condTok);
        }
    }
}

void simplecpp::TokenList::removeComments()
{
    Token *tok = frontToken;
    while (tok) {
        Token * const tok1 = tok;
        tok = tok->next;
        if (tok1->comment) {
            // a comment is replaced by one space character
            if (tok1->previous)
                tok1->previous->whitespaceahead = true;
            deleteToken(tok1);
        }
    }
}

std::string simplecpp::TokenList::readUntil(Stream &stream, const Location &location, const char start, const char end, OutputList *outputList)
{
    std::string ret;
    ret += start;

    bool backslash = false;
    char ch = 0;
    while (ch != end && ch != '\r' && ch != '\n' && stream.good()) {
        ch = stream.readChar();
        if (backslash && ch == '\n') {
            ch = 0;
            backslash = false;
            continue;
        }
        backslash = false;
        ret += ch;
        if (ch == '\\') {
            bool update_ch = false;
            char next = 0;
            do {
                next = stream.readChar();
                if (next == '\r' || next == '\n') {
                    ret.erase(ret.size()-1U);
                    backslash = (next == '\r');
                    update_ch = false;
                } else if (next == '\\') {
                    update_ch = !update_ch;
                }
                ret += next;
            } while (next == '\\');
            if (update_ch)
                ch = next;
        }
    }

    if (!stream.good() || ch != end) {
        clear();
        if (outputList) {
            Output err{
                Output::SYNTAX_ERROR,
                location,
                std::string("No pair for character (") + start + "). Can't process file. File is either invalid or unicode, which is currently not supported."
            };
            outputList->emplace_back(std::move(err));
        }
        return "";
    }

    return ret;
}

const simplecpp::Token* simplecpp::TokenList::lastLineTok(int maxsize) const
{
    const Token* prevTok = nullptr;
    int count = 0;
    for (const Token *tok = cback(); ; tok = tok->previous) {
        if (!sameline(tok, cback()))
            break;
        if (tok->comment)
            continue;
        if (++count > maxsize)
            return nullptr;
        prevTok = tok;
    }
    return prevTok;
}

const simplecpp::Token* simplecpp::TokenList::isLastLinePreprocessor(int maxsize) const
{
    const Token * const prevTok = lastLineTok(maxsize);
    if (prevTok && prevTok->op == '#')
        return prevTok;
    return nullptr;
}

unsigned int simplecpp::TokenList::fileIndex(const std::string &filename)
{
    const auto it = std::find_if(files.cbegin(), files.cend(), [&filename](const std::string &f) {
        return f == filename;
    });
    if (it != files.cend())
        return static_cast<unsigned int>(std::distance(files.cbegin(), it));
    files.emplace_back(filename);
    return files.size() - 1U;
}

const std::string& simplecpp::TokenList::file(const Location& loc) const
{
    static const std::string s_emptyFileName;
    return loc.fileIndex < files.size() ? files[loc.fileIndex] : s_emptyFileName;
}


namespace simplecpp {
    class Macro;
    using MacroMap = std::unordered_map<TokenString,Macro>;

    /** Set the location of all tokens */
    static void setLocation(TokenList &tokens, const Location &loc)
    {
        for (Token *tok = tokens.front(); tok; tok = tok->next)
            tok->location = loc;
    }

    class Macro {
    public:
        /** a usage of this macro: where it is used and, for INTn_C / UINTn_C, the argument of the call */
        struct Usage {
            explicit Usage(const Location &location) : location(location) {}
            Location location;
            std::string arg;
        };

        explicit Macro(std::vector<std::string> &f) : nameTokDef(nullptr), valueToken(nullptr), endToken(nullptr), files(f), tokenListDefine(f), variadic(false), variadicOpt(false), valueDefinedInCode_(false) {}

        /**
         * @throws std::runtime_error thrown on bad macro syntax
         */
        Macro(const Token *tok, std::vector<std::string> &f) : nameTokDef(nullptr), files(f), tokenListDefine(f), valueDefinedInCode_(true) {
            if (sameline(tok->previousSkipComments(), tok))
                throw std::runtime_error("bad macro syntax");
            if (tok->op != '#')
                throw std::runtime_error("bad macro syntax");
            const Token * const hashtok = tok;
            tok = tok->next;
            if (!tok || tok->str() != DEFINE)
                throw std::runtime_error("bad macro syntax");
            tok = tok->next;
            if (!tok || !tok->name || !sameline(hashtok,tok))
                throw std::runtime_error("bad macro syntax");
            if (!parseDefine(tok))
                throw std::runtime_error("bad macro syntax");
        }

        /**
         * @throws std::runtime_error thrown on bad macro syntax
         */
        Macro(const std::string &name, const std::string &value, std::vector<std::string> &f) : nameTokDef(nullptr), files(f), tokenListDefine(f), valueDefinedInCode_(false) {
            const std::string def(name + ' ' + value + '\n');
            StdCharBufStream stream(reinterpret_cast<const unsigned char*>(def.data()), def.size());
            tokenListDefine.readfile(stream);
            if (!parseDefine(tokenListDefine.cfront()))
                throw std::runtime_error("bad macro syntax. macroname=" + name + " value=" + value);
        }

        Macro(const Macro &other) : nameTokDef(nullptr), files(other.files), tokenListDefine(other.files), valueDefinedInCode_(other.valueDefinedInCode_) {
            // TODO: remove the try-catch - see #537
            // avoid bugprone-exception-escape clang-tidy warning
            try {
                *this = other;
            }
            catch (const Error&) {} // NOLINT(bugprone-empty-catch)
        }

        ~Macro() {
            delete optExpandValue;
            delete optNoExpandValue;
        }

        Macro &operator=(const Macro &other) {
            if (this != &other) {
                files = other.files;
                valueDefinedInCode_ = other.valueDefinedInCode_;
                if (other.tokenListDefine.empty()) {
                    parseDefine(other.nameTokDef);
                }
                else {
                    tokenListDefine = other.tokenListDefine;
                    parseDefine(tokenListDefine.cfront());
                }
                usageList = other.usageList;
            }
            return *this;
        }

        bool valueDefinedInCode() const {
            return valueDefinedInCode_;
        }

        /**
         * Copy a macro call "NAME( ... )" to a tokenlist with no line breaks:
         * all tokens get the location of the name.
         * @param dest    destination tokenlist
         * @param nameTok the macro name, followed by '('
         * @return the token after the ')', or nullptr if the call is not terminated
         * @throws invalidDirectiveAsMacroParameter if there is a preprocessor directive in the arguments
         */
        static const Token *copyMacroCall(TokenList &dest, const Token *nameTok) {
            const Location &loc = nameTok->location;
            dest.push_back(new Token(nameTok->str(), loc, nameTok->whitespaceahead));
            const Token *tok = nameTok->next;
            dest.push_back(new Token(tok->str(), loc, tok->whitespaceahead));
            tok = tok->next;
            int par = 1;
            while (tok && par > 0) {
                if (tok->op == '(')
                    ++par;
                else if (tok->op == ')')
                    --par;
                else if (tok->op == '#' && !sameline(tok->previous, tok))
                    throw invalidDirectiveAsMacroParameter(tok->location);
                dest.push_back(new Token(tok->str(), loc, tok->whitespaceahead));
                tok = tok->next;
            }
            return tok;
        }

        /**
         * Expand macro. This will recursively expand inner macros.
         * @param output     destination tokenlist
         * @param rawtok     macro token
         * @param macros     list of macros
         * @param inputFiles the input files
         * @param outputList destination for warnings, may be nullptr
         * @return token after macro
         * @throws Error thrown on missing or invalid preprocessor directives
         * @throws wrongNumberOfParameters thrown on invalid number of parameters
         * @throws invalidHashHash thrown on invalid ## usage
         */
        const Token * expand(TokenList & output,
                             const Token * rawtok,
                             const MacroMap &macros,
                             std::vector<std::string> &inputFiles,
                             OutputList *outputList) const {
            std::set<TokenString> expandedmacros;

#ifdef SIMPLECPP_DEBUG_MACRO_EXPANSION
            std::cout << "expand " << name() << " " << locstring(rawtok->location) << std::endl;
#endif

            TokenList output2(inputFiles);

            if (functionLike() && rawtok->next && rawtok->next->op == '(') {
                const Token * const rawtok1 = rawtok;
                TokenList rawtokens2(inputFiles);
                rawtok = copyMacroCall(rawtokens2, rawtok);
                if (expand(output2, rawtok1->location, rawtokens2.cfront(), macros, expandedmacros, outputList))
                    rawtok = rawtok1->next;
            } else {
                rawtok = expand(output2, rawtok->location, rawtok, macros, expandedmacros, outputList);
            }
            while (output2.cback() && rawtok) {
                unsigned int par = 0;
                Token* macro2tok = output2.back();
                while (macro2tok) {
                    if (macro2tok->op == '(') {
                        if (par==0)
                            break;
                        --par;
                    } else if (macro2tok->op == ')') {
                        ++par;
                    }
                    macro2tok = macro2tok->previous;
                }
                if (macro2tok) { // macro2tok->op == '('
                    macro2tok = macro2tok->previous;
                    expandedmacros.insert(name());
                } else if (rawtok->op == '(') {
                    macro2tok = output2.back();
                }
                const Macro * const calledMacro = callableMacro(macro2tok, macros, std::set<TokenString>());
                if (!calledMacro || (output2.cfront() != output2.cback() && macro2tok->str() == this->name()))
                    break;
                TokenList rawtokens2(inputFiles);
                const Location loc(macro2tok->location);
                while (macro2tok) {
                    Token * const next = macro2tok->next;
                    rawtokens2.push_back(new Token(macro2tok->str(), loc));
                    output2.deleteToken(macro2tok);
                    macro2tok = next;
                }
                par = (rawtokens2.cfront() != rawtokens2.cback()) ? 1U : 0U;
                const Token *rawtok2 = rawtok;
                for (; rawtok2; rawtok2 = rawtok2->next) {
                    rawtokens2.push_back(new Token(rawtok2->str(), loc));
                    if (rawtok2->op == '(') {
                        ++par;
                    }
                    else if (rawtok2->op == ')') {
                        if (par <= 1U)
                            break;
                        --par;
                    }
                }
                if (!rawtok2 || par != 1U)
                    break;
                const Token * const output2End = output2.cback();
                if (calledMacro->expand(output2, rawtok->location, rawtokens2.cfront(), macros, expandedmacros, outputList) != nullptr)
                    break;
                if (output2.cback() == output2End)
                    skipTrailingName(output2);
                rawtok = rawtok2->next;
            }
            output.takeTokens(output2);
            return rawtok;
        }

        /** macro name */
        const TokenString &name() const {
            return nameTokDef->str();
        }

        /** location for macro definition */
        const Location &defineLocation() const {
            return nameTokDef->location;
        }

        /** how has this macro been used so far */
        const std::list<Usage> &usage() const {
            return usageList;
        }

        /** is this a function like macro */
        bool functionLike() const {
            return nameTokDef->next &&
                   nameTokDef->next->op == '(' &&
                   sameline(nameTokDef, nameTokDef->next) &&
                   nameTokDef->next->location.col == nameTokDef->location.col + nameTokDef->str().size();
        }

        /** base class for errors */
        struct Error {
            Error(const Location &loc, const std::string &s) : location(loc), what(s) {}
            const Location location;
            const std::string what;
        };

        struct invalidDirectiveAsMacroParameter : public Error {
            explicit invalidDirectiveAsMacroParameter(const Location &loc)
                : Error(loc, "it is invalid to use a preprocessor directive as macro parameter") {}
        };

        /** Struct that is thrown when macro is expanded with wrong number of parameters */
        struct wrongNumberOfParameters : public Error {
            wrongNumberOfParameters(const Location &loc, const std::string &macroName) : Error(loc, "Wrong number of parameters for macro \'" + macroName + "\'.") {}
        };

        /** Struct that is thrown when there is invalid ## usage */
        struct invalidHashHash : public Error {
            static inline std::string format(const std::string &macroName, const std::string &message) {
                return "Invalid ## usage when expanding \'" + macroName + "\': " + message;
            }

            invalidHashHash(const Location &loc, const std::string &macroName, const std::string &message)
                : Error(loc, format(macroName, message)) {}

            static inline invalidHashHash unexpectedToken(const Location &loc, const std::string &macroName, const Token *tokenA) {
                return {loc, macroName, "Unexpected token '"+ tokenA->str()+"'"};
            }

            static inline invalidHashHash cannotCombine(const Location &loc, const std::string &macroName, const Token *tokenA, const Token *tokenB) {
                return {loc, macroName, "Combining '"+ tokenA->str()+ "' and '"+ tokenB->str() + "' yields an invalid token."};
            }

            static inline invalidHashHash unexpectedNewline(const Location &loc, const std::string &macroName) {
                return {loc, macroName, "Unexpected newline"};
            }

            static inline invalidHashHash universalCharacterUB(const Location &loc, const std::string &macroName, const Token* tokenA, const std::string& strAB) {
                return {loc, macroName, "Combining '\\"+ tokenA->str()+ "' and '"+ strAB.substr(tokenA->str().size()) + "' yields universal character '\\" + strAB + "'. This is undefined behavior according to C standard chapter 5.1.1.2, paragraph 4."};
            }
        };
    private:
        /** Create new token where Token::macro is set for replaced tokens */
        Token *newMacroToken(const TokenString &str, const Location &loc, bool replaced, const Token *expandedFromToken=nullptr) const {
            auto *tok = new Token(str,loc);
            if (replaced)
                tok->macro = nameTokDef->str();
            if (expandedFromToken)
                tok->setExpandedFrom(expandedFromToken, this);
            return tok;
        }

        /**
         * @throws Error thrown in case of __VA_OPT__ issues
         */
        bool parseDefine(const Token *nametoken) {
            nameTokDef = nametoken;
            variadic = false;
            variadicOpt = false;
            delete optExpandValue;
            optExpandValue = nullptr;
            delete optNoExpandValue;
            optNoExpandValue = nullptr;
            if (!nameTokDef) {
                valueToken = endToken = nullptr;
                args.clear();
                return false;
            }

            // function like macro..
            if (functionLike()) {
                args.clear();
                const Token *argtok = nameTokDef->next->next;
                while (sameline(nametoken, argtok) && argtok->op != ')') {
                    if (argtok->str() == "..." &&
                        argtok->next && argtok->next->op == ')') {
                        variadic = true;
                        if (!argtok->previous->name)
                            args.emplace_back("__VA_ARGS__");
                        argtok = argtok->next; // goto ')'
                        break;
                    }
                    if (argtok->op != ',')
                        args.emplace_back(argtok->str());
                    argtok = argtok->next;
                }
                if (!sameline(nametoken, argtok)) {
                    endToken = argtok ? argtok->previous : argtok;
                    valueToken = nullptr;
                    return false;
                }
                valueToken = argtok ? argtok->next : nullptr;
            } else {
                args.clear();
                valueToken = nameTokDef->next;
            }

            if (!sameline(valueToken, nameTokDef))
                valueToken = nullptr;
            endToken = valueToken;
            while (sameline(endToken, nameTokDef)) {
                if (variadic && endToken->str() == "__VA_OPT__")
                    variadicOpt = true;
                endToken = endToken->next;
            }

            if (variadicOpt) {
                TokenList expandValue(files);
                TokenList noExpandValue(files);
                for (const Token *tok = valueToken; tok && tok != endToken;) {
                    if (tok->str() == "__VA_OPT__") {
                        if (!sameline(tok, tok->next) || tok->next->op != '(')
                            throw Error(tok->location, "In definition of '" + nameTokDef->str() + "': Missing opening parenthesis for __VA_OPT__");
                        tok = tok->next->next;
                        int par = 1;
                        while (tok && tok != endToken) {
                            if (tok->op == '(')
                                par++;
                            else if (tok->op == ')')
                                par--;
                            else if (tok->str() == "__VA_OPT__")
                                throw Error(tok->location, "In definition of '" + nameTokDef->str() + "': __VA_OPT__ cannot be nested");
                            if (par == 0) {
                                tok = tok->next;
                                break;
                            }
                            expandValue.push_back(new Token(*tok));
                            tok = tok->next;
                        }
                        if (par != 0) {
                            const Token *const lastTok = expandValue.back() ? expandValue.back() : valueToken->next;
                            throw Error(lastTok->location, "In definition of '" + nameTokDef->str() + "': Missing closing parenthesis for __VA_OPT__");
                        }
                    } else {
                        expandValue.push_back(new Token(*tok));
                        noExpandValue.push_back(new Token(*tok));
                        tok = tok->next;
                    }
                }
                optExpandValue = new TokenList(std::move(expandValue));
                optNoExpandValue = new TokenList(std::move(noExpandValue));
            }

            return true;
        }

        unsigned int getArgNum(const TokenString &str) const {
            unsigned int par = 0;
            while (par < args.size()) {
                if (str == args[par])
                    return par;
                par++;
            }
            return ~0U;
        }

        /**
         * Find the arguments of a call of this macro.
         * @param nameTokInst    the macro name in the call
         * @param calledInDefine the call is in a #define, so the arguments must be on the same line
         * @return the tokens that delimit the arguments: the '(', each ',' between two arguments and the ')'.
         *         Empty when the macro is not function-like.
         * @throws wrongNumberOfParameters if a function-like macro has the wrong number of arguments.
         *         This includes the case where the name is not followed by '(' (or is the last token),
         *         so the caller must check for that first if it is not an error.
         */
        std::vector<const Token *> getMacroParameters(const Token *nameTokInst, bool calledInDefine) const {
            if (!functionLike())
                return {};
            std::vector<const Token *> parametertokens;
            if (nameTokInst->next && nameTokInst->next->op == '(')
                parametertokens = findArgumentDelimiters(nameTokInst, calledInDefine);

            if (variadic) {
                if (parametertokens.size() < args.size())
                    throw wrongNumberOfParameters(nameTokInst->location, name());
            } else {
                if (parametertokens.size() != args.size() + (args.empty() ? 2U : 1U))
                    throw wrongNumberOfParameters(nameTokInst->location, name());
            }
            return parametertokens;
        }

        /** Is name one of the <stdint.h> integer constant macros INTn_C / UINTn_C, n = 8, 16, 32, 64 or MAX? */
        static bool isIntegerConstantMacro(const std::string &name) {
            const std::string::size_type pos = (name.compare(0, 1, "U") == 0) ? 1 : 0;
            if (name.compare(pos, 3, "INT") != 0)
                return false;
            const std::string bits = name.substr(pos + 3);
            return bits == "8_C" || bits == "16_C" || bits == "32_C" || bits == "64_C" || bits == "MAX_C";
        }

        /**
         * The text between the parentheses of the call: the tokens joined with a single space, comments are
         * skipped. nameTokInst is followed by '('. If the call is in a macro body, the parameters of the
         * enclosing macro have already been substituted and the macros in them expanded.
         */
        static std::string writtenArgument(const Token *nameTokInst, bool calledInDefine) {
            std::string ret;
            unsigned int par = 0U;
            for (const Token *tok = nameTokInst->next->next; calledInDefine ? sameline(tok, nameTokInst) : (tok != nullptr); tok = tok->next) {
                if (tok->comment)
                    continue;
                if (tok->op == '(') {
                    ++par;
                } else if (tok->op == ')') {
                    if (par == 0U)
                        break;
                    --par;
                }
                if (!ret.empty())
                    ret += ' ';
                ret += tok->str();
            }
            return ret;
        }

        /** The '(', each ',' between two arguments and the ')' of the call. nameTokInst is followed by '('. */
        std::vector<const Token *> findArgumentDelimiters(const Token *nameTokInst, bool calledInDefine) const {
            std::vector<const Token *> parametertokens;
            parametertokens.emplace_back(nameTokInst->next);
            unsigned int par = 0U;
            for (const Token *tok = nameTokInst->next->next; calledInDefine ? sameline(tok, nameTokInst) : (tok != nullptr); tok = tok->next) {
                if (tok->op == '(') {
                    ++par;
                }
                else if (tok->op == ')') {
                    if (par == 0U) {
                        parametertokens.emplace_back(tok);
                        break;
                    }
                    --par;
                } else if (par == 0U && tok->op == ',' && (!variadic || parametertokens.size() < args.size())) {
                    parametertokens.emplace_back(tok);
                }
            }
            return parametertokens;
        }

        const Token *appendTokens(TokenList &tokens,
                                  const Location &rawloc,
                                  const Token * const lpar,
                                  const MacroMap &macros,
                                  const std::set<TokenString> &expandedmacros,
                                  const std::vector<const Token*> &parametertokens,
                                  OutputList *outputList) const {
            if (!lpar || lpar->op != '(')
                return nullptr;
            unsigned int par = 0;
            const Token *tok = lpar;
            while (sameline(lpar, tok)) {
                if (tok->op == '#' && sameline(tok,tok->next) && tok->next->op == '#' && sameline(tok,tok->next->next)) {
                    // A##B => AB
                    tok = expandHashHash(tokens, rawloc, tok, macros, expandedmacros, parametertokens, outputList, false);
                } else if (tok->op == '#' && sameline(tok, tok->next) && tok->next->op != '#') {
                    tok = expandHash(tokens, rawloc, tok, expandedmacros, parametertokens, outputList);
                } else {
                    if (!expandArg(tokens, tok, rawloc, macros, expandedmacros, parametertokens, outputList)) {
                        tokens.push_back(new Token(*tok));
                        if (tok->macro.empty() && (par > 0 || tok->str() != "("))
                            tokens.back()->macro = name();
                    }

                    if (tok->op == '(') {
                        ++par;
                    }
                    else if (tok->op == ')') {
                        --par;
                        if (par == 0U)
                            break;
                    }
                    tok = tok->next;
                }
            }
            setLocation(tokens, lpar->location);
            return sameline(lpar,tok) ? tok : nullptr;
        }

        /**
         * Expand the built-in macros __FILE__, __LINE__ and __COUNTER__.
         * The usage must already be added to usageList, __COUNTER__ is the number of earlier usages.
         * @param output      destination tokenlist
         * @param loc         location of the usage
         * @param nameTokInst the macro name
         * @return true if the macro is built-in and its value is added to output
         */
        bool expandBuiltin(TokenList &output, const Location &loc, const Token *nameTokInst) const {
            std::string value;
            if (nameTokInst->str() == "__FILE__")
                value = '\"' + output.file(loc) + '\"';
            else if (nameTokInst->str() == "__LINE__")
                value = toString(loc.line);
            else if (nameTokInst->str() == "__COUNTER__")
                value = toString(usageList.size() - 1U);
            else
                return false;
            output.push_back(new Token(value, loc));
            output.back()->macro = nameTokInst->str();
            return true;
        }

        const Token * expand(TokenList & output, const Location &loc, const Token * const nameTokInst, const MacroMap &macros, std::set<TokenString> expandedmacros, OutputList *outputList) const {
            expandedmacros.insert(nameTokInst->str());

#ifdef SIMPLECPP_DEBUG_MACRO_EXPANSION
            std::cout << "  expand " << name() << " " << locstring(defineLocation()) << std::endl;
#endif

            usageList.emplace_back(loc);

            if (expandBuiltin(output, loc, nameTokInst))
                return nameTokInst->next;

            const bool calledInDefine = (loc.fileIndex != nameTokInst->location.fileIndex ||
                                         loc.line < nameTokInst->location.line);

            // No arguments => not macro expansion
            if (functionLike() && nameTokInst->next && nameTokInst->next->op != '(') {
                output.push_back(new Token(nameTokInst->str(), loc));
                return nameTokInst->next;
            }

            if (functionLike() && nameTokInst->next && nameTokInst->next->op == '(' && isIntegerConstantMacro(name()))
                usageList.back().arg = writtenArgument(nameTokInst, calledInDefine);

            std::vector<const Token*> parametertokens1(getMacroParameters(nameTokInst, calledInDefine));

            // If macro call uses __COUNTER__ then expand that first
            TokenList tokensparams(files);
            std::vector<const Token *> parametertokens2;
            if (!parametertokens1.empty()) {
                bool counter = false;
                for (const Token *tok = parametertokens1[0]; tok != parametertokens1.back(); tok = tok->next) {
                    if (tok->str() == "__COUNTER__") {
                        counter = true;
                        break;
                    }
                }

                const MacroMap::const_iterator m = macros.find("__COUNTER__");

                if (!counter || m == macros.end()) {
                    parametertokens2.swap(parametertokens1);
                }
                else {
                    const Macro &counterMacro = m->second;
                    unsigned int par = 0;
                    for (const Token *tok = parametertokens1[0]; tok && par < parametertokens1.size(); tok = tok->next) {
                        if (tok->str() == "__COUNTER__") {
                            tokensparams.push_back(new Token(toString(counterMacro.usageList.size()), tok->location));
                            counterMacro.usageList.emplace_back(tok->location);
                        } else {
                            tokensparams.push_back(new Token(*tok));
                            if (tok == parametertokens1[par]) {
                                parametertokens2.emplace_back(tokensparams.cback());
                                par++;
                            }
                        }
                    }
                }
            }

            // NOLINTNEXTLINE(misc-const-correctness) - technically correct but used to access non-const data
            Token * const output_end_1 = output.back();

            const Token *valueToken2;
            const Token *endToken2;

            if (variadicOpt) {
                if (parametertokens2.size() > args.size() && parametertokens2[args.size() - 1]->next->op != ')')
                    valueToken2 = optExpandValue->cfront();
                else
                    valueToken2 = optNoExpandValue->cfront();
                endToken2 = nullptr;
            } else {
                valueToken2 = valueToken;
                endToken2 = endToken;
            }

            // expand
            for (const Token *tok = valueToken2; tok != endToken2;) {
                if (tok->op != '#') {
                    // A##B => AB
                    if (sameline(tok, tok->next) && tok->next && tok->next->op == '#' && tok->next->next && tok->next->next->op == '#') {
                        if (!sameline(tok, tok->next->next->next))
                            throw invalidHashHash::unexpectedNewline(tok->location, name());
                        if (variadic && tok->op == ',' && tok->next->next->next->str() == args.back()) {
                            Token *const comma = newMacroToken(tok->str(), loc, isReplaced(expandedmacros), tok);
                            output.push_back(comma);
                            tok = expandToken(output, loc, tok->next->next->next, macros, expandedmacros, parametertokens2, outputList);
                            if (output.back() == comma)
                                output.deleteToken(comma);
                            continue;
                        }
                        TokenList new_output(files);
                        if (!expandArg(new_output, tok, parametertokens2))
                            output.push_back(newMacroToken(tok->str(), loc, isReplaced(expandedmacros), tok));
                        else if (new_output.empty()) // placemarker token
                            output.push_back(newMacroToken("", loc, isReplaced(expandedmacros)));
                        else
                            for (const Token *tok2 = new_output.cfront(); tok2; tok2 = tok2->next)
                                output.push_back(newMacroToken(tok2->str(), loc, isReplaced(expandedmacros), tok2));
                        tok = tok->next;
                    } else {
                        tok = expandToken(output, loc, tok, macros, expandedmacros, parametertokens2, outputList);
                    }
                    continue;
                }

                int numberOfHash = 1;
                const Token *hashToken = tok->next;
                while (sameline(tok,hashToken) && hashToken->op == '#') {
                    hashToken = hashToken->next;
                    ++numberOfHash;
                }
                if (numberOfHash == 4 && tok->next->location.col + 1 == tok->next->next->location.col) {
                    // # ## #  => ##
                    output.push_back(newMacroToken("##", loc, isReplaced(expandedmacros)));
                    tok = hashToken;
                    continue;
                }

                if (numberOfHash >= 2 && tok->location.col + 1 < tok->next->location.col) {
                    output.push_back(new Token(*tok));
                    tok = tok->next;
                    continue;
                }

                tok = tok->next;
                if (tok == endToken2) {
                    if (tok) {
                        output.push_back(new Token(*tok->previous));
                    }
                    else {
                        output.push_back(new Token(*nameTokInst));
                        output.back()->setstr("\"\"");
                    }
                    break;
                }
                if (tok->op == '#') {
                    // A##B => AB
                    tok = expandHashHash(output, loc, tok->previous, macros, expandedmacros, parametertokens2, outputList);
                } else {
                    // #123 => "123"
                    tok = expandHash(output, loc, tok->previous, expandedmacros, parametertokens2, outputList);
                }
            }

            if (!functionLike()) {
                for (Token *tok = output_end_1 ? output_end_1->next : output.front(); tok; tok = tok->next) {
                    tok->macro = nameTokInst->str();
                }
            }

            if (!parametertokens1.empty())
                parametertokens1.swap(parametertokens2);

            const Token * const callEnd = functionLike() ? parametertokens2.back() : nameTokInst;

            // Whitespace at the end of the replacement list does not count.
            // The expansion is followed by the whitespace after the macro call.
            if (output.back() && output.back() != output_end_1)
                output.back()->whitespaceahead = callEnd->whitespaceahead;

            return callEnd->next;
        }

        /** Returns the macro when @p tok is the name of a function-like macro
         *  that the rescan can still expand; nullptr otherwise */
        static const Macro *callableMacro(const Token *tok, const MacroMap &macros, const std::set<TokenString> &expandedmacros) {
            if (!tok || !tok->name || tok->skippedByRescan)
                return nullptr;
            const MacroMap::const_iterator it = macros.find(tok->str());
            if (it == macros.end() || expandedmacros.find(tok->str()) != expandedmacros.end())
                return nullptr;
            if (!it->second.functionLike() || tok->isExpandedFrom(&it->second))
                return nullptr;
            return &it->second;
        }

        /** Mark @p tok when it is the name of a macro that is being expanded, so
         *  that it is never expanded later */
        static void markExpanding(Token *tok, const MacroMap &macros, const std::set<TokenString> &expandedmacros) {
            if (!tok->name || expandedmacros.find(tok->str()) == expandedmacros.end())
                return;
            const MacroMap::const_iterator it = macros.find(tok->str());
            if (it != macros.end())
                tok->markExpandedFrom(&it->second);
        }

        /** A macro call that expanded to nothing came after the last token of
         *  @p output: the rescan has passed that token, so when it is the name of
         *  a function-like macro, a '(' that comes later does not start a call. */
        static void skipTrailingName(TokenList &output) {
            if (output.back() && output.back()->name)
                output.back()->skippedByRescan = true;
        }

        /** Copy the call of the function-like macro named by @p nameTok whose
         *  "( ... )" follows @p tok into @p call, stopping before @p end. Returns
         *  the matching ')', or nullptr when there is no complete call. */
        static const Token *gatherCall(TokenList &call, const Token *nameTok, const Token *tok, const Token *end = nullptr) {
            if (!sameline(tok, tok->next) || tok->next->op != '(')
                return nullptr;
            call.push_back(new Token(nameTok->str(), tok->next->location));
            unsigned int par = 0;
            for (const Token *tok2 = tok->next; tok2 && tok2 != end; tok2 = tok2->next) {
                call.push_back(new Token(*tok2));
                if (tok2->op == '(') {
                    ++par;
                } else if (tok2->op == ')') {
                    if (--par == 0U)
                        return tok2;
                }
            }
            return nullptr;
        }

        /** Gather the call of the function-like macro named by @p nameTok whose
         *  arguments follow the replacement-list token @p tok into @p call.
         *  Parameters are substituted before the rescan, so the parentheses are
         *  counted in the substituted tokens: parameters that expand to nothing are
         *  skipped, and the '(' and the ')' of the call can each come from the
         *  replacement list or from a parameter. A parameter that is followed by
         *  ## is not macro-expanded: the search stops at it when it comes before
         *  the '('. Returns the replacement-list token
         *  that supplied the ')', or nullptr when there is no complete call. When
         *  that token is a parameter, its expansion is copied whole since it may
         *  hold more than the end of the call. When the first parameter that does
         *  not expand to nothing does not start with '(', its expansion is moved
         *  to @p arg and the parameter is returned in @p argTok, so that the
         *  caller does not expand it again. */
        const Token *gatherCallAfter(TokenList &call, const Token *nameTok, const Location &loc, const Token *tok, const MacroMap &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens, OutputList *outputList, TokenList &arg, const Token *&argTok) const {
            TokenList callArgs(files);
            unsigned int par = 0;
            for (const Token *after = tok->next; sameline(tok, after);) {
                if (par > 0U && after->op == '#' && sameline(after, after->next) && (after->next->op != '#' || sameline(after, after->next->next))) {
                    // # and ## keep the parentheses balanced
                    if (after->next->op == '#')
                        after = expandHashHash(callArgs, loc, after, macros, expandedmacros, parametertokens, outputList, false);
                    else
                        after = expandHash(callArgs, loc, after, expandedmacros, parametertokens, outputList);
                    continue;
                }
                const Token * const end = callArgs.cback();
                bool isParameter;
                if (sameline(after, after->next) && after->next->op == '#' && sameline(after, after->next->next) && after->next->next->op == '#') {
                    // the left operand of ## is substituted without macro expansion
                    isParameter = expandArg(callArgs, after, parametertokens);
                    // the paste can change what follows the macro name
                    if (isParameter && par == 0U)
                        return nullptr;
                } else {
                    isParameter = expandArg(callArgs, after, loc, macros, expandedmacros, parametertokens, outputList);
                }
                if (!isParameter) {
                    // only parameters, which can expand to nothing, can come between
                    // the macro name and its '('
                    if (par == 0U && after->op != '(')
                        return nullptr;
                    callArgs.push_back(new Token(*after));
                    if (after->macro.empty() && par > 0U)
                        callArgs.back()->macro = name();
                }
                for (const Token *tok2 = end ? end->next : callArgs.cfront(); tok2; tok2 = tok2->next) {
                    if (tok2->op == '(') {
                        ++par;
                    } else if (par == 0U) {
                        // the parameter does not start with '('
                        arg.takeTokens(callArgs);
                        argTok = after;
                        return nullptr;
                    } else if (tok2->op == ')' && --par == 0U) {
                        // the tokens of the call must be on the same line
                        for (Token *tok3 = callArgs.front(); tok3 != tok2->next; tok3 = tok3->next)
                            tok3->location = callArgs.cfront()->location;
                        call.push_back(new Token(nameTok->str(), callArgs.cfront()->location));
                        call.takeTokens(callArgs);
                        return after;
                    }
                }
                after = after->next;
            }
            return nullptr;
        }

        /** Expand while the expansion result in @p temp ends with the name of a
         *  function-like macro whose arguments are supplied by the tokens after @p tok.
         *  Each round consumes that macro call from the token stream. The @p gather
         *  callback copies the macro name and its call into its list and returns the
         *  last token of the call, or nullptr when no complete call is available (see
         *  gatherCall and gatherCallAfter). Returns the first token that was not
         *  consumed (tok->next when no call was expanded). */
        template<class GatherCall>
        const Token *expandTrailingCalls(TokenList &output, TokenList &temp, const Location &loc, const Token *tok, const MacroMap &macros, const std::set<TokenString> &expandedmacros, OutputList *outputList, GatherCall gather) const {
            while (const Macro * const calledMacro = callableMacro(temp.cback(), macros, expandedmacros)) {
                TokenList call(files);
                const Token * const last = gather(call, temp.cback(), tok);
                if (!last)
                    break;
                output.takeTokens(temp);
                output.deleteToken(output.back());
                if (last->name) {
                    // the call ended in a parameter that may hold more than the call
                    temp.takeTokens(call);
                    rescanArg(temp, loc, macros, expandedmacros, outputList);
                } else {
                    calledMacro->expand(temp, loc, call.cfront(), macros, expandedmacros, outputList);
                }
                if (temp.empty())
                    skipTrailingName(output);
                tok = last;
            }
            output.takeTokens(temp);
            return tok->next;
        }

        /** Append @p temp, the result of the replacement-list token @p tok, to
         *  @p output. When it ends with the name of a function-like macro whose call
         *  follows in the replacement list, the call is expanded, with
         *  @p callExpandedmacros when given. Returns the next replacement-list token. */
        const Token *recursiveExpandToken(TokenList &output, TokenList &temp, const Location &loc, const Token *tok, const MacroMap &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens, OutputList *outputList, const std::set<TokenString> *callExpandedmacros = nullptr) const {
            for (;;) {
                TokenList arg(files);
                const Token *argTok = nullptr;
                const Token * const next = expandTrailingCalls(output, temp, loc, tok, macros, callExpandedmacros ? *callExpandedmacros : expandedmacros, outputList, [&](TokenList &call, const Token *nameTok, const Token *tok2) {
                    return gatherCallAfter(call, nameTok, loc, tok2, macros, expandedmacros, parametertokens, outputList, arg, argTok);
                });
                if (!argTok)
                    return next;
                // The search for the call has expanded a parameter that does not
                // start it. Continue with that expansion as expandToken would.
                rescanArg(arg, loc, macros, expandedmacros, outputList);
                if (arg.empty())
                    skipTrailingName(output);
                temp.takeTokens(arg);
                tok = argTok;
                callExpandedmacros = nullptr;
            }
        }

        /** Rescan the macro argument in @p tokens after it has been substituted into
         *  the replacement list, expanding each call of a function-like macro that is
         *  complete within it. The argument pre-expansion leaves such a call behind
         *  when the macro name and its '(' were separated by tokens that expanded to
         *  nothing, as in "f EMPTY() (1)". The result of each call is rescanned
         *  against the tokens that follow it. Names of macros that are being expanded
         *  are marked so that they are never expanded later. The marks that the
         *  pre-expansion left on the tokens do not apply to this rescan. */
        void rescanArg(TokenList &tokens, const Location &loc, const MacroMap &macros, const std::set<TokenString> &expandedmacros, OutputList *outputList) const {
            // Usually the pre-expansion has left no call behind. Then the tokens are
            // only changed in place, which avoids copying them at each nesting level.
            bool hasCall = false;
            for (Token *tok = tokens.front(); tok; tok = tok->next) {
                tok->skippedByRescan = false;
                if (hasCall)
                    continue;
                hasCall = sameline(tok, tok->next) && tok->next->op == '(' && callableMacro(tok, macros, expandedmacros) != nullptr;
                if (!hasCall)
                    markExpanding(tok, macros, expandedmacros);
            }
            if (!hasCall)
                return;
            TokenList input(files);
            input.takeTokens(tokens);
            for (const Token *tok = input.cfront(); tok;) {
                TokenList temp(files);
                temp.push_back(new Token(*tok));
                markExpanding(temp.back(), macros, expandedmacros);
                tok = expandTrailingCalls(tokens, temp, loc, tok, macros, expandedmacros, outputList, [](TokenList &call, const Token *nameTok, const Token *tok2) {
                    return gatherCall(call, nameTok, tok2);
                });
            }
        }

        const Token *expandToken(TokenList &output, const Location &loc, const Token *tok, const MacroMap &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens, OutputList *outputList) const {
            // Not name..
            if (!tok->name) {
                output.push_back(newMacroToken(tok->str(), loc, true, tok));
                return tok->next;
            }

            // Macro parameter..
            {
                TokenList temp(files);
                bool hasMacroName = false;
                if (expandArg(temp, tok, loc, macros, expandedmacros, parametertokens, outputList, &hasMacroName)) {
                    if (tok->str() == "__VA_ARGS__" && temp.empty() && output.cback() && output.cback()->str() == "," &&
                        tok->nextSkipComments() && tok->nextSkipComments()->str() == ")")
                        output.deleteToken(output.back());
                    if (hasMacroName && !temp.empty()) {
                        rescanArg(temp, loc, macros, expandedmacros, outputList);
                        // the argument held only macro calls that expanded to nothing:
                        // the rescan has passed the token before it
                        if (temp.empty())
                            skipTrailingName(output);
                    }
                    return recursiveExpandToken(output, temp, loc, tok, macros, expandedmacros, parametertokens, outputList);
                }
            }

            // Macro..
            const MacroMap::const_iterator it = macros.find(tok->str());
            if (it != macros.end() && expandedmacros.find(tok->str()) == expandedmacros.end()) {
                const Macro &calledMacro = it->second;
                if (!calledMacro.functionLike()) {
                    TokenList temp(files);
                    calledMacro.expand(temp, loc, tok, macros, expandedmacros, outputList);
                    if (temp.empty())
                        skipTrailingName(output);
                    std::set<std::string> expandedmacros2(expandedmacros);
                    expandedmacros2.insert(tok->str());
                    return recursiveExpandToken(output, temp, loc, tok, macros, expandedmacros, parametertokens, outputList, &expandedmacros2);
                }
                // the call's arguments follow in the replacement list
                TokenList temp(files);
                temp.push_back(newMacroToken(tok->str(), loc, true, tok));
                return recursiveExpandToken(output, temp, loc, tok, macros, expandedmacros, parametertokens, outputList);
            }

            const Token *next = nullptr;
            if (tok->str() == DEFINED && expandDefined(output, loc, tok, macros, parametertokens, next))
                return next;

            output.push_back(newMacroToken(tok->str(), loc, true, tok));
            if (it != macros.end())
                output.back()->markExpandedFrom(&it->second);
            return tok->next;
        }

        /**
         * Find the operand of "defined" in a replacement list: "defined X", "defined(X)" or "defined X ## Y".
         * @param tok    the "defined" token
         * @param first  output: X
         * @param second output: Y for "X ## Y", otherwise nullptr
         * @param last   output: the last token of the operand
         * @return false if there is no operand
         */
        static bool findDefinedOperand(const Token *tok, const Token *&first, const Token *&second, const Token *&last) {
            const Token * const tok2 = tok->next;
            const Token * const tok3 = tok2 ? tok2->next : nullptr;
            const Token * const tok4 = tok3 ? tok3->next : nullptr;
            first = last = second = nullptr;
            if (sameline(tok, tok4) && tok2->op == '(' && tok3->name && tok4->op == ')') {
                first = tok3;
                last = tok4;
            } else if (sameline(tok,tok2) && tok2->name) {
                first = last = tok2;
            }
            if (!first)
                return false;
            if (first->next && first->next->op == '#' && first->next->next && first->next->next->op == '#' && first->next->next->next && first->next->next->next->name && sameline(first,first->next->next->next)) {
                second = first->next->next->next;
                last = second;
            }
            return true;
        }

        /**
         * The name of the macro that "defined" checks.
         * @param first   X in "defined X"
         * @param second  Y in "defined X ## Y", or nullptr
         * @param argText for "X ## Y": gives the text of a parameter, and returns false if the token is not a parameter
         */
        template<class ArgText>
        static std::string definedMacroName(const Token *first, const Token *second, ArgText argText) {
            std::string macroName = first->str();
            if (second) {
                std::string text;
                if (argText(first, text))
                    macroName = text;
                if (argText(second, text))
                    macroName += text;
                else
                    macroName += second->str();
            }
            return macroName;
        }

        /**
         * Expand "defined X" or "defined(X)" in the replacement list to 1 or 0.
         * The operand can be a parameter, or two names pasted with ##.
         * @param output          destination tokenlist
         * @param loc             location for the result
         * @param tok             the "defined" token
         * @param macros          all macros
         * @param parametertokens arguments of this macro call
         * @param next            output: the token after the operand
         * @return false if there is no operand, then nothing is added to output
         */
        bool expandDefined(TokenList &output, const Location &loc, const Token *tok, const MacroMap &macros, const std::vector<const Token*> &parametertokens, const Token *&next) const {
            const Token *first;
            const Token *second;
            const Token *last;
            if (!findDefinedOperand(tok, first, second, last))
                return false;
            TokenList temp(files);
            const std::string macroName = definedMacroName(first, second, [&](const Token *argtok, std::string &text) {
                if (!expandArg(temp, argtok, parametertokens))
                    return false;
                text = temp.cback() ? temp.cback()->str() : "";
                return true;
            });
            const bool def = (macros.find(macroName) != macros.end());
            output.push_back(newMacroToken(def ? "1" : "0", loc, true));
            next = last->next;
            return true;
        }

        bool expandArg(TokenList &output, const Token *tok, const std::vector<const Token*> &parametertokens) const {
            if (!tok->name)
                return false;

            const unsigned int argnr = getArgNum(tok->str());
            if (argnr >= args.size())
                return false;

            // empty variadic parameter
            if (variadic && argnr + 1U >= parametertokens.size())
                return true;

            for (const Token *partok = parametertokens[argnr]->next; partok != parametertokens[argnr + 1U]; partok = partok->next)
                output.push_back(new Token(*partok));

            return true;
        }

        /** Append the fully macro-expanded argument for parameter @p tok to @p output.
         *  @p hasMacroName, when given, is set when the argument held a macro name
         *  (otherwise the output cannot hold one); the caller then passes the output
         *  to rescanArg, which clears the marks that the pre-expansion left on it.
         *  Returns false when @p tok is not a parameter. */
        bool expandArg(TokenList &output, const Token *tok, const Location &loc, const MacroMap &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens, OutputList *outputList, bool *hasMacroName = nullptr) const {
            if (!tok->name)
                return false;
            const unsigned int argnr = getArgNum(tok->str());
            if (argnr >= args.size())
                return false;
            if (hasMacroName)
                *hasMacroName = false;
            if (variadic && argnr + 1U >= parametertokens.size()) // empty variadic parameter
                return true;
            const Token * const argEnd = parametertokens[argnr + 1U];
            // NOLINTNEXTLINE(misc-const-correctness) - technically correct but used to access non-const data
            Token * const outputEnd = output.back();
            bool macroName = false;
            for (const Token *partok = parametertokens[argnr]->next; partok != argEnd;) {
                const MacroMap::const_iterator it = macros.find(partok->str());
                if (it != macros.end())
                    macroName = true;
                if (it != macros.end() && !partok->isExpandedFrom(&it->second) && (partok->str() == name() || expandedmacros.find(partok->str()) == expandedmacros.end())) {
                    std::set<TokenString> expandedmacros2(expandedmacros); // temporary amnesia to allow reexpansion of currently expanding macros during argument evaluation
                    expandedmacros2.erase(name());
                    TokenList temp(files);
                    partok = it->second.expand(temp, loc, partok, macros, std::move(expandedmacros2), outputList);
                    if (partok->op == '(' && temp.cback() && temp.cback()->name) {
                        // the expansion result may end with the name of a function-like
                        // macro whose arguments are supplied by the remaining argument tokens
                        std::set<TokenString> expandedmacros3(expandedmacros);
                        expandedmacros3.erase(name());
                        partok = expandTrailingCalls(output, temp, loc, partok->previous, macros, expandedmacros3, outputList, [&](TokenList &call, const Token *nameTok, const Token *tok2) {
                            return gatherCall(call, nameTok, tok2, argEnd);
                        });
                    } else {
                        output.takeTokens(temp);
                    }
                } else {
                    output.push_back(newMacroToken(partok->str(), loc, isReplaced(expandedmacros), partok));
                    output.back()->macro = partok->macro;
                    partok = partok->next;
                }
            }
            if (tok->whitespaceahead && output.back())
                output.back()->whitespaceahead = true;
            if (macroName) {
                if (hasMacroName) {
                    *hasMacroName = true;
                } else {
                    // the substituted tokens are rescanned again, so the marks that the
                    // expansions above left on them do not apply
                    for (Token *tok2 = outputEnd ? outputEnd->next : output.front(); tok2; tok2 = tok2->next)
                        tok2->skippedByRescan = false;
                }
            }
            return true;
        }

        /**
         * Expand #X => "X"
         * @param output  destination tokenlist
         * @param loc     location for expanded token
         * @param tok     The # token
         * @param expandedmacros   set with expanded macros, with this macro
         * @param parametertokens  parameters given when expanding this macro
         * @return token after the X
         */
        const Token *expandHash(TokenList &output, const Location &loc, const Token *tok, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens, OutputList *outputList) const {
            TokenList tokenListHash(files);
            const MacroMap macros2; // temporarily bypass macro expansion
            tok = expandToken(tokenListHash, loc, tok->next, macros2, expandedmacros, parametertokens, outputList);
            output.push_back(newMacroToken(stringify(tokenListHash, loc, name(), outputList), loc, isReplaced(expandedmacros)));
            return tok;
        }

        /**
         * Expand A##B => AB
         * The A should already be expanded. Call this when you reach the first # token
         * @param output  destination tokenlist
         * @param loc     location for expanded token
         * @param tok     first # token
         * @param macros  all macros
         * @param expandedmacros   set with expanded macros, with this macro
         * @param parametertokens  parameters given when expanding this macro
         * @param expandResult     expand ## result i.e. "AB"?
         * @return token after B
         */
        const Token *expandHashHash(TokenList &output, const Location &loc, const Token *tok, const MacroMap &macros, const std::set<TokenString> &expandedmacros, const std::vector<const Token*> &parametertokens, OutputList *outputList, bool expandResult=true) const {
            Token *A = output.back();
            if (!A)
                throw invalidHashHash(tok->location, name(), "Missing first argument");
            if (!sameline(tok, tok->next) || !sameline(tok, tok->next->next))
                throw invalidHashHash::unexpectedNewline(tok->location, name());

            const bool canBeConcatenatedStringOrChar = isStringLiteral(A->str()) || isCharLiteral(A->str());

            const Token * const B = tok->next->next;
            checkHashHashOperands(tok->location, name(), A, B);

            TokenList tokensB(files);
            const Token *nextTok = B->next;

            if (canBeConcatenatedStringOrChar) {
                // It seems clearer to handle this case separately even though the code is similar-ish, but we don't want to merge here.
                // TODO The question is whether the ## or varargs may still apply, and how to provoke?
                if (expandArg(tokensB, B, parametertokens)) {
                    setLocation(tokensB, loc);
                } else {
                    tokensB.push_back(new Token(*B));
                    tokensB.back()->location = loc;
                }
                output.takeTokens(tokensB);
            } else {
                std::string strAB;

                const bool varargs = variadic && !args.empty() && B->str() == args[args.size()-1U];

                if (expandArg(tokensB, B, parametertokens)) {
                    if (tokensB.empty()) {
                        strAB = A->str();
                    }
                    else if (varargs && A->op == ',') {
                        strAB = ",";
                    }
                    else {
                        checkPaste(tok->location, name(), A, tokensB.cfront());
                        strAB = A->str() + tokensB.cfront()->str();
                        tokensB.deleteToken(tokensB.front());
                    }
                } else {
                    checkPaste(tok->location, name(), A, B);
                    strAB = A->str() + B->str();
                }

                checkUniversalCharacterPaste(tok->location, name(), A, strAB);

                if (varargs && tokensB.empty() && tok->previous->str() == ",") {
                    output.deleteToken(A);
                }
                else if (strAB != "," && macros.find(strAB) == macros.end()) {
                    A->setstr(strAB);
                    setLocation(tokensB, loc);
                    output.takeTokens(tokensB);
                } else if (sameline(B, nextTok) && sameline(B, nextTok->next) && nextTok->op == '#' && nextTok->next->op == '#') {
                    TokenList output2(files);
                    output2.push_back(new Token(strAB, tok->location));
                    nextTok = expandHashHash(output2, loc, nextTok, macros, expandedmacros, parametertokens, outputList);
                    output.deleteToken(A);
                    output.takeTokens(output2);
                } else {
                    output.deleteToken(A);
                    TokenList tokens(files);
                    tokens.push_back(new Token(strAB, tok->location));
                    // for function like macros, push the (...)
                    if (tokensB.empty() && sameline(B,B->next) && B->next->op=='(') {
                        const MacroMap::const_iterator it = macros.find(strAB);
                        if (it != macros.end() && expandedmacros.find(strAB) == expandedmacros.end() && it->second.functionLike()) {
                            const Token * const tok2 = appendTokens(tokens, loc, B->next, macros, expandedmacros, parametertokens, outputList);
                            if (tok2)
                                nextTok = tok2->next;
                        }
                    }
                    if (expandResult)
                        expandToken(output, loc, tokens.cfront(), macros, expandedmacros, parametertokens, outputList);
                    else
                        output.takeTokens(tokens);
                    setLocation(tokensB, loc);
                    output.takeTokens(tokensB);
                }
            }

            return nextTok;
        }

        /** A is a punctuator, not a name, number or literal. Pasting it is checked after B is expanded. */
        static bool isPunctuatorOperand(const Token *A) {
            return !A->name && !A->number && !A->str().empty() && !isStringLiteral(A->str()) && !isCharLiteral(A->str());
        }

        /**
         * Check the operands of A ## B before B is expanded.
         * @param loc       the location of the ##
         * @param macroName the macro that is expanded
         * @param A         the token before ##, already expanded
         * @param B         the token after ## in the macro definition
         * @throws invalidHashHash if A and B can not be pasted
         */
        static void checkHashHashOperands(const Location &loc, const std::string &macroName, const Token *A, const Token *B) {
            if (!isPunctuatorOperand(A)) {
                if (!B->name && !B->number && B->op && !B->isOneOf("#="))
                    throw invalidHashHash::unexpectedToken(loc, macroName, B);
                if (B->op == '=')
                    throw invalidHashHash::cannotCombine(loc, macroName, A, B);
            }

            // Superficial check; more in-depth would in theory be possible _after_ expandArg
            if ((isStringLiteral(A->str()) || isCharLiteral(A->str())) && (B->number || !B->name))
                throw invalidHashHash::cannotCombine(loc, macroName, A, B);
        }

        /**
         * Check that pasting A and B gives a valid token, when A is a punctuator
         * or B is a string or character literal.
         * @param loc       the location of the ##
         * @param macroName the macro that is expanded
         * @param A         the token before ##
         * @param B         the first token after ##, after expansion
         * @throws invalidHashHash if the result is not a valid token
         */
        static void checkPaste(const Location &loc, const std::string &macroName, const Token *A, const Token *B) {
            if (isPunctuatorOperand(A) && !isValidPunctuatorPaste(A->str() + B->str()))
                throw invalidHashHash::cannotCombine(loc, macroName, A, B);
            // a name or number followed by a literal: only an encoding prefix such as L or u8 is valid
            if (A->name || A->number) {
                const std::string &b = B->str();
                const std::string::size_type quote = b.find_first_of("\"\'");
                if (quote == std::string::npos || b.size() < quote + 2 || b.back() != b[quote] ||
                    (quote > 0 && !isStringLiteralPrefix(b.substr(0, quote))))
                    return;
                const bool isChar = b[quote] == '\'';
                const bool rawChar = isChar && A->str().back() == 'R';
                const bool validPrefix = A->name && quote == 0 && isStringLiteralPrefix(A->str());
                if (!validPrefix || rawChar)
                    throw invalidHashHash::cannotCombine(loc, macroName, A, B);
            }
        }

        /**
         * Producing a universal character name with ## is undefined behavior.
         * @param loc       the location of the ##
         * @param macroName the macro that is expanded
         * @param A         the token before ##
         * @param strAB     the pasted text
         * @throws invalidHashHash if A follows a backslash and strAB is a universal character name
         */
        static void checkUniversalCharacterPaste(const Location &loc, const std::string &macroName, const Token *A, const std::string &strAB) {
            if (!A->previous || A->previous->str() != "\\")
                return;
            if ((strAB[0] == 'u' && strAB.size() == 5) || (strAB[0] == 'U' && strAB.size() == 9))
                throw invalidHashHash::universalCharacterUB(loc, macroName, A, strAB);
        }

        static bool isReplaced(const std::set<std::string> &expandedmacros) {
            // return true if size > 1
            auto it = expandedmacros.cbegin();
            if (it == expandedmacros.cend())
                return false;
            ++it;
            return (it != expandedmacros.cend());
        }

        /** name token in definition */
        const Token *nameTokDef;

        /** arguments for macro */
        std::vector<TokenString> args;

        /** first token in replacement string */
        const Token *valueToken;

        /** token after replacement string */
        const Token *endToken;

        /** files */
        std::vector<std::string> &files;

        /** this is used for -D where the definition is not seen anywhere in code */
        TokenList tokenListDefine;

        /** usage of this macro */
        mutable std::list<Usage> usageList;

        /** is macro variadic? */
        bool variadic;

        /** does the macro expansion have __VA_OPT__? */
        bool variadicOpt;

        /** Expansion value for varadic macros with __VA_OPT__ expanded and discarded respectively */
        const TokenList *optExpandValue{};
        const TokenList *optNoExpandValue{};

        /** was the value of this macro actually defined in the code? */
        bool valueDefinedInCode_;
    };
}

namespace simplecpp {

#ifdef __CYGWIN__
    static bool startsWith(const std::string &s, const std::string &p)
    {
        return (s.size() >= p.size()) && std::equal(p.begin(), p.end(), s.begin());
    }

    std::string convertCygwinToWindowsPath(const std::string &cygwinPath)
    {
        std::string windowsPath;

        std::string::size_type pos = 0;
        if (cygwinPath.size() >= 11 && startsWith(cygwinPath, "/cygdrive/")) {
            const unsigned char driveLetter = cygwinPath[10];
            if (std::isalpha(driveLetter)) {
                if (cygwinPath.size() == 11) {
                    windowsPath = toupper(driveLetter);
                    windowsPath += ":\\";   // volume root directory
                    pos = 11;
                } else if (cygwinPath[11] == '/') {
                    windowsPath = toupper(driveLetter);
                    windowsPath += ":";
                    pos = 11;
                }
            }
        }

        for (; pos < cygwinPath.size(); ++pos) {
            unsigned char c = cygwinPath[pos];
            if (c == '/')
                c = '\\';
            windowsPath += c;
        }

        return windowsPath;
    }
#endif

    bool isAbsolutePath(const std::string &path)
    {
#ifdef SIMPLECPP_WINDOWS
        // C:\\path\\file
        // C:/path/file
        if (path.length() >= 3 && std::isalpha(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/'))
            return true;

        // \\host\path\file
        // //host/path/file
        if (path.length() >= 2 && (path[0] == '\\' || path[0] == '/') && (path[1] == '\\' || path[1] == '/'))
            return true;

        return false;
#else
        return !path.empty() && path[0] == '/';
#endif
    }
}

namespace simplecpp {
    /**
     * perform path simplifications for . and ..
     */
    std::string simplifyPath(std::string path)
    {
        if (path.empty())
            return path;

        std::string::size_type pos;

        // replace backslash separators
        std::replace(path.begin(), path.end(), '\\', '/');

        const bool unc(path.compare(0,2,"//") == 0);

        // replace "//" with "/"
        pos = 0;
        while ((pos = path.find("//",pos)) != std::string::npos) {
            path.erase(pos,1);
        }

        // remove "./"
        pos = 0;
        while ((pos = path.find("./",pos)) != std::string::npos) {
            if (pos == 0 || path[pos - 1U] == '/')
                path.erase(pos,2);
            else
                pos += 2;
        }

        // remove trailing dot if path ends with "/."
        if (endsWith(path,"/."))
            path.erase(path.size()-1);

        // simplify ".."
        pos = 1; // don't simplify ".." if path starts with that
        while ((pos = path.find("/..", pos)) != std::string::npos) {
            // not end of path, then string must be "/../"
            if (pos + 3 < path.size() && path[pos + 3] != '/') {
                ++pos;
                continue;
            }
            // get previous subpath
            std::string::size_type pos1 = path.rfind('/', pos - 1U);
            if (pos1 == std::string::npos) {
                pos1 = 0;
            } else {
                pos1 += 1U;
            }
            const std::string previousSubPath = path.substr(pos1, pos - pos1);
            if (previousSubPath == "..") {
                // don't simplify
                ++pos;
            } else {
                // remove previous subpath and ".."
                path.erase(pos1, pos - pos1 + 4);
                if (path.empty())
                    path = ".";
                // update pos
                pos = (pos1 == 0) ? 1 : (pos1 - 1);
            }
        }

        // Remove trailing '/'?
        //if (path.size() > 1 && endsWith(path, "/"))
        //    path.erase(path.size()-1);

        if (unc)
            path = '/' + path;

        return path;
    }
}

/** Evaluate sizeof(type)
 * @throws std::runtime_error thrown on missing arguments or invalid expression
 */
static void simplifySizeof(simplecpp::TokenList &expr, const std::map<std::string, std::size_t> &sizeOfType)
{
    for (simplecpp::Token *tok = expr.front(); tok; tok = tok->next) {
        if (tok->str() != "sizeof")
            continue;
        const simplecpp::Token *tok1 = tok->next;
        if (!tok1) {
            throw std::runtime_error("missing sizeof argument");
        }
        const simplecpp::Token *tok2 = tok1->next;
        if (!tok2) {
            throw std::runtime_error("missing sizeof argument");
        }
        if (tok1->op == '(') {
            tok1 = tok1->next;
            while (tok2->op != ')') {
                tok2 = tok2->next;
                if (!tok2) {
                    throw std::runtime_error("invalid sizeof expression");
                }
            }
        }

        std::string type;
        for (const simplecpp::Token *typeToken = tok1; typeToken != tok2; typeToken = typeToken->next) {
            if ((typeToken->str() == "unsigned" || typeToken->str() == "signed") && typeToken->next->name)
                continue;
            if (typeToken->str() == "*" && type.find('*') != std::string::npos)
                continue;
            if (!type.empty())
                type += ' ';
            type += typeToken->str();
        }

        const std::map<std::string, std::size_t>::const_iterator it = sizeOfType.find(type);
        if (it != sizeOfType.end())
            tok->setstr(toString(it->second));
        else
            continue;

        tok2 = tok2->next;
        while (tok->next != tok2)
            expr.deleteToken(tok->next);
    }
}

static bool isCpp17OrLater(const simplecpp::DUI &dui)
{
    const std::string std_ver = simplecpp::getCppStdString(dui.std);
    return std_ver.empty() || (std_ver >= "201703L");
}

static bool isGnu(const simplecpp::DUI &dui)
{
    return dui.std.rfind("gnu", 0) != std::string::npos;
}

static std::string dirPath(const std::string& path, bool withTrailingSlash=true)
{
    const std::size_t lastSlash = path.find_last_of("\\/");
    if (lastSlash == std::string::npos) {
        return "";
    }
    return path.substr(0, lastSlash + (withTrailingSlash ? 1U : 0U));
}

static std::string openHeader(std::ifstream &f, const simplecpp::DUI &dui, const std::string &sourcefile, const std::string &header, bool systemheader);

/** Evaluate __has_include(include)
 * @throws std::runtime_error thrown on missing arguments or invalid expression
 */
static void simplifyHasInclude(simplecpp::TokenList &expr, const simplecpp::DUI &dui)
{
    if (!isCpp17OrLater(dui) && !isGnu(dui))
        return;

    for (simplecpp::Token *tok = expr.front(); tok; tok = tok->next) {
        if (tok->str() != HAS_INCLUDE)
            continue;
        const simplecpp::Token *tok1 = tok->next;
        if (!tok1) {
            throw std::runtime_error("missing __has_include argument");
        }
        const simplecpp::Token *tok2 = tok1->next;
        if (!tok2) {
            throw std::runtime_error("missing __has_include argument");
        }
        if (tok1->op == '(') {
            tok1 = tok1->next;
            while (tok2->op != ')') {
                tok2 = tok2->next;
                if (!tok2) {
                    throw std::runtime_error("invalid __has_include expression");
                }
            }
        }

        const std::string &sourcefile = expr.file(tok->location);
        const bool systemheader = (tok1 && tok1->op == '<');
        std::string header;
        if (systemheader) {
            const simplecpp::Token *tok3 = tok1->next;
            if (!tok3) {
                throw std::runtime_error("missing __has_include closing angular bracket");
            }
            while (tok3->op != '>') {
                tok3 = tok3->next;
                if (!tok3) {
                    throw std::runtime_error("invalid __has_include expression");
                }
            }

            for (const simplecpp::Token *headerToken = tok1->next; headerToken != tok3; headerToken = headerToken->next)
                header += headerToken->str();
        } else {
            header = tok1->str().substr(1U, tok1->str().size() - 2U);
        }
        std::ifstream f;
        const std::string header2 = openHeader(f,dui,sourcefile,header,systemheader);
        tok->setstr(header2.empty() ? "0" : "1");

        tok2 = tok2->next;
        while (tok->next != tok2)
            expr.deleteToken(tok->next);
    }
}

/** Evaluate name
 * @throws std::runtime_error thrown on undefined function-like macro
 */
static void simplifyName(simplecpp::TokenList &expr, const simplecpp::DUI &dui)
{
    // In C++ and C23 the keywords true/false are 1/0 in #if expressions
    const simplecpp::cstd_t cstd = simplecpp::getCStd(dui.std);
    const bool hasTrueFalse = (cstd == simplecpp::CUnknown || cstd >= simplecpp::C23);

    for (simplecpp::Token *tok = expr.front(); tok; tok = tok->next) {
        if (tok->name) {
            if (hasTrueFalse && (tok->str() == "true" || tok->str() == "false")) {
                tok->setstr(tok->str() == "true" ? "1" : "0");
                continue;
            }
            if (isAlternativeOpName(tok)) {
                bool alt;
                if (tok->str() == "not" || tok->str() == "compl") {
                    alt = isAlternativeUnaryOp(tok,tok->str());
                } else {
                    alt = isAlternativeBinaryOp(tok,tok->str());
                }
                if (alt)
                    continue;
            }
            if (tok->next && tok->next->str() == "(")
                throw std::runtime_error("undefined function-like macro invocation: " + tok->str() + "( ... )");
            tok->setstr("0");
        }
    }
}

/*
 * Reads at least minlen and at most maxlen digits (inc. prefix) in base base
 * from s starting at position pos and converts them to a
 * unsigned long long value, updating pos to point to the first
 * unused element of s.
 * Returns ULLONG_MAX if the result is not representable and
 * @throws std::runtime_error thrown if the above requirements were not possible to satisfy.
 */
static unsigned long long stringToULLbounded(
    const std::string& s,
    std::size_t& pos,
    int base = 0,
    std::ptrdiff_t minlen = 1,
    std::size_t maxlen = std::string::npos
    )
{
    const std::string sub = s.substr(pos, maxlen);
    const char * const start = sub.c_str();
    char* end;
    const unsigned long long value = std::strtoull(start, &end, base);
    pos += end - start;
    if (end - start < minlen)
        throw std::runtime_error("expected digit");
    return value;
}

long long simplecpp::characterLiteralToLL(const std::string& str)
{
    // default is wide/utf32
    bool narrow = false;
    bool utf8 = false;
    bool utf16 = false;

    std::size_t pos;

    if (!str.empty() && str[0] == '\'') {
        narrow = true;
        pos = 1;
    } else if (str.size() >= 2 && str[0] == 'u' && str[1] == '\'') {
        utf16 = true;
        pos = 2;
    } else if (str.size() >= 3 && str[0] == 'u' && str[1] == '8' && str[2] == '\'') {
        utf8 = true;
        pos = 3;
    } else if (str.size() >= 2 && (str[0] == 'L' || str[0] == 'U') && str[1] == '\'') {
        pos = 2;
    } else {
        throw std::runtime_error("expected a character literal");
    }

    unsigned long long multivalue = 0;

    std::size_t nbytes = 0;

    while (pos + 1 < str.size()) {
        if (str[pos] == '\'' || str[pos] == '\n')
            throw std::runtime_error("raw single quotes and newlines not allowed in character literals");

        if (nbytes >= 1 && !narrow)
            throw std::runtime_error("multiple characters only supported in narrow character literals");

        unsigned long long value;

        if (str[pos] == '\\') {
            pos++;
            const char escape = str[pos++];

            if (pos >= str.size())
                throw std::runtime_error("unexpected end of character literal");

            switch (escape) {
            // obscure GCC extensions
            case '%':
            case '(':
            case '[':
            case '{':
            // standard escape sequences
            case '\'':
            case '"':
            case '?':
            case '\\':
                value = static_cast<unsigned char>(escape);
                break;

            case 'a':
                value = static_cast<unsigned char>('\a');
                break;
            case 'b':
                value = static_cast<unsigned char>('\b');
                break;
            case 'f':
                value = static_cast<unsigned char>('\f');
                break;
            case 'n':
                value = static_cast<unsigned char>('\n');
                break;
            case 'r':
                value = static_cast<unsigned char>('\r');
                break;
            case 't':
                value = static_cast<unsigned char>('\t');
                break;
            case 'v':
                value = static_cast<unsigned char>('\v');
                break;

            // GCC extension for ESC character
            case 'e':
            case 'E':
                value = static_cast<unsigned char>('\x1b');
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                // octal escape sequences consist of 1 to 3 digits
                value = stringToULLbounded(str, --pos, 8, 1, 3);
                break;

            case 'x':
                // hexadecimal escape sequences consist of at least 1 digit
                value = stringToULLbounded(str, pos, 16);
                break;

            case 'u':
            case 'U': {
                // universal character names have exactly 4 or 8 digits
                const std::size_t ndigits = (escape == 'u' ? 4 : 8);
                value = stringToULLbounded(str, pos, 16, ndigits, ndigits);

                // UTF-8 encodes code points above 0x7f in multiple code units
                // code points above 0x10ffff are not allowed
                if (((narrow || utf8) && value > 0x7f) || (utf16 && value > 0xffff) || value > 0x10ffff)
                    throw std::runtime_error("code point too large");

                if (value >= 0xd800 && value <= 0xdfff)
                    throw std::runtime_error("surrogate code points not allowed in universal character names");

                break;
            }

            default:
                throw std::runtime_error("invalid escape sequence");
            }
        } else {
            value = static_cast<unsigned char>(str[pos++]);

            if (!narrow && value >= 0x80) {
                // Assuming this is a UTF-8 encoded code point.
                // This decoder may not completely validate the input.
                // Noncharacters are neither rejected nor replaced.

                int additional_bytes;
                if (value >= 0xf5)  // higher values would result in code points above 0x10ffff
                    throw std::runtime_error("assumed UTF-8 encoded source, but sequence is invalid");
                if (value >= 0xf0)
                    additional_bytes = 3;
                else if (value >= 0xe0)
                    additional_bytes = 2;
                else if (value >= 0xc2) // 0xc0 and 0xc1 are always overlong 2-bytes encodings
                    additional_bytes = 1;
                else
                    throw std::runtime_error("assumed UTF-8 encoded source, but sequence is invalid");

                value &= (1 << (6 - additional_bytes)) - 1;

                while (additional_bytes--) {
                    if (pos + 1 >= str.size())
                        throw std::runtime_error("assumed UTF-8 encoded source, but character literal ends unexpectedly");

                    const unsigned char c = str[pos++];

                    if (((c >> 6) != 2)    // ensure c has form 0xb10xxxxxx
                        || (!value && additional_bytes == 1 && c < 0xa0)    // overlong 3-bytes encoding
                        || (!value && additional_bytes == 2 && c < 0x90))   // overlong 4-bytes encoding
                        throw std::runtime_error("assumed UTF-8 encoded source, but sequence is invalid");

                    value = (value << 6) | (c & ((1 << 7) - 1));
                }

                if (value >= 0xd800 && value <= 0xdfff)
                    throw std::runtime_error("assumed UTF-8 encoded source, but sequence is invalid");

                if ((utf8 && value > 0x7f) || (utf16 && value > 0xffff) || value > 0x10ffff)
                    throw std::runtime_error("code point too large");
            }
        }

        if (((narrow || utf8) && value > std::numeric_limits<unsigned char>::max()) || (utf16 && value >> 16) || value >> 32)
            throw std::runtime_error("numeric escape sequence too large");

        multivalue <<= CHAR_BIT;
        multivalue |= value;
        nbytes++;
    }

    if (pos + 1 != str.size() || str[pos] != '\'')
        throw std::runtime_error("missing closing quote in character literal");

    if (!nbytes)
        throw std::runtime_error("empty character literal");

    // ordinary narrow character literal's value is determined by (possibly signed) char
    if (narrow && nbytes == 1)
        return static_cast<char>(multivalue);

    // while multi-character literal's value is determined by (signed) int
    if (narrow)
        return static_cast<int>(multivalue);

    // All other cases are unsigned. Since long long is at least 64bit wide,
    // while the literals at most 32bit wide, the conversion preserves all values.
    return multivalue;
}

static bool isFloatingPointLiteral(const std::string &str)
{
    const bool startsWithDotDigit = str.size() > 1U && str[0] == '.' &&
                                    std::isdigit(static_cast<unsigned char>(str[1]));
    if (!simplecpp::Token::isNumberLike(str) && !startsWithDotDigit)
        return false;

    if (str.find('.') != std::string::npos)
        return true;

    return str.find_first_of(isHex(str) ? "pP" : "eE") != std::string::npos;
}

/**
 * @throws std::runtime_error thrown on invalid literal
 */
static void simplifyNumbers(simplecpp::TokenList &expr)
{
    for (simplecpp::Token *tok = expr.front(); tok; tok = tok->next) {
        if (isFloatingPointLiteral(tok->str()))
            throw std::runtime_error("floating point literal in preprocessor expression");
        if (tok->str().size() == 1U)
            continue;
        if (tok->str().compare(0,2,"0x") == 0)
            tok->setstr(toString(stringToULL(tok->str())));
        else if (!tok->number && tok->str().find('\'') != std::string::npos)
            tok->setstr(toString(simplecpp::characterLiteralToLL(tok->str())));
    }
}

static void simplifyComments(simplecpp::TokenList &expr)
{
    for (simplecpp::Token *tok = expr.front(); tok;) {
        simplecpp::Token * const d = tok;
        tok = tok->next;
        if (d->comment)
            expr.deleteToken(d);
    }
}

/**
 * @throws std::runtime_error thrown on invalid literals, missing sizeof arguments or invalid expressions,
 * missing __has_include() arguments or expressions, undefined function-like macros, invalid number literals
 * @throws std::overflow_error thrown on overflow or division by zero
 */
static long long evaluate(simplecpp::TokenList &expr, const simplecpp::DUI &dui, const std::map<std::string, std::size_t> &sizeOfType)
{
    simplifyComments(expr);
    simplifySizeof(expr, sizeOfType);
    simplifyHasInclude(expr, dui);
    simplifyName(expr, dui);
    simplifyNumbers(expr);
    expr.constFold();
    if (!expr.cfront() || expr.cfront() != expr.cback() || !expr.cfront()->number)
        throw std::runtime_error("invalid expression");
    return stringToLL(expr.cfront()->str());
}

static const simplecpp::Token *gotoNextLine(const simplecpp::Token *tok)
{
    const unsigned int line = tok->location.line;
    const unsigned int file = tok->location.fileIndex;
    while (tok && tok->location.line == line && tok->location.fileIndex == file)
        tok = tok->next;
    return tok;
}

#ifdef SIMPLECPP_WINDOWS

class NonExistingFilesCache {
public:
    NonExistingFilesCache() {}

    bool contains(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return (m_pathSet.find(path) != m_pathSet.end());
    }

    void add(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pathSet.insert(path);
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pathSet.clear();
    }

private:
    std::set<std::string> m_pathSet;
    std::mutex m_mutex;
};

static NonExistingFilesCache nonExistingFilesCache;

#endif

static std::string openHeaderDirect(std::ifstream &f, const std::string &path)
{
#ifdef SIMPLECPP_WINDOWS
    if (nonExistingFilesCache.contains(path))
        return "";  // file is known not to exist, skip expensive file open call
#endif
    f.open(path.c_str());
    if (f.is_open())
        return path;
#ifdef SIMPLECPP_WINDOWS
    nonExistingFilesCache.add(path);
#endif
    return "";
}

static std::string openHeader(std::ifstream &f, const simplecpp::DUI &dui, const std::string &sourcefile, const std::string &header, bool systemheader)
{
    if (simplecpp::isAbsolutePath(header))
        return openHeaderDirect(f, simplecpp::simplifyPath(header));

    // prefer first to search the header relatively to source file if found, when not a system header
    if (!systemheader) {
        std::string path = openHeaderDirect(f, simplecpp::simplifyPath(dirPath(sourcefile) + header));
        if (!path.empty()) {
            return path;
        }
    }

    // search the header on the include paths (provided by the flags "-I...")
    for (const auto &includePath : dui.includePaths) {
        std::string path = openHeaderDirect(f, simplecpp::simplifyPath(includePath + "/" + header));
        if (!path.empty())
            return path;
    }
    return "";
}

namespace {
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
}

struct simplecpp::FileDataCache::Impl
{
    void clear()
    {
        mIdMap.clear();
    }

    using id_map_type = std::unordered_map<FileID, FileData *, FileID::Hasher>;

    id_map_type mIdMap;
};

simplecpp::FileDataCache::FileDataCache()
    : mImpl(new Impl)
{}

simplecpp::FileDataCache::~FileDataCache() = default;
simplecpp::FileDataCache::FileDataCache(FileDataCache &&) SIMPLECPP_NOEXCEPT = default;
simplecpp::FileDataCache &simplecpp::FileDataCache::operator=(simplecpp::FileDataCache &&) SIMPLECPP_NOEXCEPT = default;

static bool getFileId(const std::string &path, FileID &id);

std::pair<simplecpp::FileData *, bool> simplecpp::FileDataCache::tryload(FileDataCache::name_map_type::iterator &name_it, const simplecpp::DUI &dui, std::vector<std::string> &filenames, simplecpp::OutputList *outputList)
{
    const std::string &path = name_it->first;
    FileID fileId;

    if (!getFileId(path, fileId))
        return {nullptr, false};

    const auto id_it = mImpl->mIdMap.find(fileId);
    if (id_it != mImpl->mIdMap.end()) {
        name_it->second = id_it->second;
        return {id_it->second, false};
    }

    auto *const data = new FileData {path, TokenList(path, filenames, dui, outputList)};

    if (dui.removeComments)
        data->tokens.removeComments();

    name_it->second = data;
    mImpl->mIdMap.emplace(fileId, data);
    mData.emplace_back(data);

    return {data, true};
}

std::pair<simplecpp::FileData *, bool> simplecpp::FileDataCache::get_private(const std::string &sourcefile, const std::string &header, const simplecpp::DUI &dui, bool systemheader, std::vector<std::string> &filenames, simplecpp::OutputList *outputList)
{
    if (isAbsolutePath(header)) {
        auto ins = mNameMap.emplace(simplecpp::simplifyPath(header), nullptr);

        if (ins.second) {
            const auto ret = tryload(ins.first, dui, filenames, outputList);
            if (ret.first != nullptr) {
                return ret;
            }
        } else {
            return {ins.first->second, false};
        }

        return {nullptr, false};
    }

    if (!systemheader) {
        auto ins = mNameMap.emplace(simplecpp::simplifyPath(dirPath(sourcefile) + header), nullptr);

        if (ins.second) {
            const auto ret = tryload(ins.first, dui, filenames, outputList);
            if (ret.first != nullptr) {
                return ret;
            }
        } else if (ins.first->second != nullptr) {
            return {ins.first->second, false};
        }
    }

    for (const auto &includePath : dui.includePaths) {
        auto ins = mNameMap.emplace(simplecpp::simplifyPath(includePath + "/" + header), nullptr);

        if (ins.second) {
            const auto ret = tryload(ins.first, dui, filenames, outputList);
            if (ret.first != nullptr) {
                return ret;
            }
        } else if (ins.first->second != nullptr) {
            return {ins.first->second, false};
        }
    }

    return {nullptr, false};
}

std::pair<simplecpp::FileData *, bool> simplecpp::FileDataCache::get(const std::string &sourcefile, const std::string &header, const simplecpp::DUI &dui, bool systemheader, std::vector<std::string> &filenames, simplecpp::OutputList *outputList)
{
    auto ret = get_private(sourcefile, header, dui, systemheader, filenames, outputList);

    if (mLoadCallback && ret.first)
        mLoadCallback(*ret.first, ret.second);

    return ret;
}

void simplecpp::FileDataCache::clear()
{
    mImpl->clear();
    mNameMap.clear();
    mData.clear();
}

static bool getFileId(const std::string &path, FileID &id)
{
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    BOOL ret = GetFileInformationByHandleEx(hFile, FileIdInfo, &id.fileIdInfo, sizeof(id.fileIdInfo));
    if (!ret) {
        const DWORD err = GetLastError();
        if (err == ERROR_INVALID_PARAMETER || // encountered when using a non-NTFS filesystem e.g. exFAT
            err == ERROR_NOT_SUPPORTED) // encountered on Windows Server Core (used as a Docker container)
        {
            BY_HANDLE_FILE_INFORMATION fileInfo;
            ret = GetFileInformationByHandle(hFile, &fileInfo);
            if (ret) {
                id.fileIdInfo.VolumeSerialNumber = static_cast<std::uint64_t>(fileInfo.dwVolumeSerialNumber);
                id.fileIdInfo.FileId.IdentifierHi = static_cast<std::uint64_t>(fileInfo.nFileIndexHigh);
                id.fileIdInfo.FileId.IdentifierLo = static_cast<std::uint64_t>(fileInfo.nFileIndexLow);
            }
        }
    }

    CloseHandle(hFile);

    return ret == TRUE;
#else
    struct stat statbuf;

    if (stat(path.c_str(), &statbuf) != 0)
        return false;

    id.dev = statbuf.st_dev;
    id.ino = statbuf.st_ino;

    return true;
#endif
}

simplecpp::FileDataCache simplecpp::load(const simplecpp::TokenList &rawtokens, std::vector<std::string> &filenames, const simplecpp::DUI &dui, simplecpp::OutputList *outputList, FileDataCache cache)
{
#ifdef SIMPLECPP_WINDOWS
    if (dui.clearIncludeCache)
        nonExistingFilesCache.clear();
#endif

    std::list<const Token *> filelist;

    // -include files
    for (auto it = dui.includes.cbegin(); it != dui.includes.cend(); ++it) {
        const std::string &filename = *it;

        const auto loadResult = cache.get("", filename, dui, false, filenames, outputList);
        const bool loaded = loadResult.second;
        FileData *const filedata = loadResult.first;

        if (filedata == nullptr) {
            if (outputList) {
                simplecpp::Output err{
                    simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND,
                    {},
                    "Can not open include file '" + filename + "' that is explicitly included."
                };
                outputList->emplace_back(std::move(err));
            }
            continue;
        }

        if (!loaded)
            continue;

        if (!filedata->tokens.front())
            continue;

        if (dui.removeComments)
            filedata->tokens.removeComments();

        filelist.emplace_back(filedata->tokens.front());
    }

    for (const Token *rawtok = rawtokens.cfront(); rawtok || !filelist.empty(); rawtok = rawtok ? rawtok->next : nullptr) {
        if (rawtok == nullptr) {
            rawtok = filelist.back();
            filelist.pop_back();
        }

        if (rawtok->op != '#' || sameline(rawtok->previousSkipComments(), rawtok))
            continue;

        rawtok = rawtok->nextSkipComments();
        if (!rawtok || rawtok->str() != INCLUDE)
            continue;

        const std::string &sourcefile = rawtokens.file(rawtok->location);

        const Token * const htok = rawtok->nextSkipComments();
        if (!sameline(rawtok, htok))
            continue;

        const bool systemheader = (htok->str()[0] == '<');
        const std::string header(htok->str().substr(1U, htok->str().size() - 2U));

        const auto loadResult = cache.get(sourcefile, header, dui, systemheader, filenames, outputList);
        const bool loaded = loadResult.second;

        if (!loaded)
            continue;

        FileData *const filedata = loadResult.first;

        if (!filedata->tokens.front())
            continue;

        if (dui.removeComments)
            filedata->tokens.removeComments();

        filelist.emplace_back(filedata->tokens.front());
    }

    return cache;
}

static bool preprocessToken(simplecpp::TokenList &output, const simplecpp::Token *&tok1, simplecpp::MacroMap &macros, std::vector<std::string> &files, simplecpp::OutputList *outputList, const simplecpp::DUI &dui)
{
    const simplecpp::Token * const tok = tok1;
    const simplecpp::MacroMap::const_iterator it = tok->name ? macros.find(tok->str()) : macros.end();
    if (it != macros.end()) {
        simplecpp::TokenList value(files);
        try {
            tok1 = it->second.expand(value, tok, macros, files, outputList);
        } catch (const simplecpp::Macro::invalidDirectiveAsMacroParameter &err) {
            if (outputList) {
                simplecpp::Output out{
                    simplecpp::Output::DIRECTIVE_AS_MACRO_PARAMETER,
                    err.location,
                    "failed to expand \'" + tok->str() + "\', " + err.what
                };
                outputList->emplace_back(std::move(out));
            }
            return false;
        } catch (const simplecpp::Macro::Error &err) {
            if (outputList) {
                simplecpp::Output out{
                    simplecpp::Output::SYNTAX_ERROR,
                    err.location,
                    "failed to expand \'" + tok->str() + "\', " + err.what
                };
                outputList->emplace_back(std::move(out));
            }
            return false;
        }
        output.takeTokens(value);
    } else {
        if (!tok->comment || !dui.removeComments)
            output.push_back(new simplecpp::Token(*tok));
        tok1 = tok->next;
    }
    return true;
}

static void getLocaltime(struct tm &ltime)
{
    time_t t;
    time(&t);
#ifndef _WIN32
    // NOLINTNEXTLINE(misc-include-cleaner) - false positive
    localtime_r(&t, &ltime);
#else
    localtime_s(&ltime, &t);
#endif
}

static std::string getDateDefine(const struct tm *timep)
{
    char buf[] = "??? ?? ????";
    strftime(buf, sizeof(buf), "%b %d %Y", timep);
    return std::string("\"").append(buf).append("\"");
}

static std::string getTimeDefine(const struct tm *timep)
{
    char buf[] = "??:??:??";
    strftime(buf, sizeof(buf), "%H:%M:%S", timep);
    return std::string("\"").append(buf).append("\"");
}

void simplecpp::preprocess(simplecpp::TokenList &output, const simplecpp::TokenList &rawtokens, std::vector<std::string> &files, simplecpp::FileDataCache &cache, const simplecpp::DUI &dui, simplecpp::OutputList *outputList, std::list<simplecpp::MacroUsage> *macroUsage, std::list<simplecpp::IfCond> *ifCond)
{
#ifdef SIMPLECPP_WINDOWS
    if (dui.clearIncludeCache)
        nonExistingFilesCache.clear();
#endif

    std::map<std::string, std::size_t> sizeOfType(rawtokens.sizeOfType);
    sizeOfType.insert(std::make_pair("char", sizeof(char)));
    sizeOfType.insert(std::make_pair("short", sizeof(short)));
    sizeOfType.insert(std::make_pair("short int", sizeof(short)));
    sizeOfType.insert(std::make_pair("int", sizeof(int)));
    sizeOfType.insert(std::make_pair("long", sizeof(long)));
    sizeOfType.insert(std::make_pair("long int", sizeof(long)));
    sizeOfType.insert(std::make_pair("long long", sizeof(long long)));
    sizeOfType.insert(std::make_pair("float", sizeof(float)));
    sizeOfType.insert(std::make_pair("double", sizeof(double)));
    sizeOfType.insert(std::make_pair("long double", sizeof(long double)));
    sizeOfType.insert(std::make_pair("char *", sizeof(char *)));
    sizeOfType.insert(std::make_pair("short *", sizeof(short *)));
    sizeOfType.insert(std::make_pair("short int *", sizeof(short *)));
    sizeOfType.insert(std::make_pair("int *", sizeof(int *)));
    sizeOfType.insert(std::make_pair("long *", sizeof(long *)));
    sizeOfType.insert(std::make_pair("long int *", sizeof(long *)));
    sizeOfType.insert(std::make_pair("long long *", sizeof(long long *)));
    sizeOfType.insert(std::make_pair("float *", sizeof(float *)));
    sizeOfType.insert(std::make_pair("double *", sizeof(double *)));
    sizeOfType.insert(std::make_pair("long double *", sizeof(long double *)));

    // use a dummy vector for the macros because as this is not part of the file and would add an empty entry - e.g. /usr/include/poll.h
    std::vector<std::string> dummy;

    const bool hasInclude = isCpp17OrLater(dui) || isGnu(dui);
    MacroMap macros;
    bool strictAnsiDefined = false;
    for (auto it = dui.defines.cbegin(); it != dui.defines.cend(); ++it) {
        const std::string &macrostr = *it;
        const std::string::size_type eq = macrostr.find('=');
        const std::string::size_type par = macrostr.find('(');
        const std::string macroname = macrostr.substr(0, std::min(eq,par));
        if (macroname == "__STRICT_ANSI__")
            strictAnsiDefined = true;
        if (dui.undefined.find(macroname) != dui.undefined.end())
            continue;
        const std::string lhs(macrostr.substr(0,eq));
        const std::string rhs(eq==std::string::npos ? std::string("1") : macrostr.substr(eq+1));
        try {
            const Macro macro(lhs, rhs, dummy);
            macros.insert(std::pair<TokenString,Macro>(macro.name(), macro));
        } catch (const std::runtime_error& e) {
            if (outputList) {
                simplecpp::Output err{
                    Output::DUI_ERROR,
                    {},
                    e.what()
                };
                outputList->emplace_back(std::move(err));
            }
            output.clear();
            return;
        } catch (const simplecpp::Macro::Error& e) {
            if (outputList) {
                simplecpp::Output err{
                    Output::DUI_ERROR,
                    {},
                    e.what
                };
                outputList->emplace_back(std::move(err));
            }
            output.clear();
            return;
        }
    }

    const bool strictAnsiUndefined = dui.undefined.find("__STRICT_ANSI__") != dui.undefined.cend();
    if (!isGnu(dui) && !strictAnsiDefined && !strictAnsiUndefined)
        macros.insert(std::pair<TokenString, Macro>("__STRICT_ANSI__", Macro("__STRICT_ANSI__", "1", dummy)));

    macros.insert(std::make_pair("__FILE__", Macro("__FILE__", "__FILE__", dummy)));
    macros.insert(std::make_pair("__LINE__", Macro("__LINE__", "__LINE__", dummy)));
    macros.insert(std::make_pair("__COUNTER__", Macro("__COUNTER__", "__COUNTER__", dummy)));
    struct tm ltime {};
    getLocaltime(ltime);
    macros.insert(std::make_pair("__DATE__", Macro("__DATE__", getDateDefine(&ltime), dummy)));
    macros.insert(std::make_pair("__TIME__", Macro("__TIME__", getTimeDefine(&ltime), dummy)));

    if (!dui.std.empty()) {
        const cstd_t c_std = simplecpp::getCStd(dui.std);
        if (c_std != CUnknown) {
            const std::string std_def = simplecpp::getCStdString(c_std);
            if (!std_def.empty())
                macros.insert(std::make_pair("__STDC_VERSION__", Macro("__STDC_VERSION__", std_def, dummy)));
        } else {
            const cppstd_t cpp_std = simplecpp::getCppStd(dui.std);
            if (cpp_std == CPPUnknown) {
                if (outputList) {
                    simplecpp::Output err{
                        Output::DUI_ERROR,
                        {},
                        "unknown standard specified: '" + dui.std + "'"
                    };
                    outputList->emplace_back(std::move(err));
                }
                output.clear();
                return;
            }
            const std::string std_def = simplecpp::getCppStdString(cpp_std);
            if (!std_def.empty())
                macros.insert(std::make_pair("__cplusplus", Macro("__cplusplus", std_def, dummy)));
        }
    }

    // True => code in current #if block should be kept
    // ElseIsTrue => code in current #if block should be dropped. the code in the #else should be kept.
    // AlwaysFalse => drop all code in #if and #else
    enum IfState : std::uint8_t { True, ElseIsTrue, AlwaysFalse };
    std::stack<int> ifstates;
    std::stack<const Token *> iftokens;
    ifstates.push(True);

    std::stack<const Token *> includetokenstack;

    std::set<std::string> pragmaOnce;

    includetokenstack.push(rawtokens.cfront());
    for (auto it = dui.includes.cbegin(); it != dui.includes.cend(); ++it) {
        const FileData *const filedata = cache.get("", *it, dui, false, files, outputList).first;
        if (filedata == nullptr) {
            if (outputList) {
                simplecpp::Output err{
                    simplecpp::Output::EXPLICIT_INCLUDE_NOT_FOUND,
                    {},
                    "Can not open include file '" + *it + "' that is explicitly included."
                };
                outputList->emplace_back(std::move(err));
            }
        }
        else if (filedata->tokens.cfront() != nullptr) {
            includetokenstack.push(filedata->tokens.cfront());
        }
    }

    std::map<std::string, std::list<Location>> maybeUsedMacros;

    for (const Token *rawtok = nullptr; rawtok || !includetokenstack.empty();) {
        if (rawtok == nullptr) {
            rawtok = includetokenstack.top();
            includetokenstack.pop();
            continue;
        }

        if (rawtok->op == '#' && !sameline(rawtok->previousSkipComments(), rawtok)) {
            if (!sameline(rawtok, rawtok->next)) {
                rawtok = rawtok->next;
                continue;
            }
            rawtok = rawtok->next;
            if (!rawtok->name) {
                rawtok = gotoNextLine(rawtok);
                continue;
            }

            if (ifstates.size() <= 1U && (rawtok->str() == ELIF || rawtok->str() == ELIFDEF ||
                                          rawtok->str() == ELIFNDEF || rawtok->str() == ELSE ||
                                          rawtok->str() == ENDIF)) {
                if (outputList) {
                    simplecpp::Output err{
                        Output::SYNTAX_ERROR,
                        rawtok->location,
                        "#" + rawtok->str() + " without #if"
                    };
                    outputList->emplace_back(std::move(err));
                }
                output.clear();
                return;
            }

            if (ifstates.top() == True && (rawtok->str() == ERROR || rawtok->str() == WARNING)) {
                if (outputList) {
                    std::string msg;
                    for (const Token *tok = rawtok->next; tok && sameline(rawtok,tok); tok = tok->next) {
                        if (!msg.empty() && isNameChar(tok->str()[0]))
                            msg += ' ';
                        msg += tok->str();
                    }
                    msg = '#' + rawtok->str() + ' ' + msg;
                    simplecpp::Output err{
                        rawtok->str() == ERROR ? Output::ERROR : Output::WARNING,
                        rawtok->location,
                        std::move(msg)
                    };

                    outputList->emplace_back(std::move(err));
                }
                if (rawtok->str() == ERROR) {
                    output.clear();
                    return;
                }
            }

            if (rawtok->str() == DEFINE) {
                if (ifstates.top() != True)
                    continue;
                try {
                    const Macro &macro = Macro(rawtok->previous, files);
                    if (dui.undefined.find(macro.name()) == dui.undefined.end()) {
                        const MacroMap::iterator it = macros.find(macro.name());
                        if (it == macros.end())
                            macros.insert(std::pair<TokenString, Macro>(macro.name(), macro));
                        else
                            it->second = macro;
                    }
                } catch (const std::runtime_error &err) {
                    if (outputList) {
                        simplecpp::Output out{
                            Output::SYNTAX_ERROR,
                            rawtok->location,
                            std::string("Failed to parse #define, ") + err.what()
                        };
                        outputList->emplace_back(std::move(out));
                    }
                    output.clear();
                    return;
                } catch (const simplecpp::Macro::Error &err) {
                    if (outputList) {
                        simplecpp::Output out{
                            simplecpp::Output::SYNTAX_ERROR,
                            err.location,
                            "Failed to parse #define, " + err.what
                        };
                        outputList->emplace_back(std::move(out));
                    }
                    output.clear();
                    return;
                }
            } else if (ifstates.top() == True && rawtok->str() == INCLUDE) {
                TokenList inc1(files);
                for (const Token *inctok = rawtok->next; sameline(rawtok,inctok); inctok = inctok->next) {
                    if (!inctok->comment)
                        inc1.push_back(new Token(*inctok));
                }
                TokenList inc2(files);
                if (!inc1.empty() && inc1.cfront()->name) {
                    const Token *inctok = inc1.cfront();
                    if (!preprocessToken(inc2, inctok, macros, files, outputList, dui)) {
                        output.clear();
                        return;
                    }
                } else {
                    inc2.takeTokens(inc1);
                }

                if (!inc1.empty() && !inc2.empty() && inc2.cfront()->op == '<' && inc2.cback()->op == '>') {
                    TokenString hdr;
                    // TODO: Sometimes spaces must be added in the string
                    // Somehow preprocessToken etc must be told that the location should be source location not destination location
                    for (const Token *tok = inc2.cfront(); tok; tok = tok->next) {
                        hdr += tok->str();
                    }
                    inc2.clear();
                    inc2.push_back(new Token(hdr, inc1.cfront()->location));
                    inc2.front()->op = '<';
                }

                if (inc2.empty() || inc2.cfront()->str().size() <= 2U) {
                    if (outputList) {
                        simplecpp::Output err{
                            Output::SYNTAX_ERROR,
                            rawtok->location,
                            "No header in #include"
                        };
                        outputList->emplace_back(std::move(err));
                    }
                    output.clear();
                    return;
                }

                const Token * const inctok = inc2.cfront();

                const bool systemheader = (inctok->str()[0] == '<');
                const std::string header(inctok->str().substr(1U, inctok->str().size() - 2U));
                const FileData *const filedata = cache.get(rawtokens.file(rawtok->location), header, dui, systemheader, files, outputList).first;
                if (filedata == nullptr) {
                    if (outputList) {
                        simplecpp::Output out{
                            simplecpp::Output::MISSING_HEADER,
                            rawtok->location,
                            "Header not found: " + inctok->str()
                        };
                        outputList->emplace_back(std::move(out));
                    }
                } else if (includetokenstack.size() >= 400) {
                    if (outputList) {
                        simplecpp::Output out{
                            simplecpp::Output::INCLUDE_NESTED_TOO_DEEPLY,
                            rawtok->location,
                            "#include nested too deeply"
                        };
                        outputList->emplace_back(std::move(out));
                    }
                } else if (pragmaOnce.find(filedata->filename) == pragmaOnce.end()) {
                    includetokenstack.push(gotoNextLine(rawtok));
                    rawtok = filedata->tokens.cfront();
                    continue;
                }
            } else if (rawtok->str() == IF || rawtok->str() == IFDEF || rawtok->str() == IFNDEF || rawtok->str() == ELIF ||
                       rawtok->str() == ELIFDEF || rawtok->str() == ELIFNDEF) {
                if (!sameline(rawtok,rawtok->next)) {
                    if (outputList) {
                        simplecpp::Output out{
                            simplecpp::Output::SYNTAX_ERROR,
                            rawtok->location,
                            "Syntax error in #" + rawtok->str()
                        };
                        outputList->emplace_back(std::move(out));
                    }
                    output.clear();
                    return;
                }

                const bool isElif = (rawtok->str() == ELIF || rawtok->str() == ELIFDEF || rawtok->str() == ELIFNDEF);

                bool conditionIsTrue;
                // An #if is only evaluated in an active group. An #elif is only evaluated
                // if no previous group was taken; otherwise it is skipped (C 6.10.1p6).
                if (isElif ? (ifstates.top() != ElseIsTrue) : (ifstates.top() != True)) {
                    conditionIsTrue = false;
                }
                else if (rawtok->str() == IFDEF || rawtok->str() == ELIFDEF) {
                    const std::string &name = rawtok->next->str();
                    conditionIsTrue = (macros.find(name) != macros.end() || (hasInclude && name == HAS_INCLUDE));
                    maybeUsedMacros[name].emplace_back(rawtok->next->location);
                    if (ifCond) {
                        const std::string E = conditionIsTrue ? "1" : "0";
                        const long long result = conditionIsTrue ? 1 : 0;
                        ifCond->emplace_back(rawtok->location, E, result);
                    }
                } else if (rawtok->str() == IFNDEF || rawtok->str() == ELIFNDEF) {
                    const std::string &name = rawtok->next->str();
                    conditionIsTrue = (macros.find(name) == macros.end() && !(hasInclude && name == HAS_INCLUDE));
                    maybeUsedMacros[name].emplace_back(rawtok->next->location);
                    if (ifCond) {
                        const std::string E = conditionIsTrue ? "1" : "0";
                        const long long result = conditionIsTrue ? 1 : 0;
                        ifCond->emplace_back(rawtok->location, E, result);
                    }
                } else { /*if (rawtok->str() == IF || rawtok->str() == ELIF)*/
                    TokenList expr(files);
                    for (const Token *tok = rawtok->next; tok && tok->location.sameline(rawtok->location); tok = tok->next) {
                        if (!tok->name) {
                            expr.push_back(new Token(*tok));
                            continue;
                        }

                        if (tok->str() == DEFINED) {
                            tok = tok->next;
                            const bool par = (tok && tok->op == '(');
                            if (par)
                                tok = tok->next;
                            if (tok) {
                                if (macros.find(tok->str()) != macros.end()) {
                                    maybeUsedMacros[tok->str()].emplace_back(tok->location);
                                    expr.push_back(new Token("1", tok->location));
                                } else if (hasInclude && tok->str() == HAS_INCLUDE) {
                                    expr.push_back(new Token("1", tok->location));
                                } else {
                                    expr.push_back(new Token("0", tok->location));
                                }
                            }
                            if (par)
                                tok = tok ? tok->next : nullptr;
                            if (!tok || !sameline(rawtok,tok) || (par && tok->op != ')')) {
                                if (outputList) {
                                    Output out{
                                        Output::SYNTAX_ERROR,
                                        rawtok->location,
                                        "failed to evaluate " + std::string(rawtok->str() == IF ? "#if" : "#elif") + " condition"
                                    };
                                    outputList->emplace_back(std::move(out));
                                }
                                output.clear();
                                return;
                            }
                            continue;
                        }

                        if (hasInclude && tok->str() == HAS_INCLUDE) {
                            tok = tok->next;
                            const bool par = (tok && tok->op == '(');
                            if (par)
                                tok = tok->next;
                            bool closingAngularBracket = false;
                            if (tok) {
                                const std::string &sourcefile = rawtokens.file(rawtok->location);
                                const bool systemheader = tok->op == '<';
                                std::string header;

                                if (systemheader) {
                                    while ((tok = tok->next) && tok->op != '>')
                                        header += tok->str();
                                    if (tok && tok->op == '>')
                                        closingAngularBracket = true;
                                } else {
                                    header = tok->str().substr(1U, tok->str().size() - 2U);
                                    closingAngularBracket = true;
                                }
                                if (tok) {
                                    std::ifstream f;
                                    const std::string header2 = openHeader(f,dui,sourcefile,header,systemheader);
                                    expr.push_back(new Token(header2.empty() ? "0" : "1", tok->location));
                                }
                            }
                            if (par)
                                tok = tok ? tok->next : nullptr;
                            if (!tok || !sameline(rawtok,tok) || (par && tok->op != ')') || (!closingAngularBracket)) {
                                if (outputList) {
                                    Output out{
                                        Output::SYNTAX_ERROR,
                                        rawtok->location,
                                        "failed to evaluate " + std::string(rawtok->str() == IF ? "#if" : "#elif") + " condition"
                                    };
                                    outputList->emplace_back(std::move(out));
                                }
                                output.clear();
                                return;
                            }
                            continue;
                        }

                        // usage of defined macros is recorded when they are expanded
                        if (macros.find(tok->str()) == macros.end())
                            maybeUsedMacros[tok->str()].emplace_back(tok->location);

                        const Token *tmp = tok;
                        if (!preprocessToken(expr, tmp, macros, files, outputList, dui)) {
                            output.clear();
                            return;
                        }
                        if (!tmp)
                            break;
                        tok = tmp->previous;
                    }
                    try {
                        if (ifCond) {
                            std::string E;
                            for (const simplecpp::Token *tok = expr.cfront(); tok; tok = tok->next)
                                E += (E.empty() ? "" : " ") + tok->str();
                            const long long result = evaluate(expr, dui, sizeOfType);
                            conditionIsTrue = (result != 0);
                            ifCond->emplace_back(rawtok->location, E, result);
                        } else {
                            const long long result = evaluate(expr, dui, sizeOfType);
                            conditionIsTrue = (result != 0);
                        }
                    } catch (const std::runtime_error &e) {
                        if (outputList) {
                            std::string msg = "failed to evaluate " + std::string(rawtok->str() == IF ? "#if" : "#elif") + " condition";
                            if (e.what() && *e.what())
                                msg += std::string(", ") + e.what();
                            Output out{
                                Output::SYNTAX_ERROR,
                                rawtok->location,
                                std::move(msg)
                            };
                            outputList->emplace_back(std::move(out));
                        }
                        output.clear();
                        return;
                    }
                }

                if (rawtok->str() != ELIF && rawtok->str() != ELIFDEF && rawtok->str() != ELIFNDEF) {
                    // push a new ifstate..
                    if (ifstates.top() != True)
                        ifstates.push(AlwaysFalse);
                    else
                        ifstates.push(conditionIsTrue ? True : ElseIsTrue);
                    iftokens.push(rawtok);
                } else {
                    if (ifstates.top() == True)
                        ifstates.top() = AlwaysFalse;
                    else if (ifstates.top() == ElseIsTrue && conditionIsTrue)
                        ifstates.top() = True;
                    iftokens.top()->nextcond = rawtok;
                    iftokens.top() = rawtok;
                }
            } else if (rawtok->str() == ELSE) {
                ifstates.top() = (ifstates.top() == ElseIsTrue) ? True : AlwaysFalse;
                iftokens.top()->nextcond = rawtok;
                iftokens.top() = rawtok;
            } else if (rawtok->str() == ENDIF) {
                ifstates.pop();
                iftokens.top()->nextcond = rawtok;
                iftokens.pop();
            } else if (rawtok->str() == UNDEF) {
                if (ifstates.top() == True) {
                    const Token *tok = rawtok->next;
                    while (sameline(rawtok,tok) && tok->comment)
                        tok = tok->next;
                    if (sameline(rawtok, tok))
                        macros.erase(tok->str());
                }
            } else if (ifstates.top() == True && rawtok->str() == PRAGMA && rawtok->next && rawtok->next->str() == ONCE && sameline(rawtok,rawtok->next)) {
                pragmaOnce.insert(rawtokens.file(rawtok->location));
            }
            if (ifstates.top() != True && rawtok->nextcond)
                rawtok = rawtok->nextcond->previous;
            else
                rawtok = gotoNextLine(rawtok);
            continue;
        }

        if (ifstates.top() != True) {
            // drop code
            rawtok = gotoNextLine(rawtok);
            continue;
        }

        bool hash=false, hashhash=false;
        if (rawtok->op == '#' && sameline(rawtok,rawtok->next)) {
            if (rawtok->next->op != '#') {
                hash = true;
                rawtok = rawtok->next; // skip '#'
            } else if (sameline(rawtok,rawtok->next->next)) {
                hashhash = true;
                rawtok = rawtok->next->next; // skip '#' '#'
            }
        }

        const Location loc(rawtok->location);
        TokenList tokens(files);

        if (!preprocessToken(tokens, rawtok, macros, files, outputList, dui)) {
            output.clear();
            return;
        }

        if (hash || hashhash) {
            std::string s;
            for (const Token *hashtok = tokens.cfront(); hashtok; hashtok = hashtok->next)
                s += hashtok->str();
            if (hash)
                output.push_back(new Token('\"' + s + '\"', loc));
            else if (output.back())
                output.back()->setstr(output.cback()->str() + s);
            else
                output.push_back(new Token(s, loc));
        } else {
            output.takeTokens(tokens);
        }
    }

    if (macroUsage) {
        for (simplecpp::MacroMap::const_iterator macroIt = macros.begin(); macroIt != macros.end(); ++macroIt) {
            const Macro &macro = macroIt->second;
            std::list<Macro::Usage> usage = macro.usage();
            const std::list<Location>& temp = maybeUsedMacros[macro.name()];
            usage.insert(usage.end(), temp.begin(), temp.end());
            for (std::list<Macro::Usage>::iterator usageIt = usage.begin(); usageIt != usage.end(); ++usageIt) {
                MacroUsage mu(macro.valueDefinedInCode());
                mu.macroName = macro.name();
                mu.macroLocation = macro.defineLocation();
                mu.useLocation = usageIt->location;
                mu.arg = std::move(usageIt->arg);
                macroUsage->emplace_back(std::move(mu));
            }
        }
    }
}

void simplecpp::cleanup(FileDataCache &cache)
{
    cache.clear();
}

simplecpp::cstd_t simplecpp::getCStd(const std::string &std)
{
    if (std == "c90" || std == "c89" || std == "iso9899:1990" || std == "iso9899:199409" || std == "gnu90" || std == "gnu89")
        return C89;
    if (std == "c99" || std == "c9x" || std == "iso9899:1999" || std == "iso9899:199x" || std == "gnu99" || std == "gnu9x")
        return C99;
    if (std == "c11" || std == "c1x" || std == "iso9899:2011" || std == "gnu11" || std == "gnu1x")
        return C11;
    if (std == "c17" || std == "c18" || std == "iso9899:2017" || std == "iso9899:2018" || std == "gnu17" || std == "gnu18")
        return C17;
    if (std == "c23" || std == "gnu23" || std == "c2x" || std == "gnu2x")
        return C23;
    if (std == "c2y" || std == "gnu2y")
        return C2Y;
    return CUnknown;
}

const char *simplecpp::getCStdName(cstd_t std)
{
    switch (std) {
    case CUnknown: return "C";
    case C89: return "C89";
    case C99: return "C99";
    case C11: return "C11";
    case C17: return "C17";
    case C23: return "C23";
    case C2Y: return "C2Y";
    }
    return "";
}

std::string simplecpp::getCStdString(cstd_t std)
{
    switch (std) {
    case C89:
        // __STDC_VERSION__ is not set for C90 although the macro was added in the 1994 amendments
        return "";
    case C99:
        return "199901L";
    case C11:
        return "201112L";
    case C17:
        return "201710L";
    case C23:
        // supported by GCC 9+ and Clang 9+
        // Clang 9, 10, 11, 12, 13 return "201710L"
        // Clang 14, 15, 16, 17 return "202000L"
        // Clang 9, 10, 11, 12, 13, 14, 15, 16, 17 do not support "c23" and "gnu23"
        return "202311L";
    case C2Y:
        // supported by GCC 15+ and Clang 19+
        // Clang 19, 20, 21, 22 return "202400L"
        return "202500L";
    case CUnknown:
        return "";
    }
    return "";
}

std::string simplecpp::getCStdString(const std::string &std)
{
    return getCStdString(getCStd(std));
}

simplecpp::cppstd_t simplecpp::getCppStd(const std::string &std)
{
    if (std == "c++98" || std == "c++03" || std == "gnu++98" || std == "gnu++03")
        return CPP03;
    if (std == "c++11" || std == "gnu++11" || std == "c++0x" || std == "gnu++0x")
        return CPP11;
    if (std == "c++14" || std == "c++1y" || std == "gnu++14" || std == "gnu++1y")
        return CPP14;
    if (std == "c++17" || std == "c++1z" || std == "gnu++17" || std == "gnu++1z")
        return CPP17;
    if (std == "c++20" || std == "c++2a" || std == "gnu++20" || std == "gnu++2a")
        return CPP20;
    if (std == "c++23" || std == "c++2b" || std == "gnu++23" || std == "gnu++2b")
        return CPP23;
    if (std == "c++26" || std == "c++2c" || std == "gnu++26" || std == "gnu++2c")
        return CPP26;
    return CPPUnknown;
}

const char *simplecpp::getCppStdName(cppstd_t std)
{
    switch (std) {
    case CPPUnknown: return "C++";
    case CPP03: return "C++03";
    case CPP11: return "C++11";
    case CPP14: return "C++14";
    case CPP17: return "C++17";
    case CPP20: return "C++20";
    case CPP23: return "C++23";
    case CPP26: return "C++26";
    }
    return "";
}

std::string simplecpp::getCppStdString(cppstd_t std)
{
    switch (std) {
    case CPP03:
        return "199711L";
    case CPP11:
        return "201103L";
    case CPP14:
        return "201402L";
    case CPP17:
        return "201703L";
    case CPP20:
        // GCC 10 returns "201703L" - correct in 11+
        return "202002L";
    case CPP23:
        // supported by GCC 11+ and Clang 12+
        // GCC 11, 12, 13 return "202100L"
        // Clang 12, 13, 14, 15, 16 do not support "c++23" and "gnu++23" and return "202101L"
        // Clang 17, 18 return "202302L"
        return "202302L";
    case CPP26:
        // supported by GCC 14+ and Clang 17+
        return "202400L";
    case CPPUnknown:
        return "";
    }
    return "";
}

std::string simplecpp::getCppStdString(const std::string &std)
{
    return getCppStdString(getCppStd(std));
}
