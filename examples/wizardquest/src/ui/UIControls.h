#ifndef UICONTROLS_H
#define UICONTROLS_H

#include <magique/ui/types/UIContainer.h>
#include <magique/ui/types/UIObject.h>

using namespace magique;

struct PlayerHUD final : UIObject
{
    void draw(const Rectangle &bounds) override;
};

struct LobbyBrowser final : UIObject
{
    LobbyBrowser() : UIObject(0, 0, 1920, 1080) {}
};


struct LobbyWindow  final : UIContainer
{
    void draw(const Rectangle &bounds) override
    {

    }
};
#endif //UICONTROLS_H