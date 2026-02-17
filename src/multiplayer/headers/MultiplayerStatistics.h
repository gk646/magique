#ifndef MAGEQUEST_MULTIPLAYERSTATISTICS_H
#define MAGEQUEST_MULTIPLAYERSTATISTICS_H

#ifdef MAGIQUE_DEBUG
#include <algorithm>
#include <cstring>
#include <magique/core/Engine.h>
#endif
#include "magique/internal/InternalTypes.h"

struct MultiplayerStatistics final
{
#ifdef MAGIQUE_DEBUG
    void reset()
    {
        startTick = magique::EngineGetTick();
        incoming.reset();
        outgoing.reset();
    }
    void addOutgoing(const magique::Payload& payload) { outgoing.add(payload.type, payload.size); }
    void addIncoming(const magique::Payload& payload) { incoming.add(payload.type, payload.size); }

    magique::internal::MultiplayerStatsData getStats() const
    {
        return {incoming.getSorted(), outgoing.getSorted(), incoming.size, outgoing.size};
    }

private:
    struct DirectionData final
    {
        uint32_t counts[UINT8_MAX]{};
        uint32_t size;

        void reset()
        {
            std::memset(counts, 0, sizeof(counts));
            size = 0;
        }
        void add(MessageType type, int addSize)
        {
            counts[(int)type]++;
            size += addSize;
        }
        std::array<magique::internal::MessageCount, UINT8_MAX> getSorted() const
        {
            std::array<magique::internal::MessageCount, UINT8_MAX> arr{};
            for (int i = 0; i < UINT8_MAX; i++)
            {
                arr[i] = {(MessageType)i, counts[i]};
            }
            auto compareDescending =
                [](const magique::internal::MessageCount& a, const magique::internal::MessageCount& b)
            {
                return a.count > b.count;
            };
            std::ranges::sort(arr, compareDescending);
            return arr;
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
    magique::internal::MultiplayerStatsData getStats() const { return {}; }
#endif
};

#endif // MAGEQUEST_MULTIPLAYERSTATISTICS_H
