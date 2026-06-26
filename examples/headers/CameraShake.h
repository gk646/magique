#ifndef MAGIQUE_EXAMPLE_CAMERASHAKE_H
#define MAGIQUE_EXAMPLE_CAMERASHAKE_H

#include <magique/magique.hpp>

using namespace magique;

struct Example final : Game
{
    void onStartup(AssetLoader& loader) override { EngineShowCompass(); }

    void onDrawGame(GameState gameState, Camera2D& camera2D) override
    {
        BeginMode2D(camera2D);
        DrawCircle(0, 0, 50, RED); // Draw a circle
        EndMode2D();
    }

    void onUpdateGame(GameState gameState) override
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Point direction = {1, 0}; // Horizontal, to the right
            float maxDist = 5;        // Maximum distance of the shake in pixels from the origin (either direction)
            float velocity = 10;      // Velocity of the shake in pixels/s
            float decay = 10.0F;      // How fast the acceleration decays in pixels/s
            CameraShakeImpulse(direction, maxDist, velocity, decay);
            LOG_INFO("Camera shake");
        }
    }
};

#endif // MAGIQUE_EXAMPLE_CAMERASHAKE_H
