// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_BUTTON_H
#define MAGIQUE_BUTTON_H

#include <magique/ui/UIObject.h>
#include <magique/internal/PlatformIncludes.h>
#include <string>
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
    using ClickFunc = std::function<void(const Rectangle& bounds, int mouseButton)>;

    struct Button : UIObject
    {
        // Creates a new button from coordinates in the logical UI resolution
        Button(float x, float y, float w, float h);
        Button(float w, float h, Anchor anchor, Point inset = {});

        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Called each tick on update thread
        void onUpdate(const Rectangle& bounds, bool isDrawn) override { updateActions(bounds); }

        // Sets a function that is called on click - same as onClick but without needing to override the button class
        void wireOnClick(ClickFunc func);

        // Sets the disabled state - prevents only the onClick and wireOnClick activations
        // Does NOT affect the base UIObject methods
        void setDisabled(bool value);
        bool getIsDisabled() const;

        void setHoverText(const char* value);
        const char* getHoverText() const;

    protected:
        // Called once when the mouse position enters the button
        virtual void onHover(const Rectangle& bounds) {}

        // Called if the mouse is clicked inside the button bounds - can be called multiple times with different buttons
        virtual void onClick(const Rectangle& bounds, int button) {}

        // Updates the action state and calls onHover() and onClick() if necessary
        // Note: You can conditionally NOT call this based on LayeredInput.isConsumed() to respect layers
        void updateActions(const Rectangle& bounds);

        // Draws a default graphical representation of this button
        void drawDefault(const Rectangle& bounds) const;

        // Draws the hovered text at the mouse leftbound
        void drawHoverText(const Font& fnt, float size, Color back, Color outline, Color text) const;

    private:
        ClickFunc clickFunc = nullptr;
        std::string hoverText;
        bool isHovered = false;
        bool isDisabled = false;
    };
} // namespace magique

M_UNIGNORE_WARNING()

#endif //MAGIQUE_BUTTON_H
