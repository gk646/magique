#include <cstdio>
#include <raylib/raylib.h>

#include "PerformanceDisplay.h"
#include "internal/globals/EngineConfig.h"
#include "internal/globals/PerformanceData.h"
#include "external/raylib/src/coredata.h"


#if MAGIQUE_MULITPLAYER_SUPPORT == 1
#include <steam/steam_api.h>
#include "multiplayer/MultiplayerSession.h"
void AssignConnectionInfo(bool isHost, float& inBytes, float& outBytes)
{
    inBytes = 0;
    outBytes = 0;
    if (isHost)
    {
        const auto conn = Glob::GDT.mp.session->p2p.connections[0];
        SteamNetConnectionRealTimeStatus_t info{};
        SteamNetworkingSockets()->GetConnectionRealTimeStatus(conn, &info, 0, nullptr);
        inBytes += info.m_flInBytesPerSec;
        outBytes += info.m_flOutBytesPerSec;
    }
    else
    {
        for (const auto conn : Glob::GDT.mp.session->p2p.connections)
        {
            if (conn == k_HSteamNetConnection_Invalid)
                continue;
            SteamNetConnectionRealTimeStatus_t info{};
            SteamNetworkingSockets()->GetConnectionRealTimeStatus(conn, &info, 0, nullptr);
            inBytes += info.m_flInBytesPerSec;
            outBytes += info.m_flOutBytesPerSec;
        }
    }
}
#endif

float DrawBlock(const char* text, const Font& f, const float fs, const Vector2 pos, const float w)
{
    const float blockHeight = fs * 1.15F;
    const float borderWidth = fs * 0.1F;
    const Vector2 textPosition = {pos.x + w * 0.07F, pos.y + (blockHeight - fs) / 2};

    const Rectangle container = {pos.x, pos.y, w, blockHeight};
    DrawRectangleRec(container, GRAY);
    DrawRectangleLinesEx(container, borderWidth, LIGHTGRAY);
    DrawTextEx(f, text, textPosition, fs, 0.5F, LIGHTGRAY);
    return w;
}

void PerformanceDisplay::updateValues()
{
    tickCounter++;
    // Update values only every 10 ticks
    if (tickCounter != updateDelayTicks) [[likely]]
        return;

    auto& font = magique::global::ENGINE_CONFIG.font;
    constexpr auto fs = 20;

    int block = 0;
    const int fps = GetFPS();
    snprintf(blocks[block].text, 32, "FPS: %d", fps);
    blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;

    block++;
    auto val = static_cast<float>(magique::global::PERF_DATA.logicTickTime) / 1'000'000.0F; // nanos
    snprintf(blocks[block].text, 32, "CPU: %.1f", val);
    blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;

    block++;
    val = static_cast<float>(magique::global::PERF_DATA.drawTickTime) / 1'000'000.0F;
    snprintf(blocks[block].text, 32, "GPU: %.1f", val);
    blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;

    block++;
    const int calls = RLGL.State.prevDrawCalls;
    snprintf(blocks[block].text, 32, "Draw Calls: %.1d", calls);
    blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;

#if MAGIQUE_MULITPLAYER_SUPPORT == 1
    if (Glob::GDT.mp.session)
    {
        float inBytes, outBytes;
        AssignConnectionInfo(Glob::GDT.mp.session->isHost, inBytes, outBytes);

        block++;
        val = inBytes;
        snprintf(blocks[block].text, 32, "In: %d", (int)val);
        blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;

        block++;
        val = outBytes;
        snprintf(blocks[block].text, 32, "Out: %d", (int)val);
        blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;
    }
    else
#endif
    {
        block++;
        blocks[block].width = 0;
        block++;
        blocks[block].width = 0;
    }

    tickCounter = 0;
}

void PerformanceDisplay::draw()
{
    Vector2 position = {15, 0};
    auto& font = magique::global::ENGINE_CONFIG.font;
    const auto fs = 20;
    for (const auto& block : blocks)
    {
        if (block.width == 0)
            continue;
        position.x += DrawBlock(block.text, font, fs, position, block.width);
    }
}