#define CATCH_CONFIG_MAIN

#include "MageQuest.h"
#include <magique/assets/AssetPacker.h>

int main()
{
    util::SetLogLevel(util::LEVEL_INFO);
    assets::CompileImage("../res");
    MageQuest myGame;
    return myGame.run();
}