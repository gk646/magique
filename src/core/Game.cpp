// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <random> // Needed for non-deterministic hardware randomness
#include <cxstructs/SmallVector.h>
#include <raylib/config.h>
#include <raylib/raylib.h>
#include <raylib/rlgl.h>

#include <magique/core/Game.h>
#include <magique/core/Core.h>
#include <magique/core/Draw.h>
#include <magique/core/Debug.h>
#include <magique/core/CollisionDetection.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Components.h>
#include <magique/ecs/Scripting.h>
#include <magique/ui/UI.h>
#include <magique/assets/AssetLoader.h>
#include <magique/util/JobSystem.h>
#include <magique/core/GameConfig.h>
#include <magique/gamedev/Achievements.h>
#include <magique/ui/WindowManager.h>

#include "internal/globals/EngineData.h"
#include "internal/globals/EngineConfig.h"
#include "internal/globals/LoadingData.h"
#include "internal/globals/PerformanceData.h"
#include "internal/globals/PathFindingData.h"
#include "internal/globals/ShaderData.h"
#include "internal/globals/TextureAtlas.h"
#include "internal/globals/AudioPlayer.h"
#include "internal/globals/ScriptData.h"
#include "internal/globals/UIData.h"
#include "internal/globals/ConsoleData.h"
#include "internal/globals/ParticleData.h"
#include "internal/globals/StaticCollisionData.h"
#include "internal/globals/DynamicCollisionData.h"
#ifdef MAGIQUE_STEAM
#include "internal/globals/SteamData.h"
#include "internal/globals/MultiplayerData.h"
#elif MAGIQUE_LAN
#include "internal/globals/MultiplayerData.h"
#endif
#include "internal/utils/CollisionPrimitives.h"
#include "internal/misc/IncludeWindows.h"
#include "internal/utils/OSUtil.h"
#include "internal/globals/JobScheduler.h"

#include "external/raylib-compat/rcore_compat.h"
#include "external/raylib/src/external/glad.h"

#include "internal/systems/StaticCollisionSystem.h"
#include "internal/systems/DynamicCollisionSystem.h"
#include "internal/systems/InputSystem.h"
#include "internal/systems/LogicSystem.h"
#include "internal/systems/LightingSystem.h"

#include "core/headers/MainThreadUtil.h"
#include "core/headers/Updater.h"
#include "core/headers/Renderer.h"
#include "core/headers/MainThread.h"
#if MAGIQUE_INCLUDE_FONT == 1
#include "internal/misc/CascadiaCode.h"
#endif

// Note: All includes are pulled out topside for clarity
// Here the whole render and update loops happen

namespace magique
{
    namespace internal
    {
        bool InitMagique()
        {
            static bool initCalled = false;
            if (initCalled)
            {
                LOG_WARNING("Init called twice. Skipping...");
                return false;
            }
            initCalled = true;

            // Setup raylib callback
            SetTraceLogCallback(
                [](int logLevel, const char* text, va_list args)
                {
                    logLevel = std::max(logLevel - 3, 0);
                    LogInternal(static_cast<LogLevel>(logLevel), "(unknown)", 0, text, args);
                });
            global::ENGINE_CONFIG.init();
#if MAGIQUE_INCLUDE_FONT == 1
            global::ENGINE_CONFIG.font = LoadFont_CascadiaCode();
#else
            global::ENGINE_CONFIG.font = GetFontDefault();
#endif
            global::SHADERS.init();      // Loads the shaders and buffers
            global::CONSOLE_DATA.init(); // Create default commands
            global::ENGINE_DATA.camera.zoom = 1.0F;
            InitJobSystem();
            LOG_INFO("Initialized magique %s", MAGIQUE_VERSION);
            return true;
        }
    } // namespace internal

    Game::Game(const char* name) : isRunning(true), gameName(name)
    {
        global::ENGINE_DATA.gameInstance = this; // Assign global game instance
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
        SetConfigFlags(FLAG_MSAA_4X_HINT);
        InitWindow(1280, 720, name);
        InitAudioDevice();
        SetTargetFPS(GetMonitorRefreshRate(0));
        SetExitKey(0);

        using namespace std;
        using namespace chrono;
        // Generate seed
        SetRandomSeed(random_device{}() ^ static_cast<unsigned int>(steady_clock::now().time_since_epoch().count()));

        // Setup magique
        internal::InitMagique();

        LOG_INFO("Working Directory: %s", GetWorkingDirectory());
        LOG_INFO("Initialized Game: %s", gameName);

#if !defined(MAGIQUE_DEBUG) && MAGIQUE_PROFILING == 1
        LOG_WARNING("Profiling enabled in Release mode. Disable for production build");
#endif
    }

    Game::~Game()
    {
#ifdef MAGIQUE_STEAM
        global::MP_DATA.close();
        global::STEAM_DATA.close();
#elif MAGIQUE_LAN
        global::MP_DATA.close();
#endif
        CloseAudioDevice();
        CloseWindow();
        LOG_INFO("Successfully shutdown magique");
    }

    int Game::run(const char* assetPath, const char* configPath, const uint64_t encryptionKey)
    {
        auto& loader = global::LOADER;
        loader = new AssetLoader{assetPath, encryptionKey};

        auto& config = global::ENGINE_DATA.gameConfig;
        config = GameConfig::LoadFromFile(configPath, encryptionKey);

        // Call startup
        onStartup(*static_cast<AssetLoader*>(loader));

        // Load atlas to gpu - needs to be the last task
        const auto loadAtlasGPU = [](AssetContainer&) { global::ATLAS_DATA.loadToGPU(); };
        static_cast<AssetLoader*>(loader)->registerTask(loadAtlasGPU, THREAD_MAIN, LOW, 1);
        static_cast<AssetLoader*>(loader)->printStats();
        isLoading = true;

        // Run main thread
        mainthread::Setup();
        mainthread::Run(*this);
        //
        // ----------- Game Runs --------------
        //
        onShutDown();
        mainthread::Close();

        GameConfig::SaveToFile(config, configPath, encryptionKey);

#if MAGIQUE_PROFILING == 1
        LOG_INFO("Average DrawTick: %dk nanos", (int)global::PERF_DATA.getAverageTime(DRAW) / 1'000);
        LOG_INFO("Average LogicTick: %dk nanos", (int)global::PERF_DATA.getAverageTime(UPDATE) / 1'000);
#endif
        return 0;
    }

    bool Game::getIsRunning() const { return isRunning; }

    bool Game::getIsLoading() const { return isLoading; }

    const char* Game::getName() const { return gameName; }

    void Game::shutDown() { isRunning = false; }
} // namespace magique