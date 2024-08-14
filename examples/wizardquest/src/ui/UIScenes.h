#ifndef UISCENES_H
#define UISCENES_H

#include <magique/ui/UIScene.h>

#include "ui/UIControls.h"

using namespace magique;

struct GameHUD final :  UIScene
{
    LobbyBrowser* browser;
    GameHUD()
    {
        browser = new LobbyBrowser();
    }
};


#endif //UISCENES_H