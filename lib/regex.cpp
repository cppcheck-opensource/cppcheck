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

#ifdef HAVE_RULES

#include "regex.h"

#include <memory>
#include <utility>

#define PCRE2_CODE_UNIT_WIDTH 8
#ifdef _WIN32
#define PCRE2_STATIC
#endif
#include <pcre2.h>

namespace {
    std::string pcre2ErrorToString(const int errorcode)
    {
        PCRE2_UCHAR buffer[256];
        // a truncated message (PCRE2_ERROR_NOMEMORY) is still zero-terminated
        if (pcre2_get_error_message(errorcode, buffer, sizeof(buffer)) == PCRE2_ERROR_BADDATA)
            return "unknown PCRE2 error " + std::to_string(errorcode);
        return reinterpret_cast<const char*>(buffer);
    }

    class PcreRegex : public Regex
    {
    public:
        explicit PcreRegex(std::string pattern)
            : mPattern(std::move(pattern))
        {}

        ~PcreRegex() override
        {
            pcre2_code_free(mRe);
        }

        PcreRegex(const PcreRegex&) = delete;
        PcreRegex& operator=(const PcreRegex&) = delete;

        std::string compile();
        std::string match(const std::string& str, const MatchFn& matchFn) const override;

        Engine engine() const override {
            return Engine::Pcre;
        }

    private:
        std::string mPattern;
        pcre2_code* mRe{};
    };

    std::string PcreRegex::compile()
    {
        if (mRe)
            return "regular expression has already been compiled";

        int errorcode = 0;
        PCRE2_SIZE erroroffset = 0;
        pcre2_code * const re = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(mPattern.c_str()), mPattern.size(), 0, &errorcode, &erroroffset, nullptr);
        if (!re)
            return pcre2ErrorToString(errorcode);

        // JIT compilation is only an optimization - if it is not available the interpreter is used
        (void)pcre2_jit_compile(re, PCRE2_JIT_COMPLETE);

        mRe = re;

        return "";
    }

    std::string PcreRegex::match(const std::string& str, const MatchFn& matchFn) const
    {
        if (!mRe)
            return "regular expression has not been compiled yet";

        // the match data is created per call since a Regex may be shared between threads
        const std::unique_ptr<pcre2_match_data, decltype(&pcre2_match_data_free)> matchData(pcre2_match_data_create_from_pattern(mRe, nullptr), &pcre2_match_data_free);
        if (!matchData)
            return "failed to allocate PCRE2 match data";

        const auto * const subject = reinterpret_cast<PCRE2_SPTR>(str.c_str());
        const PCRE2_SIZE length = str.size();
        PCRE2_SIZE pos = 0;
        while (pos < length) {
            const int rc = pcre2_match(mRe, subject, length, pos, 0, matchData.get(), nullptr);
            if (rc == PCRE2_ERROR_NOMATCH)
                return "";
            if (rc < 0)
                return pcre2ErrorToString(rc) + " (pos: " + std::to_string(pos) + ")";

            const PCRE2_SIZE * const ovector = pcre2_get_ovector_pointer(matchData.get());
            const PCRE2_SIZE pos1 = ovector[0];
            const PCRE2_SIZE pos2 = ovector[1];

            matchFn(static_cast<int>(pos1), static_cast<int>(pos2));

            // continue at the end of the match - step forward on an empty match to avoid an endless loop
            pos = (pos2 > pos1) ? pos2 : (pos2 + 1);
        }

        return "";
    }
}

template<typename T>
static T* createAndCompileRegex(std::string pattern, std::string& err)
{
    T* regex = new T(std::move(pattern));
    err = regex->compile();
    return regex;
}

std::shared_ptr<Regex> Regex::create(std::string pattern, Engine engine, std::string& err)
{
    Regex* regex = nullptr;
    if (engine == Engine::Pcre)
        regex = createAndCompileRegex<PcreRegex>(std::move(pattern), err);
    else {
        err = "unknown regular expression engine";
    }
    if (!err.empty()) {
        delete regex;
        return nullptr;
    }
    return std::shared_ptr<Regex>(regex);
}

#endif // HAVE_RULES
