// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <ctime>
#include <cmath>
#include <cstdlib>

#include <magique/gamedev/VirtualClock.h>


namespace magique
{
    VirtualTime::VirtualTime(int seconds, int minutes, int hours, int days)
    {
        seconds = ((days * 24 + hours) * 60 + minutes) * 60 + seconds;
        div_t daysrem = div(seconds, 24 * 60 * 60);
        day = daysrem.quot;

        div_t hoursrem = div(daysrem.rem, 60 * 60);
        hour = hoursrem.quot;

        div_t minutesrem = div(hoursrem.rem, 60);
        minute = minutesrem.quot;

        second = minutesrem.rem;
    }

    VirtualTime VirtualTime::operator+(const VirtualTime& other) const { return {toSeconds() + other.toSeconds()}; }

    VirtualTime VirtualTime::operator-(const VirtualTime& other) const { return {toSeconds() - other.toSeconds()}; }

    VirtualTime& VirtualTime::operator+=(const VirtualTime& other)
    {
        *this = *this + other;
        return *this;
    }

    VirtualTime& VirtualTime::operator-=(const VirtualTime& other)
    {
        *this = *this - other;
        return *this;
    }

    bool VirtualTime::operator==(const VirtualTime& other) const { return toSeconds() == other.toSeconds(); }

    bool VirtualTime::operator!=(const VirtualTime& other) const { return !(*this == other); }

    bool VirtualTime::operator<(const VirtualTime& other) const { return toSeconds() < other.toSeconds(); }

    bool VirtualTime::operator>(const VirtualTime& other) const { return toSeconds() > other.toSeconds(); }

    bool VirtualTime::operator<=(const VirtualTime& other) const { return toSeconds() <= other.toSeconds(); }

    bool VirtualTime::operator>=(const VirtualTime& other) const { return toSeconds() >= other.toSeconds(); }

    VirtualTime& VirtualTime::operator*=(float scale)
    {
        *this = *this * scale;
        return *this;
    }

    VirtualTime VirtualTime::operator*(float scale) const
    {
        const float seconds = (float)toSeconds() * scale;
        return {(int)std::round(seconds)};
    }

    int VirtualTime::toSeconds() const { return ((day * 24 + hour) * 60 + minute) * 60 + second; }

    int VirtualTime::getSeconds() const { return second; }

    int VirtualTime::getMinutes() const { return minute; }

    int VirtualTime::getHours() const { return hour; }

    int VirtualTime::getDays() const { return day; }

    std::string VirtualTime::toString() const
    {
        return TextFormat("%id:%ih:%im:%is", getDays(), getHours(), getMinutes(), getSeconds());
    }

    VirtualClock::VirtualClock(const int realMinutes) { setRealMinutes(realMinutes); }

    static constexpr float DAY_SECONDS = 24 * 60 * 60;
    static constexpr float HOUR_SECONDS = 60 * 60;
    static constexpr float MIN_SECONDS = 60;

#define realDaySeconds (realSecondSeconds * DAY_SECONDS)
#define realHourSeconds (realSecondSeconds * HOUR_SECONDS)
#define realMinuteSeconds (realSecondSeconds * MIN_SECONDS)

    void VirtualClock::update()
    {
        if (!isPaused)
            ticks += 1.0F * timeScale;
    }

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

    VirtualTime VirtualClock::getTime() const
    {
        VirtualTime time{};
        time.day = getDay();
        time.hour = getHour();
        time.minute = getMinute();
        time.second = getSecond();
        return time;
    }

    bool VirtualClock::hasPassed(const VirtualTime& start, const VirtualTime& duration) const
    {
        const auto combined = start + duration;
        return combined > getTime();
    }

    void VirtualClock::setRealMinutes(const double realMinutes)
    {
        const auto daySeconds = 24.0 * 60.0 * 60.0;
        realSecondSeconds = (realMinutes * 60) / daySeconds;
    }

    double VirtualClock::getRealMinutes() const { return realDaySeconds / 60.0; }

    void VirtualClock::setPaused(const bool paused) { isPaused = paused; }

    bool VirtualClock::getIsPaused() const { return isPaused; }

    void VirtualClock::setTimeScale(const float scale) { timeScale = scale; }

    void VirtualClock::setByTicks(double newTicks) { ticks = newTicks; }

    double VirtualClock::getTicks() const { return ticks; }

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

    double VirtualClock::getRealPassedSeconds() const
    {
        return ticks * (1.0 / static_cast<double>(MAGIQUE_LOGIC_TICKS));
    }

    void VirtualClock::adjustTicksByRealSeconds(const double seconds)
    {
        ticks += seconds / (1.0 / static_cast<double>(MAGIQUE_LOGIC_TICKS));
    }
} // namespace magique
