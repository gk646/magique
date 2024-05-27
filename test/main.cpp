#define CATCH_CONFIG_MAIN


#include <magique/magique.hpp>


int main()
{
    magique::Game myGame;

    auto* myPtr = new int[5];

    delete [] myPtr;

    return myGame.run();
}