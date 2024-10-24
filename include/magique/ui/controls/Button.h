#ifndef MAGIQUE_BUTTON_H
#define MAGIQUE_BUTTON_H

#include <magique/ui/UIObject.h>
IGNORE_WARNING(4100)

//-----------------------------------------------
// Button
//-----------------------------------------------
// .....................................................................
//
// .....................................................................

namespace magique
{
    struct Button : UIObject
    {
        // Creates a new button from coordinates in the logical UI resolution
        Button(float x, float y, float w, float h);

        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Called each tick on update thread
        void onUpdate(const Rectangle& bounds, bool isDrawn) override { updateButtonActions(bounds); }

    protected:
        // Called once when the mouse position enters the button
        virtual void onHover(const Rectangle& bounds) {}

        // Called if the mouse is clicked inside the button bounds - can be called multiple times with different buttons
        virtual void onClick(const Rectangle& bounds, int button) {}

        // Updates the action state and calls onHover() and onClick() if necessary
        // Note: You can conditionally NOT call this based on UIInput.isConsumed() to respect layers
        void updateButtonActions(const Rectangle& bounds);

        // Draws a default graphical representation of this button
        void drawDefault(const Rectangle& bounds) const;

    private:
        bool isHovered = false;
    };
} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_BUTTON_H