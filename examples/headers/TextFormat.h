#ifndef MAGIQUE_EXAMPLE_TEXT_FORMAT_H
#define MAGIQUE_EXAMPLE_TEXT_FORMAT_H

#include <magique/core/Core.h>
#include <magique/core/Game.h>
#include <magique/ui/TextFormat.h>

// Recommended and supported
using namespace magique;

// Subclass magique::Game to implement custom logic
struct Example final : Game
{

    // Override onStartup() to trigger custom logic at the beginning
    void onStartup(AssetLoader& loader) override
    {
        SetFormatValue("PLAYER_NAME", "Geralt");
        SetFormatValue("PLAYER_AGE", 97);
    }

    // Override to draw custom content each game tick
    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        auto& font = GetFont();
        ClearBackground(BLACK); // Clear black so white text is visible

        DrawTextFmt(font, "My name is: ${PLAYER_NAME}", {150, 150}, 25);
        DrawTextFmt(font, "I am ${PLAYER_AGE} years old!", {150, 170}, 25);
    }
};


#endif // MAGIQUE_EXAMPLE_TEXT_FORMAT_H