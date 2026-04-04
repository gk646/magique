#include "Asteroids.h"

#include <magique/assets/AssetPacker.h>

int main()
{
    // Compiles the asset images
    AssetPackCompile("../res");
    Asteroids game{};
    return game.run();
}
