// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MAIN_THREAD_UTIL_H
#define MAGIQUE_MAIN_THREAD_UTIL_H

// raylib functions implemented here

extern "C"
{
    void SetTargetFPS(const int fps)
    {
        if (fps < MAGIQUE_LOGIC_TICKS)
        {
            LOG_WARNING("FPS must be more than logic tickrate!");
            return;
        }
        auto& config = magique::global::ENGINE_CONFIG.timing;
        config.frameTarget = 1.0 / static_cast<double>(fps);
        // Round down cause minimal accuracy is only 1ms - so wait 1 ms less to be accurate
        config.sleepTime = std::floor(config.frameTarget * 1000) / 1000;
        config.workPerTick = MAGIQUE_LOGIC_TICKS / static_cast<double>(fps);
    }

    int GetFPS()
    {
        auto& config = magique::global::ENGINE_CONFIG.timing;
#define FPS_BUFF_SIZE 15
        static double lastTime = 0;
        static int fpsBuffer[FPS_BUFF_SIZE] = {0};
        static int index = 0;
        static int sumFPS = 0;
        static float count = 0;

        const double currentTime = GetTime();

        const int currFPS =
            static_cast<int>(std::round(static_cast<double>(config.frameCounter) / (currentTime - lastTime)));

        sumFPS -= fpsBuffer[index];
        fpsBuffer[index] = currFPS;
        sumFPS += currFPS;
        index = (index + 1) % FPS_BUFF_SIZE;

        if (count < FPS_BUFF_SIZE)
        {
            count++;
        }
        const int ret = static_cast<int>(std::round(static_cast<float>(sumFPS) / count));

        lastTime = currentTime;
        config.frameCounter = 0;

        return ret;
    }

    // Get time in seconds for last frame drawn (delta time)
    float GetFrameTime()
    {
        const auto& perf = magique::global::PERF_DATA;
        return perf.drawTick.last();
    }
}

#endif // MAGIQUE_MAIN_THREAD_UTIL_H
