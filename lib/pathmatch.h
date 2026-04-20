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

#ifndef PATHMATCH_H
#define PATHMATCH_H

#include "config.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "path.h"

/// @addtogroup CLI
/// @{

/**
 * Path matching rules:
 * - All patterns are canonicalized (path separators vary by platform):
 *   - '/./' => '/'
 *   - '/dir/../' => '/'
 *   - '//' => '/'
 *   - Trailing slashes are removed (root slash is preserved)
 * - Patterns can contain globs:
 *   - '**' matches any number of characters including path separators.
 *   - '*' matches any number of characters except path separators.
 *   - '?' matches any single character except path separators.
 * - If a pattern looks like an absolute path (e.g. starts with '/', but varies by platform):
 *   - Match all files where the pattern matches the start of the file's canonical absolute path up until a path
 *     separator or the end of the pathname.
 * - If a pattern looks like a relative path, i.e. is '.' or '..', or
 *   starts with '.' or '..' followed by a path separator:
 *   - The pattern is interpreted as a path relative to `basepath` and then converted to an absolute path and
 *     treated as such according to the above procedure. If the pattern is relative to some other directory, it should
 *     be modified to be relative to `basepath` first (this should be done with patterns in project files, for example).
 * - Otherwise:
 *   - Match all files where the pattern matches any part of the file's canonical absolute path up until a
 *     path separator or the end of the pathname, and the matching part directly follows a path separator.
 * - If a pattern ends with a path separator before canonicalization then the final path component of the pattern
 *   only matches the final path component of the file path if the file is a directory (specified by the file mode
 *   parameter).
 *
 * TODO: Handle less common windows syntaxes:
 *   - Drive-specific relative path: C:dir\foo.cpp
 *   - Root-relative path: \dir\foo.cpp
 **/

/**
 * @brief Syntactic path matching for ignoring paths in CLI.
 */
class CPPCHECKLIB PathMatch {
public:

    /**
     * @brief Path syntax.
     *
     * windows: Case insensitive, forward and backward slashes, UNC or drive letter root.
     * unix: Case sensitive, forward slashes, slash root.
     *
     */
    enum class Syntax : std::uint8_t {
        windows,
        unix,
    };

    /**
     * @brief The default syntax for the current platform.
     */
#ifdef _WIN32
    static constexpr Syntax platform_syntax = Syntax::windows;
#else
    static constexpr Syntax platform_syntax = Syntax::unix;
#endif

    /**
     * @brief File mode of a file being matched
     *
     * regular: File is a regular file.
     * directory: File is a directory.
     */
    enum class Filemode : std::uint8_t {
        regular,
        directory,
    };

    /**
     * The constructor.
     *
     * @param patterns List of patterns.
     * @param basepath Path to which patterns and matched paths are relative, when applicable.
     * @param syntax Path syntax.
     */
    explicit PathMatch(std::vector<std::string> patterns = {}, std::string basepath = std::string(), Syntax syntax = platform_syntax);

    /**
     * @brief Match path against list of patterns.
     *
     * @param path Path to match.
     * @param mode The file mode of the file named by the path.
     * @return true if any of the masks match the path, false otherwise.
     */
    bool match(const std::string &path, Filemode mode = Filemode::regular) const;

    /**
     * @brief Match path against a single pattern.
     *
     * @param pattern Pattern to use.
     * @param path Path to match.
     * @param basepath Path to which the pattern and path is relative, when applicable.
     * @param mode The file mode of the file named by the path.
     * @param syntax Path syntax.
     * @return true if the pattern matches the path, false otherwise.
     */
    static bool match(const std::string &pattern, const std::string &path, const std::string &basepath = std::string(), Filemode mode = Filemode::regular, Syntax syntax = platform_syntax);

    /**
     * @brief Check if a pattern is a relative path name.
     *
     * @param pattern Pattern to check.
     * @return true if the pattern has the form of a relative path name pattern.
     */
    static bool isRelativePattern(const std::string &pattern)
    {
        if (pattern.empty() || pattern[0] != '.')
            return false;

        if (pattern.size() < 2 || pattern[1] == '/' || pattern[1] == '\\')
            return true;

        if (pattern[1] != '.')
            return false;

        if (pattern.size() < 3 || pattern[2] == '/' || pattern[2] == '\\')
            return true;

        return false;
    }

    /**
     * @brief Join a pattern with a base path.
     *
     * @param basepath The base path to join the pattern to.
     * @param pattern The pattern to join.
     * @return The pattern appended to the base path with a separator if the pattern is a relative
     * path name, otherwise just returns pattern.
     */
    static std::string joinRelativePattern(const std::string &basepath, const std::string &pattern)
    {
        if (isRelativePattern(pattern))
            return Path::join(basepath, pattern);
        return pattern;
    }

protected:
    class PathIterator;

private:
    /* List of patterns */
    std::vector<std::string> mPatterns;
    /* Base path to with patterns and paths are relative */
    std::string mBasepath;
    /* The syntax to use */
    Syntax mSyntax;
};

/**
 * A more correct and less convenient name for this class would be PathStringsCanonicalReverseIterator.
 *
 * This class takes two path strings and iterates their concatenation in reverse while doing canonicalization,
 * i.e. collapsing double-dots, removing extra slashes, dot-slashes, and trailing slashes, as well as converting
 * native slashes to forward slashes and optionally converting characters to lowercase.
 *
 * Both strings are optional. If both strings are present, then they're concatenated with a slash
 * (subject to canonicalization).
 *
 * Double-dots at the root level are removed. Trailing slashes are removed, the root is preserved.
 *
 * Doing the iteration in reverse allows canonicalization to be performed without lookahead. This is useful
 * for comparing path strings, potentially relative to different base paths, without having to do prior string
 * processing or extra allocations.
 *
 * The length of the output is at most strlen(a) + strlen(b) + 1.
 *
 * Example:
 *   - input:  "/hello/universe/.", "../world//"
 *   - output: "dlrow/olleh/"
 **/
