#include "WizardQuest.h"

#include <magique/assets/AssetPacker.h>

int main()
{
    magique::CompileImage("../res");
    WizardQuest game{};
    return game.run();
}