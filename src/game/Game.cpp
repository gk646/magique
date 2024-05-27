
#include <magique/game/Game.h>

#include "core/Core.h"


using namespace magique;


Game::Game(const char *name) : gameName(name)
{

}

Game::~Game() {}

void Game::init() {}

void Game::update() {}

void Game::render() {}

int Game::run() { return 0; }