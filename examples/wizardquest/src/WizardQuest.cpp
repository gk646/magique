#include "WizardQuest.h"

#include <magique/multiplayer/LocalSockets.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/ui/LoadingScreen.h>
#include <magique/ui/UI.h>
#include <raylib/raylib.h>

#include "Components.h"
#include "ui/UiScenes.h"

void WizardQuest::onStartup(AssetLoader& loader, GameConfig& config)
{
    InitLocalMultiplayer();

    GameHUD* hud = new GameHUD();

    RegisterScene(hud,"GameHUD");

}

void WizardQuest::drawGame(GameState gameState, Camera2D& camera)
{

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
    }
}