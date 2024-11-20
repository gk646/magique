#ifndef MAGIQUE_UI_H
#define MAGIQUE_UI_H

#include <magique/core/Types.h>

//===============================================
// UI Module
//===============================================
// .....................................................................
// All UI modules in magique uses a fixed logical resolution of 1920x1080. This means that you can define all dimensions,
// offsets and gaps in absolute numbers. These values are then automatically scaled to fit to the current resolution.
// To fit different screen ratios (21:9, 4:3) you can use anchor points.
// The intended workflow is to completely work in 1920x1080 and specify all measurements in that resolution.
// This makes it easy to follow designs and reason about distances while the engine handles scaling automatically.
// .....................................................................

namespace magique
{
    // T = Top / M = Middle / B = Bottom
    // L = Left / C = Center / R = Right
    // =========================
    // |(TL)      (TC)      (TR)|
    // |                        |
    // |(ML)      (MC)      (MR)|
    // |                        |
    // |(BL)      (BC)      (BR)|
    // =========================
    // Returns the (top-left) coordinates of the specified anchor point
    // Passing width and height aligns the anchor to fit them - inset applies an inset for the border points inwards
    Point GetUIAnchor(Anchor anchor, float width = 0, float height = 0, float inset = 0);

    // Accepts a value in the logical resolution and returns the value in the current resolution (scaled horizontally)
    // Note: This is useful when needing static offsets that automatically scale
    float GetScaled(float val);

    // Returns the current horizontal and vertical scaling
    Point GetUIScaling();

    // Returns the position where the left mouse button was first pressed - resets when the button is released
    // Failure: returns {-1,-1} if the mouse was not yet pressed or was released
    Point GetDragStartPosition();

    // Same as raylib's but returns a magique::Point
    Point GetMousePos();

    // Getters for input that allows for consumption - when consumed all methods return false
    // The consumed state is automatically reset at the beginning of each tick
    // Note: This is very useful when dealing with layered UI to prevent unwanted input propagation
    // Note: You can still use the raylib inputs methods if you need exceptions to this rule!
    struct UIInput final
    {
        // Returns true only if input state AND not consumed
        static bool IsKeyPressed(int key);
        static bool IsKeyDown(int key);
        static bool IsKeyReleased(int key);

        static bool IsMouseButtonPressed(int key);
        static bool IsMouseButtonDown(int key);

        // Consume the input for this tick - all input methods after this will return false
        static void Consume();
        static bool IsConsumed();
    };

} // namespace magique

#endif //MAGIQUE_UI_H