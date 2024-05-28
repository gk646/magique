#include <magique/game/Game.h>


#include <raylib.h>



namespace magique
{
    Game::Game(const char* name) : gameName(name)
    {
        InitWindow(1280,960, name);
        InitAudioDevice();
    }

    Game::~Game()
    {

    }



    int Game::run()
    {
        return 0;
    }

} // namespace magique