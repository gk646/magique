// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_BUTTON_H
#define MAGIQUE_BUTTON_H

#include <magique/ui/UIObject.h>
#include <functional>
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
        Button(float x, float y, float w, float h, ScalingMode mode = ScalingMode::FULL);
        Button(float w, float h, Anchor anchor = Anchor::NONE, Point inset = {});

        // Sets a function that is called on click - same as onClick but without needing to override the button class
        void setOnClick(const ClickFunc& func);

        // Sets the disabled state - prevents only the onClick and wireOnClick activations
        // Does NOT affect the base UIObject methods
        void setDisabled(bool value);
        bool getIsDisabled() const;

        void setHoverText(const char* value);
        const char* getHoverText() const;

    protected:
        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Called each tick on update thread
        void onUpdate(const Rectangle& bounds, bool isDrawn) override
        {
            if (isDrawn)
                updateActions(bounds);
        }

        // Called once when the mouse position enters the button
        virtual void onHover(const Rectangle& bounds) {}

        // Called on click
        virtual void onClick(const Rectangle& bounds, int mouseButton) {}

        // Updates the action state and calls onHover() and onClick() if necessary
        // Note: You can conditionally NOT call this based on LayeredInput.isConsumed() to respect layers
        void updateActions(const Rectangle& bounds);

        // Draws a default graphical representation of this button
        virtual void drawDefault(const Rectangle& bounds);

        // Draws the hovered text at the mouse leftbound
        void drawHoverText(const Font& fnt, float size, Color back, Color outline, Color text) const;

    private:
        ClickFunc clickFunc = nullptr;
        std::string hoverText;
        bool isHovered = false;
        bool isDisabled = false;
    };

    struct TextButton : Button
    {
        TextButton(const char* text, ScalingMode mode = ScalingMode::FULL);

        // Fits the width and height to the text height
        void fitToText(const Font& font, float size);

        std::string& getText();
        const std::string& getText() const;

    protected:
        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        void drawDefault(const Rectangle& bounds) override;

    private:
        std::string text;
    };

} // namespace magique

M_UNIGNORE_WARNING()

#endif // MAGIQUE_BUTTON_H
