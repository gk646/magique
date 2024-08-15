#include "WizardQuest.h"

#include <magique/multiplayer/LocalSockets.h>
#include <magique/multiplayer/Multiplayer.h>
#include <magique/ui/LoadingScreen.h>
#include <magique/ui/UI.h>
#include <raylib/raylib.h>

#include "Components.h"
#include "ui/UiScenes.h"

void WizardQuest::onStartup(AssetLoader& loader, GameConfig& config) { InitLocalMultiplayer(); }

void WizardQuest::drawGame(GameState gameState, Camera2D& camera) {}

PlayerHUD hudd = PlayerHUD();

void WizardQuest::drawUI(GameState gameState) { hudd.render(); }


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
        if(IsHost())
        {

        }
    }
}