#ifndef MAGEQUEST_MULTIPLAYERSTATISTICS_H
#define MAGEQUEST_MULTIPLAYERSTATISTICS_H

#ifdef MAGIQUE_DEBUG
#include <algorithm>
#include <magique/core/Core.h>
#endif

struct MultiplayerStatistics final
{
#ifdef MAGIQUE_DEBUG
    void reset()
    {
        startTick = magique::GetEngineTick();
        incoming.reset();
        outgoing.reset();
    }
    void addOutgoing(MessageType type, int size) { outgoing.add(type, size); }
    void addIncoming(MessageType type, int size) { incoming.add(type, size); }
    void print() const
    {
        const float passedTicks = magique::GetEngineTick() - startTick;
        printf("Message Statistics:\n");
        printf("--------------------------------------\n");
        printf("Over %.2f seconds\n", passedTicks / MAGIQUE_LOGIC_TICKS);

        printf("Incoming Messages:\n");
        incoming.print(passedTicks);

        printf("\nOutgoing Messages:\n");
        outgoing.print(passedTicks);
    }
private:
    struct DirectionData final
    {
        uint32_t counts[UINT8_MAX]{};
        uint32_t size;

        struct MessageCount
        {
            uint8_t type;
            uint32_t count;
        };
        void reset()
        {
            memset(counts, 0, sizeof(counts));
            size = 0;
        }
        void print(const float passed) const
        {
            MessageCount arr[UINT8_MAX];
            for (int i = 0; i < UINT8_MAX; i++)
            {
                arr[i] = {(uint8_t)i, counts[i]};
            }
            auto compareDescending = [](const MessageCount& a, const MessageCount& b) { return a.count > b.count; };
            std::sort(arr, arr + UINT8_MAX, compareDescending);

            uint32_t total = 0;
            for (const auto& entry : arr)
            {
                if (entry.count > 0)
                {
                    const float avg = static_cast<float>(entry.count) / passed;
                    printf("  [%2d] %5d messages (%.2f/tick)\n", (int)entry.type, entry.count, avg);
                }
                total += entry.count;
            }
            printf("  Total:\n");
            printf("    %u messages (%.2f/tick)\n", total, static_cast<float>(total) / passed);
            printf("    %u bytes (%.2f/tick)\n", size, static_cast<float>(size) / passed);
            const auto passedSeconds = passed / MAGIQUE_LOGIC_TICKS;
            printf("    %.2f MB/h\n", ((static_cast<float>(size) / passedSeconds) * 60 * 60) / 1'000'000);
        }
        void add(MessageType type, int addSize)
        {
            counts[(int)type]++;
            size += addSize;
        }
    };
    DirectionData incoming{};
    DirectionData outgoing{};
    uint32_t startTick = 0;
#else
    void reset() {}
    void addOutgoing(MessageType type, int size) {}
    void addIncoming(MessageType type, int size) {}
    void print() { LOG_WARNING("Network stats only work in DEBUG"); }
#endif
};

#endif //MAGEQUEST_MULTIPLAYERSTATISTICS_H
