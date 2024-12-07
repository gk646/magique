#include "WizardQuest.h"

#include <magique/assets/AssetPacker.h>

int main()
{
    CompileAssetImage("../res");
    WizardQuest game{};
    return game.run();
}