/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#ifndef helpersH
#define helpersH

#include "library.h"
#include "path.h"
#include "settings.h"
#include "standards.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstddef>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

class Token;
class ErrorLogger;
namespace tinyxml2 {
    class XMLDocument;
}

// TODO: make Tokenizer private
class SimpleTokenizer : public Tokenizer {
public:
    explicit SimpleTokenizer(ErrorLogger& errorlogger, bool cpp = true)
        : Tokenizer{TokenList{s_settings, cpp ? Standards::Language::CPP : Standards::Language::C}, errorlogger}
    {}

    SimpleTokenizer(const Settings& settings, ErrorLogger& errorlogger, bool cpp = true)
        : Tokenizer{TokenList{settings, cpp ? Standards::Language::CPP : Standards::Language::C}, errorlogger}
    {}

    SimpleTokenizer(const Settings& settings, ErrorLogger& errorlogger, const std::string& filename)
        : Tokenizer{TokenList{settings, Path::identify(filename, false)}, errorlogger}
    {
        list.appendFileIfNew(filename);
    }

    template<size_t size>
    bool tokenize(const char (&code)[size])
    {
        return tokenize(code, size-1);
    }

    bool tokenize(const std::string& code)
    {
        return tokenize(code.data(), code.size());
    }

    bool tokenize(const char* code, std::size_t size)
    {
        return tokenize(code, size, std::string(list.isCPP() ? "test.cpp" : "test.c"));
    }

private:
    /**
     * Tokenize code
     * @param code The code
     * @param filename Indicates if the code is C++
     * @return false if source code contains syntax errors
     */
    template<size_t size>
    bool tokenize(const char (&code)[size],
                  const std::string& filename)
    {
        return tokenize(code, size-1, filename);
    }

    bool tokenize(const char* code,
                  std::size_t size,
                  const std::string& filename)
    {
        if (list.front())
            throw std::runtime_error("token list is not empty");
        list.appendFileIfNew(filename);
        if (!list.createTokensFromBuffer(code, size))
            return false;

        return simplifyTokens1("");
    }

    // TODO: find a better solution
    static const Settings s_settings;
};

class SimpleTokenList
{
public:
    template<size_t size>
    explicit SimpleTokenList(const char (&code)[size], Standards::Language lang = Standards::Language::CPP)
        : list{settings, lang}
    {
        if (!list.createTokensFromString(code))
            throw std::runtime_error("creating tokens failed");
    }

    template<size_t size>
    explicit SimpleTokenList(const char (&code)[size], const std::string& file0, Standards::Language lang = Standards::Language::CPP)
        : list{settings, lang}
    {
        list.appendFileIfNew(file0);
        if (!list.createTokensFromString(code))
            throw std::runtime_error("creating tokens failed");
    }

    Token* front() {
        return list.front();
    }

    const Token* front() const {
        return list.front();
    }

    const TokenList& get() const {
        return list;
    }

private:
    const Settings settings;
    TokenList list;
};


class ScopedFile {
public:
    ScopedFile(std::string name, const std::string &content, std::string path = "");
    ~ScopedFile();

    const std::string& path() const
    {
        return mFullPath;
    }

    const std::string& name() const {
        return mName;
    }

    ScopedFile(const ScopedFile&) = delete;
    ScopedFile(ScopedFile&&) = delete;
    ScopedFile& operator=(const ScopedFile&) = delete;
    ScopedFile& operator=(ScopedFile&&) = delete;

private:
    const std::string mName;
    const std::string mPath;
    const std::string mFullPath;
};

namespace cppcheck {
    template<typename T>
    std::size_t count_all_of(const std::string& str, T sub) {
        std::size_t n = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(sub, pos)) != std::string::npos) {
            ++pos;
            ++n;
        }
        return n;
    }
}

/* designated initialization helper
    Usage:
    struct S
    {
        int i;
    };

    const auto s = dinit(S,
        $.i = 1
    );
 */
#define dinit(T, ...) \
    ([&] { T ${}; __VA_ARGS__; return $; }())

// Default construct object to avoid bug in clang
// error: default member initializer for 'y' needed within definition of enclosing class 'X' outside of member functions
// see https://stackoverflow.com/questions/53408962
struct make_default_obj
{
    template<class T>
    operator T() const // NOLINT
    {
        return T{};
    }
};

inline std::string filter_valueflow(const std::string& s) {
    bool filtered = false;
    std::istringstream istr(s);
    std::string ostr;
    std::string errline;
    while (std::getline(istr, errline)) {
        if (errline.find("valueflow.cpp") != std::string::npos)
        {
            filtered = true;
            continue;
        }
        ostr += errline;
        ostr += '\n'; // TODO: last line might not contain a newline
    }
    if (!filtered)
        throw std::runtime_error("no valueflow.cpp messages were filtered");
    return ostr;
}

struct LibraryHelper
{
    static bool loadxmldata(Library &lib, const char xmldata[], std::size_t len);
    static bool loadxmldata(Library &lib, Library::Error& liberr, const char xmldata[], std::size_t len);
    static Library::Error loadxmldoc(Library &lib, const tinyxml2::XMLDocument& doc);
};

class SimpleTokenizer2 : public Tokenizer {
    friend class TestSimplifyTypedef; // TODO: get rid of this
public:
    template<size_t size>
    SimpleTokenizer2(const Settings &settings, ErrorLogger &errorlogger, const char (&code)[size], const std::string& file0)
        : Tokenizer{TokenList{settings, Path::identify(file0, false)}, errorlogger}
    {
        preprocess(code, size-1, mFiles, file0, *this, errorlogger);
    }

private:
    static void preprocess(const char* code, std::size_t size, std::vector<std::string> &files, const std::string& file0, Tokenizer& tokenizer, ErrorLogger& errorlogger);

    std::vector<std::string> mFiles;
};

struct TokenListHelper
{
    template<size_t size>
    static bool createTokensFromString(TokenList& tokenlist, const char (&code)[size], const std::string& file)
    {
        if (tokenlist.front())
            throw std::runtime_error("token list is not empty");
        tokenlist.appendFileIfNew(file);
        return tokenlist.createTokensFromString(code);
    }
};

#endif // helpersH
