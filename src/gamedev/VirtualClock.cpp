#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <cmath>

#include <magique/gamedev/VirtualClock.h>
#include <magique/internal/Macros.h>

namespace magique
{
    VirtualClock::VirtualClock(const int realMinutes) { setRealMinutes(realMinutes); }

    int VirtualClock::getDay() const
    {
        const auto passedSeconds = getRealPassedSeconds();
        return static_cast<int>(passedSeconds / realDaySeconds);
    }

    int VirtualClock::getHour() const
    {
        const auto passedSeconds = getRealPassedSeconds();
        return static_cast<int>(std::fmod(passedSeconds, realDaySeconds) / realHourSeconds);
    }

    int VirtualClock::getMinute() const
    {
        const auto passedSeconds = getRealPassedSeconds();
        return static_cast<int>(std::fmod(passedSeconds, realHourSeconds) / realMinuteSeconds);
    }

    int VirtualClock::getSecond() const
    {
        const auto passedSeconds = getRealPassedSeconds();
        return static_cast<int>(std::round(std::fmod(passedSeconds, realMinuteSeconds) / realSecondSeconds));
    }

    int VirtualClock::getPassedSeconds() const
    {
        const auto passedSeconds = getRealPassedSeconds();
        return static_cast<int>(passedSeconds / realSecondSeconds);
    }

    void VirtualClock::setTime(const int hour, const int minute, const int second, const int day)
    {

        if (day >= 0)
        {
            MAGIQUE_ASSERT(day >= 0, "Invalid day value. Must be more or equal to 0");
            const auto diff = getDay() - day;
            adjustTicksByRealSeconds(static_cast<double>(diff) * realDaySeconds);
        }

        if (hour >= 0)
        {
            MAGIQUE_ASSERT(hour >= 0 && hour <= 24, "Invalid hour value. Must be between 0 and 24!");
            const auto diff = hour - getHour();
            auto adjustment = static_cast<double>(diff) * realHourSeconds;
            if (ticks + adjustment < 0)
                adjustment *= -1;
            adjustTicksByRealSeconds(adjustment);
        }

        if (minute >= 0)
        {
            MAGIQUE_ASSERT(minute >= 0 && minute <= 60, "Invalid minute value. Must be between 0 and 60!");
            const auto diff = minute - getMinute();
            auto adjustment = static_cast<double>(diff) * realMinuteSeconds;
            if (ticks + adjustment < 0)
                adjustment *= -1;
            adjustTicksByRealSeconds(adjustment);
        }

        if (second >= 0)
        {
            MAGIQUE_ASSERT(second >= 0 && second <= 60, "Invalid second value. Must be between 0 and 60!");
            const auto diff = second - getSecond();
            auto adjustment = static_cast<double>(diff) * realSecondSeconds;
            if (ticks + adjustment < 0)
                adjustment *= -1;
            adjustTicksByRealSeconds(adjustment);
        }
    }

    void VirtualClock::setRealMinutes(const int realMinutes)
    {
        realDaySeconds = static_cast<float>(realMinutes) * 60;
        realHourSeconds = realDaySeconds / 24;
        realMinuteSeconds = realDaySeconds / 1440;
        realSecondSeconds = realDaySeconds / 86400;
    }

    int VirtualClock::getRealMinutes() const { return static_cast<int>(realDaySeconds / 60.0F); }

    void VirtualClock::setPaused(const bool paused) { isPaused = paused; }

    bool VirtualClock::getIsPaused() const { return isPaused; }

    void VirtualClock::syncTimeOfDay()
    {
        // Get the current time
        const std::time_t now = std::time(nullptr); // Get the current system time in seconds since epoch
        const std::tm* localTime = std::localtime(&now);

        const int hours = localTime->tm_hour;
        const int minutes = localTime->tm_min;
        const int seconds = localTime->tm_sec;
        setTime(hours, minutes, seconds);
    }

    void VirtualClock::setTimeScale(const float scale) { timeScale = scale; }

    void VirtualClock::update()
    {
        if (!isPaused)
            ticks += 1.0F * timeScale;
    }

    double VirtualClock::getRealPassedSeconds() const
    {
        return ticks * (1.0 / static_cast<double>(MAGIQUE_LOGIC_TICKS));
    }

    void VirtualClock::adjustTicksByRealSeconds(const double seconds)
    {
        ticks += seconds / (1.0 / static_cast<double>(MAGIQUE_LOGIC_TICKS));
    }
} // namespace magique