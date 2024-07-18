#include <magique/core/Game.h>
#include <magique/core/Core.h>
#include <magique/assets/AssetLoader.h>
#include <magique/persistence/container/GameConfig.h>
#include "cxstructs/SmallVector.h"
#include <raylib/rlgl.h>

#include "../external/raylib/src/external/glad.h"
#include "../external/raylib/src/coredata.h"
#include "../external/raylib/src/external/glfw/include/GLFW/glfw3.h"

#include "globals/LogicTickData.h"
#include "globals/DrawTickData.h"
#include "globals/Configuration.h"
#include "globals/PerfData.h"
#include "globals/LogicThread.h"
#include "globals/Shaders.h"
#include "globals/TextureAtlas.h"
#include "CoreData.h"

#include "../ecs/systems/CollisionSystem.h"
#include "../ecs/systems/InputSystem.h"
#include "../ecs/systems/LogicSystem.h"
#include "../ecs/systems/LightingSystem.h"

#if defined(_WIN32)
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <processthreadsapi.h>
#endif

#include "headers/GameUtil.h"
#include "internal/headers/OSUtil.h"

#include "headers/Updater.h"
#include "headers/Renderer.h"

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
        LOG_INFO("Initialized Game: %s", gameName);
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