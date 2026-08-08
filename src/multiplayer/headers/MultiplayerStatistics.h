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
        startTick = magique::EngineGetTicks();
        incoming.reset();
        outgoing.reset();
    }
    void addOutgoing(const magique::Payload& payload) { outgoing.add(payload.type, payload.size); }
    void addIncoming(const magique::Payload& payload) { incoming.add(payload.type, payload.size); }

    magique::internal::MultiplayerStatsData getStats() const
    {
        return {incoming.getSorted(), outgoing.getSorted(), incoming.totalBytes, outgoing.totalBytes};
    }

private:
    struct DirectionData final
    {
        std::array<float, UINT8_MAX> counts{};
        std::array<float, UINT8_MAX> packetSize{};
        std::array<float, UINT8_MAX> totals{};
        float totalBytes;

        void reset()
        {
            counts = {};
            packetSize = {};
            totals = {};
            totalBytes = 0;
        }

        void add(MessageType type, int addSize)
        {
            counts[(int)type]++;
            packetSize[(int)type] = addSize;
            totals[(int)type] += addSize;
            totalBytes += addSize;
        }

        std::vector<magique::internal::PacketTypeStats> getSorted() const
        {
            std::vector<magique::internal::PacketTypeStats> ret{};
            for (int i = 0; i < UINT8_MAX; i++)
            {
                if (counts[i] > 0)
                    ret.emplace_back((MessageType)i, counts[i], packetSize[i], totals[i], totals[i] / totalBytes);
            }

            auto compareDescending =
                [](const magique::internal::PacketTypeStats& a, const magique::internal::PacketTypeStats& b)
            {
                return a.contrib > b.contrib;
            };
            std::ranges::sort(ret, compareDescending);
            return ret;
        }
    };
    DirectionData incoming{};
    DirectionData outgoing{};
    uint32_t startTick = 0;

#else
    void reset() {}
    void addOutgoing(const magique::Payload& payload) {}
    void addIncoming(const magique::Payload& payload) {}
    void print() { LOG_WARNING("Network stats only work in DEBUG"); }
    magique::internal::MultiplayerStatsData getStats() const { return {}; }
#endif
};

#endif // MAGEQUEST_MULTIPLAYERSTATISTICS_H
