// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/LoadingScreen.h>
#include <magique/internal/Macros.h>

#include "internal/globals/EngineConfig.h"

namespace magique
{
    void LoadingScreenSet(LoadingScreen* loadingScreen)
    {
        MAGIQUE_ASSERT(loadingScreen != nullptr, "Passed nullptr");
        delete global::ENGINE_CONFIG.loadingScreen;
        global::ENGINE_CONFIG.loadingScreen = loadingScreen;
    }

    LoadingScreen* LoadingScreenGet()
    {
        return global::ENGINE_CONFIG.loadingScreen;
    }

    void LoadingScreen::DrawDefault(const bool /**/, const float progressPercent)
    {
        constexpr float beginX = 0.65f;
        constexpr float beginY = 0.90f;
        constexpr float width = 0.30f;
        constexpr float height = 0.020f;
        constexpr float verticalGap = 0.0020f;
        constexpr float horizontalGap = 0.0012f;

        const auto screenWidth = static_cast<float>(GetScreenWidth());
        const auto screenHeight = static_cast<float>(GetScreenHeight());

        DrawRectangleRec({0, 0, screenWidth, screenHeight}, LIGHTGRAY);

        const Rectangle outlineRec = {screenWidth * beginX, screenHeight * beginY, screenWidth * width,
                                      screenHeight * height};
        DrawRectangleLinesEx(outlineRec, 2, DARKGRAY);

        const auto x = outlineRec.x + screenWidth * horizontalGap;
        const auto y = outlineRec.y + screenHeight * verticalGap;
        const auto lWidth = screenWidth * (progressPercent / 100) * (width - horizontalGap * 2);
        const auto lHeight = screenHeight * (height - verticalGap * 2);
        const Rectangle loadingRect = {x, y, lWidth, lHeight};
        DrawRectangleRec(loadingRect, DARKGRAY);
    }

} // namespace magique