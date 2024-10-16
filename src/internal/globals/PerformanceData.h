#ifndef PERFDATA_H
#define PERFDATA_H


#include "internal/datastructures/VectorType.h"
#include "external/raylib/src/coredata.h"

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
        uint32_t logicTickTime = 0;
        uint32_t drawTickTime = 0;
        int tickCounter = 0;
        int updateDelayTicks = 15;
        PerformanceBlock blocks[6]{}; // 5 blocks for FPS, CPU, GPU, DrawCalls, Upload, Download

#if MAGIQUE_PROFILING == 1
        vector<uint32_t> logicTimes;
        vector<uint32_t> drawTimes;
#endif

        PerformanceData()
        {
#if MAGIQUE_PROFILING == 1
            // Reserve much upfront to not impede benchmarks
            logicTimes.reserve(10000);
            drawTimes.reserve(10000);
#endif
        }

        void draw()
        {
            const auto drawBlock = [](const char* text, const Font& f, const float fs, const Vector2 pos, const float w)
            {
                const float blockHeight = fs * 1.15F;
                const float borderWidth = fs * 0.1F;
                const Vector2 textPosition = {pos.x + w * 0.07F, pos.y + (blockHeight - fs) / 2};

                const Rectangle container = {pos.x, pos.y, w, blockHeight};
                DrawRectangleRec(container, GRAY);
                DrawRectangleLinesEx(container, borderWidth, LIGHTGRAY);
                DrawTextEx(f, text, textPosition, fs, 0.5F, LIGHTGRAY);
                return w;
            };

            Vector2 position = {15, 0};
            const auto& font = global::ENGINE_CONFIG.font;
            for (const auto& block : blocks)
            {
                if (block.width == 0)
                    continue;
                constexpr auto fs = 20;
                position.x += drawBlock(block.text, font, fs, position, block.width);
            }
        }

        void updateValues()
        {
            tickCounter++;
            // Update values only every 10 ticks
            if (tickCounter != updateDelayTicks) [[likely]]
                return;

            auto& font = global::ENGINE_CONFIG.font;
            constexpr auto fs = 20;

            int block = 0;
            const int fps = GetFPS();
            snprintf(blocks[block].text, 32, "FPS: %d", fps);
            blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;

            block++;
            auto val = static_cast<float>(logicTickTime) / 1'000'000.0F; // nanos
            snprintf(blocks[block].text, 32, "CPU: %.1f", val);
            blocks[block].width = MeasureTextEx(font, blocks[block].text, fs, 1.0F).x * 1.1F;

            block++;
            val = static_cast<float>(drawTickTime) / 1'000'000.0F;
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

        void saveTickTime(const TickType t, const uint32_t time)
        {
            if (t == UPDATE)
            {
                logicTickTime = time;
#if MAGIQUE_PROFILING == 1
                logicTimes.push_back(time);
#endif
            }
            else if (t == DRAW)
            {
                drawTickTime = time;
#if MAGIQUE_PROFILING == 1
                drawTimes.push_back(time);
#endif
            }
        }

        // Dont even let it be there
#if MAGIQUE_PROFILING == 1
        [[nodiscard]] float getAverageTime(const TickType t)
        {
            vector<uint32_t>* times;
            if (t == UPDATE)
            {
                times = &logicTimes;
            }
            else if (t == DRAW)
            {
                times = &drawTimes;
            }
            else
            {
                return 0.0f;
            }

            if (times->empty())
            {
                return 0.0f;
            }
            times->pop_back();

            uint64_t sum = 0;
            for (const auto num : *times)
            {
                sum += num;
            }
            return static_cast<float>(sum) / static_cast<float>(times->size());
        }
#endif
    };

    namespace global
    {
        inline PerformanceData PERF_DATA;
    }

} // namespace magique

#endif //PERFDATA_H