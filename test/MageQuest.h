#ifndef MAGEQUEST_H
#define MAGEQUEST_H

#include <magique/game/Game.h>


class MageQuest final : public magique::Game
{
    Shader shader{};
    Shader shadow{};
    void onStartup(magique::GameLoader& gl) override
    {
        gl.registerTask([](magique::AssetContainer&) { printf("hey"); }, MAIN_THREAD);
    }
    void drawGame(entt::registry& registry, Camera2D& camera) override { DrawRectangle(0, 0, 50, 50, BLUE); }
};


#endif //MAGEQUEST_H