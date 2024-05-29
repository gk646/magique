#include <magique/game/Game.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/assets/AssetPacker.h>
#include <magique/util/Logging.h>

#include <raylib.h>
#include <entt/entity/registry.hpp>

using UpdateMethod = void (*)(entt::registry& registry);

#include "Renderer.h"
#include "Updater.h"

namespace magique
{

    Game::Game(const char* name) : gameName(name)
    {
        isRunning = true;
#ifdef MAGIQUE_DEBUG
        SetTraceLogLevel(LOG_WARNING);
#endif
        SetTraceLogLevel(LOG_WARNING);
        InitWindow(1280, 720, name);
        InitAudioDevice();
        SetExitKey(0);
        SetRandomSeed(rand() ^ std::chrono::steady_clock::now().time_since_epoch().count());
        LOG_INFO("Started Game");
    }


    Game::~Game()
    {
        CloseAudioDevice();
        CloseWindow();
    }


    int Game::run(const char* assetPath, const uint64_t encryptionKey)
    {
        AssetContainer assets;
        assets::LoadAssetImage(assetPath, assets, encryptionKey);

        updater::Run(isRunning, *this);
        renderer::Run(isRunning, *this);

        renderer::Close();
        updater::Close();

        return 0;
    }

} // namespace magique