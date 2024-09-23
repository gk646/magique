#ifndef MAGIQUE_WINDOW_H
#define MAGIQUE_WINDOW_H

#include <raylib/raylib.h>
#include <magique/ui/types/UIContainer.h>

//-----------------------------------------------
// Windows
//-----------------------------------------------
// .....................................................................
// Generic window class which can be subclasses to achieve custom visuals
// .....................................................................

namespace magique
{
    struct Window : UIContainer
    {
        // Creates a new button from coordinates in the logical UI resolution
        // If not specified the mover is 10% of the total height
        Window(float x, float y, float w, float h, float moverHeight = 0.0F);

        // Draws the control - called each tick on draw thread
        void draw(const Rectangle& bounds) override;

        // Updates the control - called each tick on update thread
        void update(const Rectangle& bounds, bool isDrawn) override;

        //----------------- DRAW -----------------//
        // Override them to achieve custom visuals

        // Draws the moveable bar of the window
        // Gets passed the bounds of the mover and if its hovered or not
        virtual void drawMover(const Rectangle bounds, const bool isHovered)
        {
            if (isHovered)
            {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) // Pressed
                {
                    DrawRectangleRounded(bounds, 0.2F, 30, DARKGRAY);
                    DrawRectangleRoundedLinesEx(bounds, 0.2F, 30, 2, GRAY);
                }
                else // Hovered
                {
                    DrawRectangleRounded(bounds, 0.2F, 30, GRAY);
                    DrawRectangleRoundedLinesEx(bounds, 0.2F, 30, 2, DARKGRAY);
                }
            }
            else // Idle
            {
                DrawRectangleRounded(bounds, 0.2F, 30, LIGHTGRAY);
                DrawRectangleRoundedLinesEx(bounds, 0.2F, 30, 2, GRAY);
            }
        }

        // Draws the body of the window - is called before drawMover()
        // Gets passed the bounds of the window
        virtual void drawBody(const Rectangle bounds)
        {
            DrawRectangleRounded(bounds, 0.2F, 30, LIGHTGRAY);
            DrawRectangleRoundedLinesEx(bounds, 0.2F, 30, 2, GRAY);
        }

    private:
        float moverHeightP = 10.0F;
    };

} // namespace magique

#endif //MAGIQUE_WINDOW_H