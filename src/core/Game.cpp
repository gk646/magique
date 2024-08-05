#include <raylib/raylib.h>
#include <raylib/rlgl.h>
#include <cxstructs/SmallVector.h>

#include <magique/core/Game.h>
#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>
#include <magique/ecs/Components.h>
#include <magique/ecs/Scripting.h>
#include <magique/ui/UI.h>
#include <magique/assets/AssetLoader.h>
#include <magique/util/Jobs.h>
#include <magique/persistence/container/GameConfig.h>

#include "external/raylib/src/external/glad.h"
#include "external/raylib/src/coredata.h"
#include "external/raylib/src/external/glfw/include/GLFW/glfw3.h"

#include "internal/globals/LogicTickData.h"
#include "internal/globals/Configuration.h"
#include "internal/globals/LoadingData.h"
#include "internal/globals/PerformanceData.h"
#include "internal/globals/ShaderData.h"
#include "internal/globals/TextureAtlas.h"
#include "internal/globals/AudioPlayer.h"
#include "internal/globals/ScriptData.h"
#include "internal/globals/UIData.h"
#include "internal/globals/CommandLineData.h"
#include "internal/globals/ParticleData.h"

#include "internal/headers/CollisionPrimitives.h"

#include "internal/systems/CollisionSystem.h"
#include "internal/systems/InputSystem.h"
#include "internal/systems/LogicSystem.h"
#include "internal/systems/LightingSystem.h"

#include "internal/headers/IncludeWindows.h"
#include "internal/headers/OSUtil.h"
#include "internal/globals/JobScheduler.h"

#include "core/headers/MainThreadUtil.h"
#include "core/headers/Updater.h"
#include "core/headers/Renderer.h"
#include "core/headers/MainThread.h"

CoreData CORE = {0};
rlglData RLGL = {0};

// Note: All includes are pulled out topside for clarity
// Here the whole render and update loops happen

namespace magique
{
    Game::Game(const char* name) : isRunning(true), gameName(name)
    {
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_MSAA_4X_HINT);
        // Init Window
        InitWindow(1280, 960, name);
        InitAudioDevice();
        SetExitKey(0);
        SetRandomSeed(rand() ^ std::chrono::steady_clock::now().time_since_epoch().count());
        InitMagique();
        LOG_INFO("Working Directory: %s", GetWorkingDirectory());
        LOG_INFO("Initialized Game: %s", gameName);
    }

    Game::~Game()
    {
        CloseAudioDevice();
        CloseWindow();
    }

    int Game::run(const char* assetPath, const char* configPath, const uint64_t encryptionKey)
    {
        auto& loader = global::LOADER;
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
        isLoading = true;

        // Run main thread
        mainthread::Setup();
        mainthread::Run(*this);
        onShutDown(config);
        mainthread::Close();

#ifdef MAGIQUE_DEBUG_PROFILE
        LOG_INFO("Average DrawTick: %dk nanos", (int)global::PERF_DATA.getAverageTime(DRAW) / 1'000);
        LOG_INFO("Average LogicTick: %dk nanos", (int)global::PERF_DATA.getAverageTime(UPDATE) / 1'000);
#endif
        return 0;
    }

    void Game::shutDown() { isRunning = false; }
} // namespace magique