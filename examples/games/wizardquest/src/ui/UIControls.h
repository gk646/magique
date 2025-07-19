#ifndef UICONTROLS_H
#define UICONTROLS_H

#include <magique/ui/UIContainer.h>
#include <magique/ui/UIObject.h>
#include <magique/ui/controls/Window.h>

using namespace magique;

struct PlayerHUD final : UIObject
{
    PlayerHUD() : UIObject(25, 50, 200, 50) {}
    void onDraw(const Rectangle& bounds) override;
    void onUpdate(const Rectangle& bounds, bool isDrawn) override;
};

struct HotbarSlot final : UIObject
{
    inline static float size = 50;
    HotbarSlot() : UIObject(0, 0, size, size, ScalingMode::KEEP_RATIO) {}
    void onDraw(const Rectangle& bounds) override;
};

struct PlayerHotbar final : UIContainer
{
    inline static int slots = 4;
    PlayerHotbar();
    void onDraw(const Rectangle& bounds) override;
};

struct MiniWindowButtons final : UIObject
{
};

struct LobbyBrowser final : Window
{
    LobbyBrowser() : Window(0, 0, 500, 600) {}
};

struct LobbyWindow final : UIContainer
{
};
#endif //UICONTROLS_H