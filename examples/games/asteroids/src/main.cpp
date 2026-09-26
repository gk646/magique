#include "Asteroids.h"

int main()
{
    AssetPackCompile("../res");
    Asteroids game{};
    return game.run();
}
