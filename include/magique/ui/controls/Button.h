// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_BUTTON_H
#define MAGIQUE_BUTTON_H

#include <magique/ui/UIObject.h>
M_IGNORE_WARNING(4100)

//===============================================
// Button
//===============================================
// .....................................................................
// The button control
// Note: IF you override onUpdate() dont forget to call updateActions()
// .....................................................................

namespace magique
{
    struct Button : UIObject
    {
        // Creates a new button from coordinates in the logical UI resolution
        Button(float x, float y, float w, float h);
        Button(float w, float h, Anchor anchor, float inset = 0.0F);

        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Called each tick on update thread
        void onUpdate(const Rectangle& bounds, bool isDrawn) override { updateActions(bounds); }

        // Sets a function that is called on click - same as onClick but without needing to override the button class
        void wireOnClick(ClickFunc func);

    protected:
        // Called once when the mouse position enters the button
        virtual void onHover(const Rectangle& bounds) {}

        // Called if the mouse is clicked inside the button bounds - can be called multiple times with different buttons
        virtual void onClick(const Rectangle& bounds, int button) {}

        // Updates the action state and calls onHover() and onClick() if necessary
        // Note: You can conditionally NOT call this based on UIInput.isConsumed() to respect layers
        void updateActions(const Rectangle& bounds);

        // Draws a default graphical representation of this button
        void drawDefault(const Rectangle& bounds) const;

    private:
        bool isHovered = false;
        ClickFunc clickFunc = nullptr;
    };
} // namespace magique

M_UNIGNORE_WARNING()

#endif //MAGIQUE_BUTTON_H