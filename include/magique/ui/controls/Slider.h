// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_SLIDER_H
#define MAGIQUE_SLIDER_H

#include <magique/ui/UIObject.h>
M_IGNORE_WARNING(4100)

//===============================================
// Slider
//===============================================
// .....................................................................
// The slider control
// Note: IF you override onUpdate() dont forget to call updateActions()
// .....................................................................

namespace magique
{
    struct Slider : UIObject
    {
        // Creates a new button from coordinates in the logical UI resolution
        Slider(float x, float y, float w, float h);
        Slider(float w, float h, Anchor anchor, float inset = 0.0F);

        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Called each tick on update thread
        void onUpdate(const Rectangle& bounds, bool isDrawn) override { updateActions(bounds); }

        // Sets the scale to be balanced between min and max - the middle will be the arithmetic mid between both values
        void setScaleBalanced(float min, float max);

        // Sets the scale to be imbalanced
        // On the left half of the slider interpolate between min-mid, on the right interpolate between mid-max
        void setScaleImblanced(float min, float mid, float max);

        // Gets the slider value according to the previously set scale
        [[nodiscard]] float getSliderValue() const;

        // Gets the percent value how close to slider is to the right from 0.0 - 1.0 - left = 0, right = 1.0
        [[nodiscard]] float getSliderPercent() const;

        // Sets the slider to the given position - from 0.0 - 1.0 - left = 0, right = 1.0
        void setSliderPercent(float value);

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

    private:
        Point getKnobPosition() const;

        constexpr static float radius = 15;
        float min = 0, mid = 0.5, max = 1;
        float sliderPos = 0.5;
        SliderMode mode = SliderMode::BALANCED;
        bool isHovered = false;
        bool isClicked = false;
        bool isDragged = false;
    };

} // namespace magique

M_UNIGNORE_WARNING()

#endif //MAGIQUE_SLIDER_H