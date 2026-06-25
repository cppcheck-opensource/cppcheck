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
#include <cstddef>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

class CPPCHECKLIB TimerResultsIntf {
public:
    virtual ~TimerResultsIntf() = default;

    virtual void addResults(const std::string& name, std::chrono::milliseconds duration) = 0;
};

class CPPCHECKLIB WARN_UNUSED TimerResults : public TimerResultsIntf {
public:
    TimerResults() = default;

    void showResults(size_t max_results = std::numeric_limits<size_t>::max(), bool metrics = true) const;
    void addResults(const std::string& name, std::chrono::milliseconds duration) override;

    void reset();

    std::map<std::string, std::vector<std::chrono::milliseconds>> getResults() const {
        std::lock_guard<std::mutex> l(mResultsSync);
        return mResults;
    }

protected:
    std::map<std::string, std::vector<std::chrono::milliseconds>> mResults;
    mutable std::mutex mResultsSync;
};

class CPPCHECKLIB Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    explicit Timer(std::string str, TimerResultsIntf* timerResults = nullptr);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void stop();

    template<class TFunc>
    static void run(std::string str, TimerResultsIntf* timerResults, const TFunc& f) {
        Timer t(std::move(str), timerResults);
        f();
    }

private:
    const std::string mName;
    TimePoint mStart;
    TimerResultsIntf* mResults{};
};

class CPPCHECKLIB OneShotTimer
{
public:
    explicit OneShotTimer(std::string name);
private:
    std::unique_ptr<TimerResultsIntf> mResults;
    std::unique_ptr<Timer> mTimer;
};

//---------------------------------------------------------------------------
#endif // timerH
