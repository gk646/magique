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
    struct VirtualClock final
    {
        // Creates a new virtual clock where a day (24 hours) takes the given minutes in real time
        // Default: equal to real time
        explicit VirtualClock(int realMinutes = 24 * 60);

        //================= TIME =================//

        // Returns the day from starting
        // Range: 0 - infinity
        [[nodiscard]] int getDay() const;

        // Returns the hours of the day in 24-hour format
        // Range: 0 - 24
        [[nodiscard]] int getHour() const;

        // Returns the minute of the hour
        // Range: 0 - 60
        [[nodiscard]] int getMinute() const;

        // Returns the second of the minute
        // Range: 0 - 60
        [[nodiscard]] int getSecond() const;

        // Gets the TOTAL amount of virtual seconds this clock has run
        [[nodiscard]] int getPassedSeconds() const;

        // Sets the given unit to the given value by either jumping forward or backward in time
        // Values have to be within their valid range as shown above - negative values are ignored
        void setTime(int hour, int minute, int second = -1, int day = -1);

        //================= ADJUST =================//

        // Sets in real minutes how long a full day (24 hours) of the virtual clock should take
        void setRealMinutes(int realMinutes);
        [[nodiscard]] int getRealMinutes() const;

        // Set the paused status - the paused clock does not
        void setPaused(bool isPaused);
        [[nodiscard]] bool getIsPaused() const;

        // Sets how fast the time progresses
        // Default: 1.0
        void setTimeScale(float scale);

        //================= UTILS =================//

        // Sets the virtual day time equal to the current real time - once set it progresses on its own again
        // Note: This only sets hour, minute and second not day!
        void syncTimeOfDay();

        //================= PERSISTENCE =================//

        // Returns an allocated pointer and size to save the current state of the virtual clock
        // IMPORTANT: allocates memory on each call
        [[nodiscard("Allocates")]] DataPointer<const unsigned char> getPersistenceData() const;

        // Restores the virtual clock to the state saved in the given data
        bool loadFromPersistenceData(const unsigned char* data, int size);

    private:
        void update();
        [[nodiscard]] double getRealPassedSeconds() const;
        void adjustTicksByRealSeconds(double seconds);

        double realDaySeconds = 24 * 60 * 60; // Real seconds this clock needs to a day
        double realHourSeconds = 60 * 60;     // Real seconds this clock needs to pass an hour
        double realMinuteSeconds = 60;        // Real seconds this clock needs to pass a minute
        double realSecondSeconds = 1;         // Real seconds this clock needs to pass a second
        double ticks = 0;
        double timeScale = 1.0F;
        bool isPaused = false;
    };

} // namespace magique

#endif //MAGIQUE_VIRTUAL_CLOCK_H