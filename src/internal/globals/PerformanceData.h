// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PERFDATA_H
#define MAGIQUE_PERFDATA_H

#include <magique/multiplayer/Networking.h>
#include <magique/gamedev/UsefulStuff.h>
#include <magique/util/RayUtils.h>

#include "external/raylib-compat/rlgl_compat.h"
#include "internal/Shared.h"
#include "internal/utils/OSUtil.h"
#ifdef MAGIQUE_LAN
#include "external/networkingsockets/steamnetworkingsockets.h"
#include "external/networkingsockets/isteamnetworkingutils.h"
#elif MAGIQUE_STEAM
#include <steam/steam_api.h>
#endif

namespace magique
{
    struct PerformanceBlock final
    {
        float width = 0;
        char text[20]{};
    };

    enum TickType
    {
        UPDATE,
        DRAW
    };

    struct PerformanceData final
    {
        BenchmarkEntry updateTick;
        BenchmarkEntry drawTick;
        Counter updateDelay{15};
        std::array<PerformanceBlock, 7> blocks; // 7 blocks for FPS, CPU, GPU, DrawCalls, Upload, Download, Ping
        uint64_t maxMemoryBytes = 0;

        void draw() const
        {
            const auto drawBlock = [](const char* text, const Vector2 pos, const float w)
            {
                const auto& config = global::ENGINE_CONFIG;
                const Vector2 textPosition = {pos.x + 3, pos.y + 1};
                const Rectangle container = {pos.x, pos.y, w, config.font.baseSize * 2.0F};

                DrawRectangleRec(container, config.theme.background);
                DrawRectangleLinesEx(container, 1, config.theme.backOutline);
                DrawTextEx(config.font, text, textPosition, config.font.baseSize * 2, 0.5F, config.theme.textPassive);
                return w;
            };

            const auto& config = global::ENGINE_CONFIG;
            if (!config.showPerformanceOverlay)
                return;

            Vector2 position = {0, 0};

            for (const auto& block : blocks)
            {
                if (block.width == 0)
                    continue;
                position.x += drawBlock(block.text, position, block.width);
                if (!config.showPerformanceOverlayExt) // Only draw FPS in simple mode
                    break;
            }
        }

        void updateValues()
        {
            if (!updateDelay.tick()) [[likely]]
            {
                return;
            }

            const auto& font = global::ENGINE_CONFIG.font;

            int block = 0;
#define UPDATE_BLOCK(value, fmt)                                                                                        \
    snprintf(blocks[block].text, 32, fmt, value);                                                                       \
    blocks[block].width = MeasureTextEx(font, blocks[block].text, font.baseSize * 2, 1.0F).x + 1;                       \
    block++;

            UPDATE_BLOCK(GetFPS(), "FPS: %d");
            UPDATE_BLOCK(updateTick.lastMillis(), "Update: %.1f");
            UPDATE_BLOCK(drawTick.lastMillis(), "Draw: %.1f");
            UPDATE_BLOCK(GetPrevDrawCalls(), "Draw Calls: %.1d");

#if defined(MAGIQUE_STEAM) || defined(MAGIQUE_LAN)
            if (NetworkInSession())
            {
                float inBytes = 0;
                float outBytes = 0;
                float ping = 0.0F;
                if (NetworkIsClient())
                {
                    const auto conn = (HSteamNetConnection)NetworkGetConnections()[0];
                    SteamNetConnectionRealTimeStatus_t info{};
                    SteamNetworkingSockets()->GetConnectionRealTimeStatus(conn, &info, 0, nullptr);
                    inBytes = info.m_flInBytesPerSec;
                    outBytes = info.m_flOutBytesPerSec;
                    ping = (float)info.m_nPing;
                }
                else if (NetworkIsHost())
                {
                    for (const auto conn : NetworkGetConnections()) // Only contains valid connections
                    {
                        SteamNetConnectionRealTimeStatus_t info{};
                        SteamNetworkingSockets()->GetConnectionRealTimeStatus(static_cast<HSteamNetConnection>(conn),
                                                                              &info, 0, nullptr);
                        inBytes += info.m_flInBytesPerSec;
                        outBytes += info.m_flOutBytesPerSec;
                        ping += (float)info.m_nPing;
                    }
                    ping /= std::max((float)NetworkGetConnections().size(), 1.0F);
                }

                UPDATE_BLOCK((int)inBytes, "In: %d");
                UPDATE_BLOCK((int)outBytes, "Out: %d");
                UPDATE_BLOCK((int)ping, "Ping: %d");
            }
            else
#endif
            {
                blocks[block++].width = 0;
                blocks[block++].width = 0;
                blocks[block++].width = 0;
            }

            const auto currentMemory = GetMemoryWorkingSet();
            if (currentMemory > maxMemoryBytes)
                maxMemoryBytes = currentMemory;
        }

        void printPerformanceStats() const
        {
            LOG_INFO("Performance Stats:");
            auto* fmt = "\t%-10s: Update: %.2f ms | Draw: %.2f ms\n";
            printf(fmt, "Avg Ticks", updateTick.getAvgMillis(), drawTick.getAvgMillis());
            fmt = "\t%-10s: Max: %.2f mb\n";
            printf(fmt, "Memory", maxMemoryBytes / 1'000'000.0F);
        }
    };

    namespace global
    {
        inline PerformanceData PERF_DATA;
    }

} // namespace magique

#endif // MAGIQUE_PERFDATA_H