class PathMatch::PathIterator {
public:
    /* Create from a pattern and base path */
    static PathIterator fromPattern(const std::string &pattern, const std::string &basepath, Syntax syntax)
    {
        if (isRelativePattern(pattern))
            return PathIterator(basepath.c_str(), pattern.c_str(), syntax);
        return PathIterator(pattern.c_str(), nullptr, syntax);
    }

    /* Create from path and base path */
    static PathIterator fromPath(const std::string &path, const std::string &basepath, Syntax syntax)
    {
        if (Path::isAbsolute(path))
            return PathIterator(path.c_str(), nullptr, syntax);
        return PathIterator(basepath.c_str(), path.c_str(), syntax);
    }

    /* Constructor */
    explicit PathIterator(const char *path_a = nullptr, const char *path_b = nullptr, Syntax syntax = platform_syntax) :
        mStart{path_a, path_b}, mSyntax(syntax)
    {

        for (int i = 0; i < 2; i++) {
            const char *&p = mEnd[i];
            p = mStart[i];

            if (p == nullptr || *p == '\0')
                continue;

            if (mPos.l == 0) {
                /* Check length of root component */
                if (issep(p[0], syntax)) {
                    mRootLength++;
                    if (syntax == Syntax::windows && issep(p[1], syntax)) {
                        mRootLength++;
                        if (p[2] == '.' || p[2] == '?') {
                            mRootLength++;
                            if (issep(p[3], syntax))
                                mRootLength++;
                        }
                    }
                } else if (syntax == Syntax::windows && isdrive(p[0]) && p[1] == ':') {
                    mRootLength += 2;
                    if (issep(p[2], syntax))
                        mRootLength++;
                }
                p += mRootLength;
                mPos.l = mRootLength;
            } else {
                /* Add path separator */
                mPos.l++;
            }

            while (*p != '\0') {
                p++;
                mPos.l++;
            }

            mPos.p = p - 1;
        }

        if (mPos.l == 0)
            mPos.c = '\0';

        skips(false);
    }

    /* Position struct */
    struct Pos {
        /* String pointer */
        const char *p;
        /* Raw characters left */
        std::size_t l;
        /* Buffered character */
        int c {EOF};
    };

    /* Save the current position */
    const Pos &getpos() const
    {
        return mPos;
    }

    /* Restore a saved position */
    void setpos(const Pos &pos)
    {
        mPos = pos;
    }

    /* Read the current character */
    char operator*() const
    {
        return current();
    }

    /* Go to the next character */
    void operator++()
    {
        advance();
    }

    /* Consume remaining characters into an std::string and reverse, use for testing */
    std::string read()
    {
        std::string str;

        while (current() != '\0') {
            str.insert(0, 1, current());
            advance();
        }

        return str;
    }

    /* Syntax helper, check if a character is a path separator */
    static bool issep(char c, Syntax syntax)
    {
        return c == '/' || (syntax == Syntax::windows && c == '\\');
    }

    /* Syntax helper, check if a character is a drive letter */
    static bool isdrive(char c)
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

private:
    /* Read the current character */
    char current() const
    {
        if (mPos.c != EOF)
            return mPos.c;

        char c = *mPos.p;

        if (mSyntax == Syntax::windows) {
            if (c == '\\')
                return '/';
            return std::tolower(c);
        }

        return c;
    }

    /* Do canonicalization on a path component boundary */
    void skips(bool leadsep)
    {
        while (mPos.l > mRootLength) {
            Pos pos = mPos;

            if (leadsep) {
                if (current() != '/')
                    break;
                nextc();
            }

            char c = current();
            if (c == '.') {
                nextc();
                c = current();
                if (c == '.') {
                    nextc();
                    c = current();
                    if (c == '/') {
                        /* Skip 'dir/../' */
                        nextc();
                        skips(false);
                        while (mPos.l > mRootLength && current() != '/')
                            nextc();
                        continue;
                    }
                } else if (c == '/') {
                    /* Skip '/./' */
                    continue;
                } else if (c == '\0') {
                    /* Skip leading './' */
                    break;
                }
            } else if (c == '/') {
                /* Skip double separator (keep root) */
                nextc();
                leadsep = false;
                continue;
            }

            mPos = pos;
            break;
        }
    }

    /* Go to the next character, doing skips on path separators */
    void advance()
    {
        nextc();

        if (current() == '/')
            skips(true);
    }

    /* Go to the next character */
    void nextc()
    {
        if (mPos.l == 0)
            return;

        mPos.l--;

        if (mPos.l == 0)
            mPos.c = '\0';
        else if (mPos.c != EOF) {
            mPos.c = EOF;
        } else {
            if (mPos.p == mStart[1]) {
                mPos.p = mEnd[0];
                mPos.c = '/';
            }
            mPos.p--;
        }
    }

    /* String start pointers */
    const char *mStart[2] {};
    /* String end pointers */
    const char *mEnd[2] {};
    /* Current position */
    Pos mPos {};
    /* Length of the root component */
    std::size_t mRootLength {};
    /* Syntax */
    Syntax mSyntax;
};

/// @}

#endif // PATHMATCH_H
