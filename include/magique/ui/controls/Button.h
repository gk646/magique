#ifndef MAGIQUE_BUTTON_H
#define MAGIQUE_BUTTON_H

#include <raylib/raylib.h>
#include <magique/ui/UIObject.h>
IGNORE_WARNING(4100)

//-----------------------------------------------
// Button
//-----------------------------------------------
// .....................................................................
// The draw functions can be overridden so the visuals can be modified completely
// Defaults implementations are provided for guidance
// .....................................................................

namespace magique
{
    struct Button : UIObject
    {
        // Creates a new button from coordinates in the logical UI resolution
        Button(float x, float y, float w, float h);

        // Called each tick on update thread
        void onUpdate(const Rectangle& bounds, bool isDrawn) override;

        // Called everytime the button is hovered if it wasn't before
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

UNIGNORE_WARNING()

#endif //MAGIQUE_BUTTON_H