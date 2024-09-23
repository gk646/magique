#ifndef UICONTROLS_H
#define UICONTROLS_H

#include <magique/ui/types/UIContainer.h>
#include <magique/ui/UIObject.h>

using namespace magique;

struct PlayerHUD final : UIObject
{
    PlayerHUD() : UIObject(25, 50, 200, 50) {}
    void draw(const Rectangle& bounds) override;
    void update(const Rectangle& bounds, bool isDrawn) override;
};

struct MiniWindowMenu final : UIObject
{

};

struct LobbyBrowser final : UIContainer
{

    LobbyBrowser() : UIContainer(0, 0, 1920, 1080) {}
    void draw(const Rectangle &bounds) override;
    void update(const Rectangle &bounds, bool isDrawn) override;
};

struct LobbyWindow final : UIContainer
{
};
#endif //UICONTROLS_H