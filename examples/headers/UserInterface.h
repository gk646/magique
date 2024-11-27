#ifndef MAGIQUE_USER_INTERFACE_EXAMPLE_H
#define MAGIQUE_USER_INTERFACE_EXAMPLE_H
#define MAGIQUE_EXAMPLE

#include <magique/core/Game.h>
#include <magique/ui/controls/Button.h>
#include <magique/ui/controls/TextField.h>
#include <magique/ui/controls/Window.h>

//-----------------------------------------------
// User Interface Example
//-----------------------------------------------
// .....................................................................
// This example uses a multitude of different UI controls to showcase their functionality
// .....................................................................

using namespace magique;

struct TextFieldWindow final : Window
{
    TextField textField1{50, 50, 150, 75};

    TextFieldWindow() : Window(150, 150, 250, 250) {}

    void onDraw(const Rectangle& bounds) override
    {
        drawDefault(bounds);
        textField1.align(Anchor::TOP_CENTER, *this, 50);
        textField1.draw();
    }
};


struct PrintButton final : Button
{
    PrintButton() : Button(50, 150, 75, 75) {}

    void onClick(const Rectangle& bounds, int button) override
    {
        printf("Button clicked!\n");
    }
};

struct Example final : Game
{
    TextField textField{50, 50, 150, 75};
    TextFieldWindow window;
    PrintButton button;

    void onLoadingFinished() override {}

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        textField.draw();
        window.draw();
        button.draw();
    }
};


#endif //MAGIQUE_USER_INTERFACE_EXAMPLE_H