#include <magique/core/Game.h>
#include <magique/core/Core.h>

#include "core/CoreData.h"

#include "Renderer.h"
#include "Updater.h"


namespace magique
{
    Game::Game(const char* name) : gameName(name)
    {
        _isRunning = true;
#ifdef MAGIQUE_DEBUG
        SetTraceLogLevel(LOG_WARNING);
#endif
        SetTraceLogLevel(LOG_WARNING);
        InitWindow(1280, 720, name);
        InitAudioDevice();
        SetExitKey(0);
        SetRandomSeed(rand() ^ std::chrono::steady_clock::now().time_since_epoch().count());
        LOG_INFO("Initialized Game");
        camera.zoom = 1.0F;
        InitMagique();
    }

    Game::~Game()
    {
        CloseAudioDevice();
        CloseWindow();
    }

    int Game::run(const char* assetPath, const uint64_t encryptionKey)
    {
        CURRENT_GAME_LOADER = new GameLoader{assetPath, encryptionKey};
        onStartup(*static_cast<GameLoader*>(CURRENT_GAME_LOADER));
        static_cast<GameLoader*>(CURRENT_GAME_LOADER)
            ->registerTask(
                [](AssetContainer&)
                {
                    for (auto& atlas : TEXTURE_ATLASES)
                    {
                        atlas.loadToGPU();
                    }
                },
                MAIN_THREAD, LOW);
        static_cast<GameLoader*>(CURRENT_GAME_LOADER)->printStats();

        _isLoading = true;

        updater::Run(_isRunning, *this);
        renderer::Run(_isLoading, *this);

        renderer::Close();
        updater::Close();

#if MAGIQUE_DEBUG == 1
        LOG_INFO("Average DrawTick: %d nanos", (int)PERF_DATA.getAverageTime(DRAW));
        LOG_INFO("Average LogicTick: %d nanos", (int)PERF_DATA.getAverageTime(UPDATE));
#endif

        return 0;
    }


} // namespace magique