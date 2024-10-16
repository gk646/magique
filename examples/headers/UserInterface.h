#ifndef MAGIQUE_USER_INTERFACE_EXAMPLE_H
#define MAGIQUE_USER_INTERFACE_EXAMPLE_H
#define MAGIQUE_EXAMPLE

#include <magique/core/Game.h>
#include <magique/ui/controls/TextField.h>

//-----------------------------------------------
// User Interface Example
//-----------------------------------------------
// .....................................................................
// This example uses a multitude of different UI controls to showcase their functionality
// .....................................................................

using namespace magique;


struct Example final : Game
{
    TextField textField{50, 50, 150, 75};

    void onLoadingFinished() override
    {

    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        textField.draw();
    }
};


#endif //MAGIQUE_USER_INTERFACE_EXAMPLE_H