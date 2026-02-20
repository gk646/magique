#ifndef MAGIQUE_USER_INTERFACE_EXAMPLE_H
#define MAGIQUE_USER_INTERFACE_EXAMPLE_H

#include <magique/core/Game.h>
#include <magique/ui/controls/Button.h>
#include <magique/ui/controls/TextField.h>
#include <magique/ui/controls/Window.h>
#include <magique/ui/controls/ScrollPane.h>
#include "magique/ui/controls/ListMenu.h"
#include <magique/util/Logging.h>

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

    void onDraw(const Rect& bounds) override
    {
        drawDefault(bounds);
        textField1.align(Anchor::TOP_CENTER, *this, 50);
        textField1.draw();
    }

    void onUpdate(const Rectangle& bounds, bool wasDrawn) override { textField1.fitBoundsToText(14); }
};

struct Example final : Game
{
    TextField textField{50, 150, 150, 75};
    TextFieldWindow window;
    Button button{50, 50, 50, 50};
    ScrollPane scrollPane{350, 350, 250, 240};
    ListMenu listMenu{200, 500, 100, 50};

    void onLoadingFinished() override
    {
        TextField* field = new TextField(150, 50);
        field->setText("This text field scales to contain its inputs");
        scrollPane.setContent(field);

        button.wireOnClick([](const Rectangle& bounds, int mouseButton) { LOG_INFO("Button clicked!\n"); });

        listMenu.wireOnSelect([](const std::string& txt) { LOG_INFO("Clicked list entry: %s", txt.c_str()); });
        listMenu.add("Entry One");
        listMenu.add("This is second!");
        listMenu.add("Iam the last :(");
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        textField.draw();
        window.draw();
        button.draw();
        scrollPane.draw();
        listMenu.draw();
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
