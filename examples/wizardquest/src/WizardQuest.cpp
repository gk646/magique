#include "WizardQuest.h"

#include <raylib/raylib.h>

#include <magique/multiplayer/LocalSockets.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/ui/LoadingScreen.h>
#include <magique/ecs/ECS.h>
#include <magique/persistence/container/GameSave.h>
#include <magique/core/Core.h>
#include <magique/gamedev/Achievements.h>

#include "Components.h"
#include "ui/UiScenes.h"
#include "loading/Loaders.h"

#include <magique/assets/AssetLoader.h>


PlayerHUD* hudd;

void WizardQuest::onStartup(AssetLoader& loader, GameConfig& config)
{
    loader.registerTask(new TileLoader(), BACKGROUND_THREAD);
    hudd = new PlayerHUD();
    InitLocalMultiplayer();
    RegisterEntity(PLAYER,
                   [](entt::entity e)
                   {
                       GiveActor(e);
                       GiveCamera(e);
                       GiveCollisionRect(e, 20, 30);
                       GiveComponent<PlayerStateC>(e);
                   });

    CreateEntity(PLAYER, 0, 0, MapID::LEVEL_1);


    auto save = GameSave::Load("MySave.save");

    AddAchievement("HeyFirst", []() { return true; });

    auto achData = save.getData<unsigned char>(StorageID::ACHIEVEMENTS);

    LoadAchievements(achData.getData(), achData.getSize());

    SetGameState(GameState::GAME);
}

void WizardQuest::drawGame(GameState gameState, Camera2D& camera) {}

void WizardQuest::drawUI(GameState gameState)
{
    switch (gameState)
    {
    case GameState::MAIN_MENU:
        break;
    case GameState::GAME:
        hudd->render();
        break;
    case GameState::GAME_OVER:
        break;
    }
}

void WizardQuest::updateGame(GameState gameState)
{
    auto& messages = ReceiveMessages();
    for (auto& msg : messages)
    {
        switch (msg.payload.type)
        {
        case MessageType::STRING:
            printf("Msg: %s", (const char*)msg.payload.data);
            break;
        }
    }

    if (IsKeyPressed(KEY_J))
    {
        if (IsHost())
        {
        }
    }
}

void WizardQuest::onShutDown()
{
    GameSave save;
    auto data = GetAchievementsData();
    save.saveData(StorageID::ACHIEVEMENTS, data.getData(), data.getSize());
    GameSave::Save(save, "MySave.save");
}