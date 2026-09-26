#include "WizardQuest.h"

int main()
{
    AssetPackCompile("../res");
    WizardQuest game{};
    return game.run();
}