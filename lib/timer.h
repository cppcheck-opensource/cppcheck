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
#ifndef timerH
#define timerH
//---------------------------------------------------------------------------

#include "config.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

enum class ShowTime : std::uint8_t {
    NONE,
    FILE,
    FILE_TOTAL,
    SUMMARY,
    TOP5_SUMMARY,
    TOP5_FILE
};

class CPPCHECKLIB TimerResultsIntf {
public:
    virtual ~TimerResultsIntf() = default;

    virtual void addResults(const std::string& name, std::chrono::milliseconds duration) = 0;

    static std::chrono::duration<double> asSeconds(std::chrono::milliseconds ms) {
        return std::chrono::duration_cast<std::chrono::duration<double>>(ms);
    }
};

struct TimerResultsData {
    std::vector<std::chrono::milliseconds> mResults;

    std::chrono::duration<double> getSeconds() const {
        return std::accumulate(mResults.cbegin(), mResults.cend(), std::chrono::duration<double>{}, [](std::chrono::duration<double> secs, std::chrono::milliseconds duration) {
            return secs + TimerResultsIntf::asSeconds(duration);
        });
    }
};

class CPPCHECKLIB WARN_UNUSED TimerResults : public TimerResultsIntf {
public:
    TimerResults() = default;

    void showResults(ShowTime mode, bool metrics = true) const;
    void addResults(const std::string& name, std::chrono::milliseconds duration) override;

    void reset();

private:
    std::map<std::string, TimerResultsData> mResults;
    mutable std::mutex mResultsSync;
};

class CPPCHECKLIB Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    Timer(std::string str, ShowTime showtimeMode, TimerResultsIntf* timerResults = nullptr);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void stop();

    template<class TFunc>
    static void run(std::string str, ShowTime showtimeMode, TimerResultsIntf* timerResults, const TFunc& f) {
        Timer t(std::move(str), showtimeMode, timerResults);
        f();
    }

private:
    const std::string mName;
    ShowTime mMode{};
    TimePoint mStart;
    TimerResultsIntf* mResults{};
};

class CPPCHECKLIB OneShotTimer
{
public:
    OneShotTimer(std::string name, ShowTime showtime);
private:
    std::unique_ptr<TimerResultsIntf> mResults;
    std::unique_ptr<Timer> mTimer;
};

//---------------------------------------------------------------------------
#endif // timerH
