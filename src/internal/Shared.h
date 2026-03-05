#ifndef MAGIQUE_SHARED_H
#define MAGIQUE_SHARED_H

namespace magique
{

    // In seconds
    struct BenchmarkEntry
    {
        void reset()
        {
            accumulated = 0;
            ticks = 0;
        }

        double add(const double seconds)
        {
            latest = seconds;
            accumulated += seconds;
            ticks++;
            return seconds;
        }

        float last() const { return (float)latest; }
        float lastMillis() const { return (float)latest * 1000.0F; }
        float getAvgSeconds() const { return static_cast<float>(accumulated / ticks); }
        float getAvgMillis() const { return static_cast<float>(accumulated / ticks * 1000); }

    private:
        double latest = 0;
        double accumulated = 0.0;
        double ticks = 0.01;
    };

} // namespace magique

#endif // MAGIQUE_SHARED_H
