#include <magique/core/Game.h>
#include <magique/core/Core.h>
#include <magique/assets/AssetLoader.h>
#include <magique/persistence/types/GameConfig.h>
#include <cxstructs/SmallVector.h>

#include "external/raylib/src/rlgl.h"

#include "core/globals/LogicTickData.h"
#include "core/globals/DrawTickData.h"
#include "core/globals/Configuration.h"
#include "core/globals/PerfData.h"
#include "core/globals/LogicThread.h"
#include "core/globals/Shaders.h"
#include "core/globals/TextureAtlas.h"
#include "core/CoreData.h"

#include "Updater.h"
#include "Renderer.h"

CoreData CORE = {0};
rlglData RLGL = {0};

namespace magique
{
    Game::Game(const char* name) : gameName(name)
    {
        _isRunning = true;
#ifdef MAGIQUE_DEBUG
        SetTraceLogLevel(LOG_WARNING);
#endif
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(1280, 720, name);
        InitAudioDevice();
        SetExitKey(0);
        SetRandomSeed(rand() ^ std::chrono::steady_clock::now().time_since_epoch().count());
        global::LOGIC_TICK_DATA.camera.zoom = 1.0F;
        InitMagique();
        LOG_INFO("Initialized Game");
    }

    Game::~Game()
    {
        CloseAudioDevice();
        CloseWindow();
    }

    int Game::run(const char* assetPath, const char* configPath, const uint64_t encryptionKey)
    {
        auto& loader = global::CURRENT_GAME_LOADER;
        loader = new AssetLoader{assetPath, encryptionKey};

        //TODO load config
        GameConfig config;

        // Call startup
        onStartup(*static_cast<AssetLoader*>(loader), config);

        // Load atlas to gpu
        const auto loadAtlasGPU = [](AssetContainer&)
        {
            for (auto& atlas : global::TEXTURE_ATLASES)
            {
                atlas.loadToGPU();
            }
        };
        static_cast<AssetLoader*>(loader)->registerTask(loadAtlasGPU, MAIN_THREAD, LOW);
        static_cast<AssetLoader*>(loader)->printStats();
        _isLoading = true;

        // Start threads
        updater::Run(_isRunning, *this);
        renderer::Run(_isLoading, *this);

        onShutDown(config);
        renderer::Close();
        updater::Close();

#ifdef MAGIQUE_DEBUG_PROFILE
        LOG_INFO("Average DrawTick: %dk nanos", (int)global::PERF_DATA.getAverageTime(DRAW) / 1'000);
        LOG_INFO("Average LogicTick: %dk nanos", (int)global::PERF_DATA.getAverageTime(UPDATE) / 1'000);
#endif
        return 0;
    }

    void Game::shutDown() { _isRunning = false; }


} // namespace magique