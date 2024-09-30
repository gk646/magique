#include "Asteroids.h"

#include <magique/assets/AssetPacker.h>

int main()
{
    // Compiles the asset images
    magique::CompileImage("../res");
    Asteroids game{};
    return game.run();
}