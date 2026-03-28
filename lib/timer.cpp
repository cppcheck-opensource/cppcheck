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

#include "timer.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

namespace {
    using dataElementType = std::pair<std::string, TimerResultsData>;
    bool more_second_sec(const dataElementType& lhs, const dataElementType& rhs)
    {
        return lhs.second.getSeconds() > rhs.second.getSeconds();
    }

    // TODO: remove and print through (synchronized) ErrorLogger instead
    std::mutex stdCoutLock;
}

// TODO: this does not include any file context when SHOWTIME_FILE thus rendering it useless - should we include the logging with the progress logging?
// that could also get rid of the broader locking
void TimerResults::showResults(ShowTime mode, bool metrics) const
{
    if (mode == ShowTime::NONE)
        return;
    std::vector<dataElementType> data;

    {
        std::lock_guard<std::mutex> l(mResultsSync);

        data.reserve(mResults.size());
        data.insert(data.begin(), mResults.cbegin(), mResults.cend());
    }
    std::sort(data.begin(), data.end(), more_second_sec);

    // lock the whole logging operation to avoid multiple threads printing their results at the same time
    std::lock_guard<std::mutex> l(stdCoutLock);

    size_t ordinal = 1; // maybe it would be nice to have an ordinal in output later!
    for (auto iter=data.cbegin(); iter!=data.cend(); ++iter) {
        if ((mode != ShowTime::TOP5_FILE && mode != ShowTime::TOP5_SUMMARY) || (ordinal<=5)) {
            const double sec = iter->second.getSeconds().count();
            std::cout << iter->first << ": " << sec << "s";
            if (metrics) {
                const double secAverage = sec / static_cast<double>(iter->second.mResults.size());
                const double secMin = asSeconds(*std::min_element(iter->second.mResults.cbegin(), iter->second.mResults.cend())).count();
                const double secMax = asSeconds(*std::max_element(iter->second.mResults.cbegin(), iter->second.mResults.cend())).count();
                std::cout << " (avg. " << secAverage << "s / min " << secMin << "s / max " << secMax << "s - " << iter->second.mResults.size() << " result(s))";
            }
            std::cout << std::endl;
        }
        ++ordinal;
    }
}

void TimerResults::addResults(const std::string& name, std::chrono::milliseconds duration)
{
    std::lock_guard<std::mutex> l(mResultsSync);

    mResults[name].mResults.push_back(duration);
}

void TimerResults::reset()
{
    std::lock_guard<std::mutex> l(mResultsSync);
    mResults.clear();
}

Timer::Timer(std::string str, ShowTime showtimeMode, TimerResultsIntf* timerResults)
    : mName(std::move(str))
    , mMode(showtimeMode)
    , mStart(Clock::now())
    , mResults(timerResults)
{}

Timer::~Timer()
{
    stop();
}

void Timer::stop()
{
    if (mMode == ShowTime::NONE)
        return;
    if (mStart != TimePoint{}) {
        if (!mResults) {
            assert(false);
        }
        else {
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - mStart);
            mResults->addResults(mName, diff);
        }
    }
    mMode = ShowTime::NONE; // prevent multiple stops
}

static std::string durationToString(std::chrono::milliseconds duration)
{
    // Extract hours
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    duration -= hours; // Subtract the extracted hours

    // Extract minutes
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
    duration -= minutes; // Subtract the extracted minutes

    // Extract seconds
    std::chrono::duration<double> seconds = std::chrono::duration_cast<std::chrono::duration<double>>(duration);

    std::string ellapsedTime;
    if (hours.count() > 0)
        ellapsedTime += std::to_string(hours.count()) + "h ";
    if (minutes.count() > 0)
        ellapsedTime += std::to_string(minutes.count()) + "m ";
    std::string secondsStr{std::to_string(seconds.count())};
    auto pos = secondsStr.find_first_of('.');
    if (pos != std::string::npos && (pos + 4) < secondsStr.size())
        secondsStr.resize(pos + 4); // keep three decimal
    return (ellapsedTime + secondsStr + "s");
}

OneShotTimer::OneShotTimer(std::string name, ShowTime showtime)
{
    if (showtime == ShowTime::NONE)
        return;

    class MyResults : public TimerResultsIntf
    {
    private:
        void addResults(const std::string &name, std::chrono::milliseconds duration) override
        {
            std::lock_guard<std::mutex> l(stdCoutLock);

            // TODO: do not use std::cout directly
            std::cout << name << ": " << durationToString(duration) << std::endl;
        }
    };

    mResults.reset(new MyResults);
    mTimer.reset(new Timer(std::move(name), showtime, mResults.get()));
}
