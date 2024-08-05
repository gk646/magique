#ifndef RENDERER_H
#define RENDERER_H

namespace magique::renderer
{
    inline double START_TIME = 0.0F;

    inline void Close()
    {
        for (uint_fast32_t i = 1; i < 15000; i++)
        {
            rlUnloadTexture(i);
        }
    }

    inline void StartTick()
    {
        START_TIME = glfwGetTime();
        PollInputEvents(); // This takes quite long and skews times
        BeginDrawing();
        AssignDrawTickCamera();
    }

    inline void EndTick()
    {
        if (global::CONFIGURATION.showPerformanceOverlay)
        {
            global::PERF_DATA.perfOverlay.draw();
        }
        EndDrawing();
        const double frameTime = glfwGetTime() - START_TIME;
        CORE.Time.frame = frameTime;
        CORE.Time.frameCounter++;
        global::PERF_DATA.saveTickTime(DRAW, static_cast<uint32_t>(frameTime * 1'000'000'000.0F));
    }

    inline void Tick(Game& game, entt::registry& registry, Camera2D& camera)
    {
        StartTick();
        {
            ClearBackground(RAYWHITE); // Thanks ray
            game.preRender();          // Pre render
            if (game.getIsRunning()) [[unlikely]]
            {
                HandleLoadingScreen(game);
                EndTick();
                return;
            }
            BeginMode2D(camera);
            {
                game.drawWorld(camera);
                global::LOGIC_TICK_DATA.lock();
                game.drawGame(registry, camera); // Draw game
                if (global::CONFIGURATION.showHitboxes) [[unlikely]]
                    RenderHitboxes(registry);
                global::LOGIC_TICK_DATA.unlock();
                RenderLighting(registry);
            }
            EndMode2D();
            game.drawUI(GetUIRoot()); // Draw UI
            global::COMMAND_LINE.draw();
        }
        EndTick();
    }


} // namespace magique::renderer


/*
 *#define CATCH_CONFIG_MAIN

//#include "../examples/asteroids/src/Asteroids.h"
#include "../src/external/cxstructs/cxutil/cxtime.h"

//#include <magique/ui/TextFormat.h>
//#include "MageQuest.h"

#include <chrono>
#include <thread>

using namespace std::chrono;

void updateTick(int& updates)
{
    for (int i = 1; i < 1005500; ++i)
    {
        volatile int b = i % i % i * 3 / 12312321 % 56666;
    }
    updates++;
}

void renderTick(int& fps)
{
    for (int i = 1; i < 100000; ++i)
    {
        volatile int b = i % i % i * 3 / 12312321 % 56666;
    }
    fps++;
}
#include <windows.h>

int main()
{
    int FPS = 90;
    int UPDATES = 60;
    timeBeginPeriod(1);
    auto func = [=]()
    {
         auto frameTime = nanoseconds(1'000'000'000 / FPS); // 120 FPS target

        auto previousTime = steady_clock::now();
        auto accu = nanoseconds::zero();
        auto updateAccu = nanoseconds::zero();
        auto totalUpdateTime = nanoseconds::zero();
        auto totalRenderTime = nanoseconds::zero();
        auto prevUpdateTime = nanoseconds::zero();
        auto prevRenderTime = nanoseconds::zero();

        float quota = (float)UPDATES / (float)FPS;
        float meter = 0.0F;
        int fps = 0;
        int updates = 0;

        while (true)
        {
            auto currentTime = steady_clock::now();
            auto elapsedTime = currentTime - previousTime;
            previousTime = currentTime;
            accu += elapsedTime;
            updateAccu += elapsedTime;

            const bool update = meter >= 1.0F;
            if (update)
            {
                meter -= 1.0F;
                auto startUpdate = steady_clock::now();
                updateTick(updates);
                auto endUpdate = steady_clock::now();
                prevUpdateTime = endUpdate - startUpdate;
                totalUpdateTime += prevUpdateTime;
            }

            auto startRender = steady_clock::now();
            renderTick(fps);
            auto endRender = steady_clock::now();
            prevRenderTime = endRender - startRender;
            totalRenderTime += prevRenderTime;

            meter += quota;

            auto sleepTime = frameTime - (prevRenderTime + (meter >= 1.0F) * prevUpdateTime);
            auto now = steady_clock::now();
            while (now < endRender + sleepTime)
            {
                now = steady_clock::now();
            }

            if (accu >= seconds(1))
            {
                printf("FPS: %d\n", fps);
                printf("Updates: %d\n", updates);
                printf("Average Update Time: %lld ns\n", totalUpdateTime.count() / updates);
                printf("Average Render Time: %lld ns\n", totalRenderTime.count() / fps);
                fps = 0;
                updates = 0;
                accu = nanoseconds::zero();
                totalUpdateTime = nanoseconds::zero();
                totalRenderTime = nanoseconds::zero();
            }
        }
    };


    timeEndPeriod(1);

    auto t = std::thread(func);

    if (t.joinable())
        t.join();


    return 0;
    //    MageQuest game{};
    // return game.run();
    return 0;
}
 **/
#endif //RENDERER_H