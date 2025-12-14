// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_VIRTUAL_CLOCK_H
#define MAGIQUE_VIRTUAL_CLOCK_H

#include <magique/core/Types.h>

//===============================
// Virtual Clock Module
//===============================
// ................................................................................
// This module allows to simulate virtual time.
// Note: Time starts at 00:00 at day 0 (so at night)
// Note: This clock only ticks when the game loop is running
// ................................................................................

namespace magique
{
    // Represents a point in time or interval
    struct VirtualTime final
    {
        VirtualTime() = default;
        // Values outside valid range wrap around and start at the beginning: hour=25 => hour=1
        VirtualTime(int hour, int minute);

        VirtualTime operator+(const VirtualTime& other) const;
        VirtualTime operator-(const VirtualTime& other) const;
        bool operator==(const VirtualTime& other) const;
        bool operator!=(const VirtualTime& other) const;
        // True if this is before - to the left on the time axis
        bool operator<(const VirtualTime& other) const;
        // True if this is after - to the right on the time axis
        bool operator>(const VirtualTime& other) const;
        bool operator<=(const VirtualTime& other) const;
        bool operator>=(const VirtualTime& other) const;

        // The whole time converted to seconds
        int toSeconds() const;

        int getSecond() const;
        int getMinute() const;
        int getHour() const;
        int getDay() const;

    private:
        int day;
        int hour;
        int minute;
        int second;
        friend VirtualClock;
    };

    struct VirtualClock final
    {
        // Creates a new virtual clock where a day (24 hours) takes the given minutes in real time
        // Default: 20 minutes (minecraft day)
        explicit VirtualClock(int realMinutes = 20);

        // Needs to be called each tick to update the clock
        void update();

        //================= TIME =================//

        // Returns the day from starting
        // Range: 0 - infinity
        [[nodiscard]] int getDay() const;

        // Returns the hours of the day in 24-hour format
        // Range: 0 - 23
        [[nodiscard]] int getHour() const;

        // Returns the minute of the hour
        // Range: 0 - 59
        [[nodiscard]] int getMinute() const;

        // Returns the second of the minute
        // Range: 0 - 59
        [[nodiscard]] int getSecond() const;

        // Gets the TOTAL amount of virtual seconds this clock has run
        [[nodiscard]] int getPassedSeconds() const;

        // Sets the given unit to the given value by either jumping forward or backward in time
        // Values have to be within their valid range as shown above - negative values are ignored
        void setTime(int hour, int minute, int second = -1, int day = -1);

        // Returns a time object of the current time
        VirtualTime getTime() const;

        // Returns true if the given duration has passed
        bool hasPassed(const VirtualTime& start, const VirtualTime& duration) const;

        //================= ADJUST =================//

        // Sets in real minutes how long a full day (24 hours) of the virtual clock should take
        void setRealMinutes(double realMinutes);
        [[nodiscard]] double getRealMinutes() const;

        // Set the paused status - if paused does not tick
        void setPaused(bool isPaused);
        [[nodiscard]] bool getIsPaused() const;

        // Sets how fast the time progresses
        // Default: 1.0
        void setTimeScale(float scale);

        // Adjusts the time as precisely as possible
        // Useful when syncing over e.g. a network
        void setByTicks(double ticks);
        [[nodiscard]] double getTicks() const;

        //================= UTILS =================//

        // Sets the virtual day time equal to the current real time - once set it progresses on its own again
        // Note: This only sets hour, minute and second NOT day!
        void syncTimeOfDay();

    private:
        [[nodiscard]] double getRealPassedSeconds() const;
        void adjustTicksByRealSeconds(double seconds);

        double realSecondSeconds = 1; // Real seconds this clock needs to pass a second
        double ticks = 0;
        double timeScale = 1.0F;
        bool isPaused = false;
        friend glz::meta<VirtualClock>;
    };

} // namespace magique

#endif //MAGIQUE_VIRTUAL_CLOCK_H
