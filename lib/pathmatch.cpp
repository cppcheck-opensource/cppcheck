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

#include "pathmatch.h"

#include "path.h"

#include <algorithm>
#include <stack>
#include <string>
#include <utility>
#include <vector>


PathMatch::PathMatch(std::vector<std::string> patterns, std::string basepath, Syntax syntax) :
    mPatterns(std::move(patterns)), mBasepath(std::move(basepath)), mSyntax(syntax)
{}

bool PathMatch::match(const std::string &path, Filemode mode) const
{
    return std::any_of(mPatterns.cbegin(), mPatterns.cend(), [&] (const std::string &pattern) {
        return match(pattern, path, mBasepath, mode, mSyntax);
    });
}

bool PathMatch::match(const std::string &pattern, const std::string &path, const std::string &basepath, Filemode mode, Syntax syntax)
{
    /* Fast paths for common patterns */
    if (pattern.empty())
        return false;

    if (pattern == "*" || pattern == "**")
        return true;

    /* If the pattern ends with a path separator it matches only directories. If the path names a regular file then
     * the last path component can't match. */
    bool dir_mismatch = PathIterator::issep(pattern.back(), syntax) && mode != Filemode::directory;

    if (!dir_mismatch && pattern == path)
        return true;

    /* A "real" pattern is absolute or relative to the base path. A pattern that isn't real can match at any
     * path component boundary. */
    bool real = Path::isAbsolute(pattern) || isRelativePattern(pattern);

    /* Pattern iterator */
    PathIterator s = PathIterator::fromPattern(pattern, basepath, syntax);
    /* Path iterator */
    PathIterator t = PathIterator::fromPath(path, basepath, syntax);

    if (dir_mismatch) {
        /* Final compponent can't match, so skip it. */
        while (*t != '\0' && *t != '/')
            ++t;
    }

    /* Pattern restart position */
    PathIterator p = s;
    /* Path restart position */
    PathIterator q = t;

    /* Backtrack stack */
    std::stack<std::pair<PathIterator::Pos, PathIterator::Pos>> b;

    for (;;) {
        switch (*s) {
        /* Star or star-star, matches any number of characters */
        case '*': {
            bool slash = false;
            ++s;
            if (*s == '*') {
                /* Star-star matches slashes as well */
                slash = true;
                ++s;
            }
            while (*t != '\0' && (slash || *t != '/')) {
                if (*s == *t) {
                    /* Could stop here, but do greedy match and add
                     * backtrack instead */
                    b.emplace(s.getpos(), t.getpos());
                }
                ++t;
            }
            continue;
        }
        /* Single character wildcard */
        case '?': {
            if (*t != '\0' && *t != '/') {
                ++s;
                ++t;
                continue;
            }
            break;
        }
        /* Start of pattern; matches start of path, or a path separator if the
         * pattern is not "real" (an absolute or relative path). */
        case '\0': {
            if (*t == '\0' || (*t == '/' && !real))
                return true;
            break;
        }
        /* Literal character */
        default: {
            if (*s == *t) {
                ++s;
                ++t;
                continue;
            }
            break;
        }
        }

        /* No match, try to backtrack */
        if (!b.empty()) {
            const auto bp = b.top();
            b.pop();
            s.setpos(bp.first);
            t.setpos(bp.second);
            continue;
        }

        /* Couldn't backtrack, try matching from the next path separator */
        while (*q != '\0' && *q != '/')
            ++q;

        if (*q == '/') {
            ++q;
            s = p;
            t = q;
            continue;
        }

        /* No more path separators to try from */
        return false;
    }
}
