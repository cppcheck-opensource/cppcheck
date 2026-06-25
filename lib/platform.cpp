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

#include "platform.h"

#include "path.h"

#include <cstring>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

#include "xml.h"

Platform::Platform()
{
    set(Type::Native);
}


bool Platform::set(Type t)
{
    switch (t) {
    case Type::Unspecified: // unknown type sizes (sizes etc are set but are not known)
    case Type::Native: // same as system this code was compile on
        type = t;
        windows = false;
        sizeof_bool = sizeof(bool);
        sizeof_short = sizeof(short);
        sizeof_int = sizeof(int);
        sizeof_long = sizeof(long);
        sizeof_long_long = sizeof(long long);
        sizeof_float = sizeof(float);
        sizeof_double = sizeof(double);
        sizeof_long_double = sizeof(long double);
        sizeof_wchar_t = sizeof(wchar_t);
        sizeof_size_t = sizeof(std::size_t);
        sizeof_pointer = sizeof(void *);
        if (type == Type::Unspecified) {
            defaultSign = '\0';
        } else {
            defaultSign = std::numeric_limits<char>::is_signed ? 's' : 'u';
        }
        char_bit = 8;
        calculateBitMembers();
        return true;
    case Type::Win32W:
    case Type::Win32A:
        type = t;
        windows = true;
        sizeof_bool = 1; // 4 in Visual C++ 4.2
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_wchar_t = 2;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        defaultSign = 's';
        char_bit = 8;
        calculateBitMembers();
        return true;
    case Type::Win64:
        type = t;
        windows = true;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 8;
        sizeof_wchar_t = 2;
        sizeof_size_t = 8;
        sizeof_pointer = 8;
        defaultSign = 's';
        char_bit = 8;
        calculateBitMembers();
        return true;
    case Type::Unix32:
        type = t;
        windows = false;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 4;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 12;
        sizeof_wchar_t = 4;
        sizeof_size_t = 4;
        sizeof_pointer = 4;
        defaultSign = 's';
        char_bit = 8;
        calculateBitMembers();
        return true;
    case Type::Unix64:
        type = t;
        windows = false;
        sizeof_bool = 1;
        sizeof_short = 2;
        sizeof_int = 4;
        sizeof_long = 8;
        sizeof_long_long = 8;
        sizeof_float = 4;
        sizeof_double = 8;
        sizeof_long_double = 16;
        sizeof_wchar_t = 4;
        sizeof_size_t = 8;
        sizeof_pointer = 8;
        defaultSign = 's';
        char_bit = 8;
        calculateBitMembers();
        return true;
    case Type::File:
        // sizes are not set.
        return false;
    }
    // unsupported platform
    return false;
}

bool Platform::set(const std::string& platformstr, std::string& errstr, const std::vector<std::string>& paths, bool debug)
{
    // TODO: needs to be invalidated in case it was already set
    if (platformstr == "win32A")
        set(Type::Win32A);
    else if (platformstr == "win32W")
        set(Type::Win32W);
    else if (platformstr == "win64")
        set(Type::Win64);
    else if (platformstr == "unix32")
        set(Type::Unix32);
    else if (platformstr == "unix64")
        set(Type::Unix64);
    else if (platformstr == "native")
        set(Type::Native);
    else if (platformstr == "unspecified")
        set(Type::Unspecified);
    else if (paths.empty()) {
        errstr = "unrecognized platform: '" + platformstr + "' (no lookup).";
        return false;
    }
    else if (!loadFromFile(paths, platformstr, debug)) {
        errstr = "unrecognized platform: '" + platformstr + "'.";
        return false;
    }

    return true;
}

