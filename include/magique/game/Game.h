#ifndef APPLICATION_H
#define APPLICATION_H

#include "magique/loading/GameLoader.h"

//-----------------------------------------------
// Game module
//-----------------------------------------------

// .....................................................................
// Core game class you should subclass
// .....................................................................

namespace magique
{
    struct Game
    {
        const char* const gameName;
        explicit Game(const char* name = "MyGame");
        virtual ~Game();

        virtual void onStartup(GameLoader& gl) {}
        virtual void onUpdateTick(){}

        virtual void onRenderTick(){}
        virtual void onUIRenderTick(){}

        int run();
    };

} // namespace magique


#endif // APPLICATION_H