#include <magique/game/Game.h>


#include <raylib.h>



namespace magique
{
    Game::Game(const char* name) : gameName(name)
    {
        InitWindow(1280,960, name);
    }

    Game::~Game()
    {

    }

    void Game::init()
    {

    }

    void Game::update()
    {

    }

    void Game::render()
    {

    }

    int Game::run()
    {
        return 0;
    }

} // namespace magique