#ifndef UICONTROLS_H
#define UICONTROLS_H

#include <magique/magique.hpp>

struct PlayerHUD final : UIObject
{
    PlayerHUD() : UIObject({25, 50, 200, 50}) {}
    void onDraw(const Rect& bounds) override;
    void onUpdate(const Rect& bounds, bool isDrawn) override;
};

struct HotbarSlot final : UIObject
{
    inline static float size = 50;
    HotbarSlot() : UIObject({0, 0, size, size}, {}, {}, ScalingMode::KEEP_RATIO) {}
    void onDraw(const Rect& bounds) override;
};

struct PlayerHotbar final : UIContainer
{
    inline static int slots = 4;
    PlayerHotbar();
    void onDraw(const Rect& bounds) override;
};

#endif // UICONTROLS_H