bool Platform::loadFromFile(const std::vector<std::string>& paths, const std::string &filename, bool debug)
{
    if (debug)
        std::cout << "looking for platform '" + filename + "'" << std::endl;

    const bool is_abs_path = Path::isAbsolute(filename);
    const bool is_rel_path = Path::isRelative(filename);

    std::string fullfilename(filename);
    // TODO: what if extension is not .xml?
    // only append extension when we provide the library name is not a path - TODO: handle relative paths?
    if (!is_abs_path && !is_rel_path && Path::getFilenameExtension(fullfilename).empty())
        fullfilename += ".xml";

    // TODO: use native separators
    std::vector<std::string> filenames;
    if (is_abs_path)
    {
        filenames.push_back(std::move(fullfilename));
    }
    else {
        // TODO: drop duplicated paths
        for (const std::string& path : paths)
        {
            if (path.empty())
                continue; // TODO: error out instead?

            std::string ppath = Path::fromNativeSeparators(path);
            if (ppath.back() != '/')
                ppath += '/';
            // TODO: look in platforms first?
            filenames.push_back(ppath + fullfilename);
            filenames.push_back(ppath + "platforms/" + fullfilename);
        }
#ifdef FILESDIR
        std::string filesdir = FILESDIR;
        if (!filesdir.empty()) {
            if (filesdir.back() != '/')
                filesdir += '/';
            // TODO: look in filesdir?
            filenames.push_back(filesdir + "platforms/" + fullfilename);
        }
#endif
    }

    // open file..
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = tinyxml2::XML_SUCCESS;
    for (const std::string & f : filenames) {
        if (debug)
            std::cout << "try to load platform file '" << f << "' ... ";
        err = doc.LoadFile(f.c_str());
        if (err == tinyxml2::XML_SUCCESS) {
            if (debug)
                std::cout << "Success" << std::endl;
            break;
        }
        if (debug)
            std::cout << doc.ErrorStr() << std::endl;
        if (err != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
            break;
    }
    if (err != tinyxml2::XML_SUCCESS)
        return false;

    return loadFromXmlDocument(&doc);
}

static const char* xmlText(const tinyxml2::XMLElement* node, bool& error)
{
    const char* const str = node->GetText();
    if (!str)
        error = true;
    return str;
}

static unsigned int xmlTextAsUInt(const tinyxml2::XMLElement* node, bool& error)
{
    unsigned int retval = 0;
    if (node->QueryUnsignedText(&retval) != tinyxml2::XML_SUCCESS)
        error = true;
    return retval;
}

static unsigned int xmlTextAsBool(const tinyxml2::XMLElement* node, bool& error)
{
    bool retval = false;
    if (node->QueryBoolText(&retval) != tinyxml2::XML_SUCCESS)
        error = true;
    return retval;
}

bool Platform::loadFromXmlDocument(const tinyxml2::XMLDocument *doc)
{
    const tinyxml2::XMLElement * const rootnode = doc->FirstChildElement();

    if (!rootnode || std::strcmp(rootnode->Name(), "platform") != 0)
        return false;

    bool error = false;
    for (const tinyxml2::XMLElement *node = rootnode->FirstChildElement(); node; node = node->NextSiblingElement()) {
        const char* name = node->Name();
        if (std::strcmp(name, "default-sign") == 0) {
            const char * const str = xmlText(node, error);
            if (!error)
                defaultSign = *str;
        } else if (std::strcmp(name, "char_bit") == 0)
            char_bit = xmlTextAsUInt(node, error);
        else if (std::strcmp(name, "sizeof") == 0) {
            for (const tinyxml2::XMLElement *sz = node->FirstChildElement(); sz; sz = sz->NextSiblingElement()) {
                const char* szname = sz->Name();
                if (std::strcmp(szname, "short") == 0)
                    sizeof_short = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "bool") == 0)
                    sizeof_bool = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "int") == 0)
                    sizeof_int = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "long") == 0)
                    sizeof_long = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "long-long") == 0)
                    sizeof_long_long = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "float") == 0)
                    sizeof_float = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "double") == 0)
                    sizeof_double = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "long-double") == 0)
                    sizeof_long_double = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "pointer") == 0)
                    sizeof_pointer = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "size_t") == 0)
                    sizeof_size_t = xmlTextAsUInt(sz, error);
                else if (std::strcmp(szname, "wchar_t") == 0)
                    sizeof_wchar_t = xmlTextAsUInt(sz, error);
            }
        }
        else if (std::strcmp(node->Name(), "windows") == 0) {
            windows = xmlTextAsBool(node, error);
        }
    }
    calculateBitMembers();
    type = Type::File;
    return !error;
}

