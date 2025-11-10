#ifndef MAGIQUE_USER_INTERFACE_EXAMPLE_H
#define MAGIQUE_USER_INTERFACE_EXAMPLE_H

#include <magique/core/Game.h>
#include <magique/ui/controls/Button.h>
#include <magique/ui/controls/TextField.h>
#include <magique/ui/controls/Window.h>
#include <magique/ui/controls/ScrollPane.h>

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

    void onUpdate(const Rectangle& bounds, bool wasDrawn) override
    {
        textField1.fitBoundsToText(14);
    }
};


struct Example final : Game
{
    TextField textField{50, 150, 150, 75};
    TextFieldWindow window;
    Button button{50,50,50,50};

    ScrollPane scrollPane{350, 350, 250, 240};

    void onLoadingFinished() override
    {
        TextField* field = new TextField(150, 50);
        field->setText("This text field scales to contain its inputs");
        scrollPane.setContent(field);

        button.wireOnClick([](const Rectangle& bounds, int mouseButton) { printf("Button clicked!\n"); });
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        textField.draw();
        window.draw();
        button.draw();
        scrollPane.draw();
    }

    void updateGame(GameState gameState) override
    {
        if (scrollPane.getContent())
        {

            scrollPane.getContent()->getAs<TextField>()->fitBoundsToText();
        }
    }
};

#endif //MAGIQUE_USER_INTERFACE_EXAMPLE_H
