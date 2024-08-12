#ifndef MAGIQUE_BUTTON_H
#define MAGIQUE_BUTTON_H

#include <raylib/raylib.h>
#include <magique/ui/types/UIObject.h>

//-----------------------------------------------
// Button
//-----------------------------------------------
// .....................................................................
// The draw functions can be overridden so the visuals can be modified completly
// Defaults implementations are provided for guidance
// .....................................................................

namespace magique
{
    struct Button : UIObject
    {
        // Creates a new button from coordinates in the logical UI resolution
        Button(GameState gameState, float x, float y, float w, float h, UILayer layer = UILayer::MEDIUM);

        // Called each tick on draw thread
        void draw() override;

        // Called each tick on update thread
        void update() override;

        // Called everytime the button is hovered if it wasnt before
        virtual void onHover(const Rectangle& bounds) {}

        // Called everytime the button is clicked with either right or left click
        virtual void onClick(const Rectangle& bounds) {}

        //----------------- DRAW -----------------//
        // Override them to achieve custom visuals

        virtual void drawIdle(const Rectangle& bounds)
        {
            DrawRectangleRounded(bounds, 0.2F, 30, LIGHTGRAY);
            DrawRectangleRoundedLinesEx(bounds, 0.2F, 30, 2, GRAY);
        }

        virtual void drawHovered(const Rectangle& bounds)
        {
            DrawRectangleRounded(bounds, 0.2F, 30, GRAY);
            DrawRectangleRoundedLinesEx(bounds, 0.2F, 30, 2, DARKGRAY);
        }

        virtual void drawClicked(const Rectangle& bounds)
        {
            DrawRectangleRounded(bounds, 0.2F, 30, DARKGRAY);
            DrawRectangleRoundedLinesEx(bounds, 0.2F, 30, 2, GRAY);
        }

    private:
        bool wasHovered = false;
    };
} // namespace magique

#endif //MAGIQUE_BUTTON_H