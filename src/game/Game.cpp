#include <magique/game/Game.h>
#include <magique/assets/AssetPacker.h>
#include <magique/util/Logging.h>

#include <raylib.h>
#include <entt/entity/registry.hpp>

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
        SetTargetFPS(60);
    }


    Game::~Game()
    {
        CloseAudioDevice();
        CloseWindow();
    }


    int Game::run(const char* assetPath, const uint64_t encryptionKey)
    {
        CURRENT_GAME_LOADER = new GameLoader{assetPath, encryptionKey};
        onStartup(*CURRENT_GAME_LOADER);
        CURRENT_GAME_LOADER->printStats();

        _isLoading = true;

        updater::Run(_isRunning, *this);
        renderer::Run(_isLoading, *this);

        renderer::Close();
        updater::Close();


        return 0;
    }


} // namespace magique