#ifndef UICONTROLS_H
#define UICONTROLS_H

#include <magique/ui/types/UIObject.h>

using namespace magique;

struct LobbyBrowser final : UIObject
{
    LobbyBrowser() : UIObject(0, 0, 1920, 1080) {}
};

#endif //UICONTROLS_H