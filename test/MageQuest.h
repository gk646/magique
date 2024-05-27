#ifndef MAGEQUEST_H
#define MAGEQUEST_H


#include <magique/game/Game.h>


class MageQuest final : magique::Game {

  void onStartup(magique::GameLoader &gl) override;

};



#endif //MAGEQUEST_H