std::string Platform::getLimitsDefines(bool c99) const
{
    std::string s;

    // climits / limits.h
    s += "CHAR_BIT=";
    s += std::to_string(char_bit);
    s += ";SCHAR_MIN=";
    s += std::to_string(min_value(char_bit));
    s += ";SCHAR_MAX=";
    s += std::to_string(max_value(char_bit));
    s += ";UCHAR_MAX=";
    s += std::to_string(max_value(char_bit+1));
    s += ";CHAR_MIN=";
    if (defaultSign == 'u')
        s += std::to_string(min_value(char_bit));
    else
        s += std::to_string(0);
    s += ";CHAR_MAX=";
    if (defaultSign == 'u')
        s += std::to_string(max_value(char_bit+1));
    else
        s += std::to_string(max_value(char_bit));
    // TODO
    //s += ";MB_LEN_MAX=";
    s += ";SHRT_MIN=";
    s += std::to_string(min_value(short_bit));
    s += ";SHRT_MAX=";
    s += std::to_string(max_value(short_bit));
    s += ";USHRT_MAX=";
    s += std::to_string(max_value(short_bit+1));
    s += ";INT_MIN=";
    s += "(-" + std::to_string(max_value(int_bit)) + " - 1)";
    s += ";INT_MAX=";
    s += std::to_string(max_value(int_bit));
    s += ";UINT_MAX=";
    s += std::to_string(max_value(int_bit+1));
    s += ";LONG_MIN=";
    s += "(-" + std::to_string(max_value(long_bit)) + "L - 1L)";
    s += ";LONG_MAX=";
    s += std::to_string(max_value(long_bit)) + "L";
    s += ";ULONG_MAX=";
    s += std::to_string(max_value_unsigned(long_bit)) + "UL";
    if (c99) {
        s += ";LLONG_MIN=";
        s += "(-" + std::to_string(max_value(long_long_bit)) + "LL - 1LL)";
        s += ";LLONG_MAX=";
        s += std::to_string(max_value(long_long_bit)) + "LL";
        s += ";ULLONG_MAX=";
        s += std::to_string(max_value_unsigned(long_long_bit)) + "ULL";
    }

    // cstdint / stdint.h
    // FIXME: these are currently hard-coded in std.cfg
    /*
        INTMAX_MIN
        INTMAX_MAX
        UINTMAX_MAX
        INTN_MIN
        INTN_MAX
        UINTN_MAX
        INT_LEASTN_MIN
        INT_LEASTN_MAX
        UINT_LEASTN_MAX
        INT_FASTN_MIN
        INT_FASTN_MAX
        UINT_FASTN_MAX
        INTPTR_MIN
        INTPTR_MAX
        UINTPTR_MAX
        SIZE_MAX
        PTRDIFF_MIN
        PTRDIFF_MAX
        SIG_ATOMIC_MIN
        SIG_ATOMIC_MAX
        WCHAR_MIN
        WCHAR_MAX
        WINT_MIN
        WINT_MAX

        // function-like macros
        // implemented in std.cfg
        INTMAX_C
        UINTMAX_C
        INTN_C
        UINTN_C
     */

    // cfloat / float.h
    /*
        // TODO: implement
        FLT_RADIX

        FLT_MANT_DIG
        DBL_MANT_DIG
        LDBL_MANT_DIG

        FLT_DIG
        DBL_DIG
        LDBL_DIG

        FLT_MIN_EXP
        DBL_MIN_EXP
        LDBL_MIN_EXP

        FLT_MIN_10_EXP
        DBL_MIN_10_EXP
        LDBL_MIN_10_EXP

        FLT_MAX_EXP
        DBL_MAX_EXP
        LDBL_MAX_EXP

        FLT_MAX_10_EXP
        DBL_MAX_10_EXP
        LDBL_MAX_10_EXP

        FLT_MAX
        DBL_MAX
        LDBL_MAX

        FLT_EPSILON
        DBL_EPSILON
        LDBL_EPSILON

        FLT_MIN
        DBL_MIN
        LDBL_MIN

        FLT_ROUNDS

        // C99 / C++11 only
        FLT_EVAL_METHOD

        DECIMAL_DIG
     */

    return s;
}

std::string Platform::getLimitsDefines(Standards::cstd_t cstd) const
{
    return getLimitsDefines(cstd >= Standards::cstd_t::C99);
}

std::string Platform::getLimitsDefines(Standards::cppstd_t cppstd) const
{
    return getLimitsDefines(cppstd >= Standards::cppstd_t::CPP11);
}
