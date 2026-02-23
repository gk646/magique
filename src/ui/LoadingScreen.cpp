// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/LoadingScreen.h>
#include <magique/util/RayUtils.h>

#include "internal/globals/EngineConfig.h"
#include "magique/ui/UI.h"

namespace magique
{
    void LoadingScreenSet(LoadingScreen* loadingScreen)
    {
        MAGIQUE_ASSERT(loadingScreen != nullptr, "Passed nullptr");
        delete global::ENGINE_CONFIG.loadingScreen;
        global::ENGINE_CONFIG.loadingScreen = loadingScreen;
    }

    LoadingScreen* LoadingScreenGet() { return global::ENGINE_CONFIG.loadingScreen; }

    void LoadingScreen::DrawDefault(const bool /**/, const float progress)
    {
        const Point dims = Point{0.4F, 0.02F} * GetScreenDims();
        auto anchor = UIGetAnchor(Anchor::BOTTOM_RIGHT, dims, {20, 20});
        const auto outside = Rect{anchor, dims};
        DrawRectFrameFilled(outside, LIGHTGRAY, GRAY);
        const auto inside = Rect::Filled(outside.shrink(2), progress/100.0F, Direction::RIGHT);
        DrawRectFrameFilled(inside, DARKGRAY, GRAY);
    }

} // namespace magique
