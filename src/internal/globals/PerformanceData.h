// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PERFDATA_H
#define MAGIQUE_PERFDATA_H

#include <magique/multiplayer/Multiplayer.h>
#include <magique/gamedev/UsefulStuff.h>

#include "external/raylib-compat/rlgl_compat.h"
#include "internal/Shared.h"
#include "internal/utils/OSUtil.h"
#if defined(MAGIQUE_LAN) || defined(MAGIQUE_STEAM)
#include "external/networkingsockets/isteamnetworkingsockets.h"
#include "external/networkingsockets/steamnetworkingtypes.h"
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
            const auto& config = global::ENGINE_CONFIG;
            if (!config.showPerformanceOverlay)
                return;

            const auto drawBlock = [](const char* text, const Font& f, const float fs, const Vector2 pos, const float w)
            {
                const auto& theme = global::ENGINE_CONFIG.theme;
                const float blockHeight = fs * 1.15F;
                const float borderWidth = fs * 0.1F;
                const Vector2 textPosition = {pos.x + w * 0.07F, pos.y + (blockHeight - fs) / 2};

                const Rectangle container = {pos.x, pos.y, w, blockHeight};
                DrawRectangleRec(container, theme.backLight);
                DrawRectangleLinesEx(container, borderWidth, theme.backDark);
                DrawTextEx(f, text, textPosition, fs, 0.5F, theme.textPassive);
                return w;
            };

            Vector2 position = {0, 0};
            const auto& font = config.font;
            const auto fs = config.fontSize;

            for (const auto& block : blocks)
            {
                if (block.width == 0)
                    continue;
                position.x += drawBlock(block.text, font, fs, position, block.width);
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
            const auto fs = global::ENGINE_CONFIG.fontSize;

            int block = 0;
#define UPDATE_BLOCK(value, fmt)                                                                                        \
    snprintf(blocks[block].text, 32, fmt, value);                                                                       \
    blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;                                   \
    block++;

            UPDATE_BLOCK(GetFPS(), "FPS: %d");
            UPDATE_BLOCK(updateTick.getAvgMillis(), "Update: %.1f");
            UPDATE_BLOCK(drawTick.getAvgMillis(), "Draw: %.1f");
            UPDATE_BLOCK(GetPrevDrawCalls(), "Draw Calls: %.1d");

#if defined(MAGIQUE_STEAM) || defined(MAGIQUE_LAN)
            if (GetInMultiplayerSession())
            {
                float inBytes = 0;
                float outBytes = 0;
                float ping = 0.0F;
                if (GetIsClient())
                {
                    const auto conn = (HSteamNetConnection)GetCurrentConnections()[0];
                    SteamNetConnectionRealTimeStatus_t info{};
                    SteamNetworkingSockets()->GetConnectionRealTimeStatus(conn, &info, 0, nullptr);
                    inBytes = info.m_flInBytesPerSec;
                    outBytes = info.m_flOutBytesPerSec;
                    ping = (float)info.m_nPing;
                }
                else if (GetIsActiveHost())
                {
                    for (const auto conn : GetCurrentConnections()) // Only contains valid connections
                    {
                        SteamNetConnectionRealTimeStatus_t info{};
                        SteamNetworkingSockets()->GetConnectionRealTimeStatus(static_cast<HSteamNetConnection>(conn),
                                                                              &info, 0, nullptr);
                        inBytes += info.m_flInBytesPerSec;
                        outBytes += info.m_flOutBytesPerSec;
                        ping += (float)info.m_nPing;
                    }
                    ping /= (float)GetCurrentConnections().size();
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

#if MAGIQUE_PROFILING == 1
            const auto currentMemory = GetMemoryWorkingSet();
            if (currentMemory > maxMemoryBytes)
                maxMemoryBytes = currentMemory;
#endif
        }

        void printPerformanceStats()
        {
#if MAGIQUE_PROFILING == 0
            return;
#endif
            LOG_INFO("Performance Stats:");
            auto* fmt = "\t%-10s: Draw: %.2f ms | Update: %.2f ms\n";
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
