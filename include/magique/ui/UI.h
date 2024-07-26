#ifndef MAGIQUE_UI_H
#define MAGIQUE_UI_H

#include <magique/core/Types.h>

//-----------------------------------------------
// UI Module
//-----------------------------------------------
// .....................................................................
// The UI module in magique uses a fixed logical resolution of 1920x1080. This means that you can define all dimensions,
// offsets and gaps in absolute numbers. These values are then automatically scaled to fit to the current resolution.
// To fit all screen rations (16:9, 4:3) you can use anchor points.
// The intended workflow is to completely work in 1920x1080 and specify all measurements in that resolution.
// This makes it easy to follow designs or reason about distances while the engine handles scaling automatically,
// .....................................................................

namespace magique
{
    // L = Left / C = Center / R = Right
    // T = Top / M = Middle / B = Bottom
    // -------------------------
    // |(LT)      (CT)      (RT)|
    // |                        |
    // |(LM)      (CM)      (RM)|
    // |                        |
    // |(LB)      (CB)      (RB)|
    // -------------------------
    // Returns the (top-left) coordinates of the specified anchor point
    // Passing width and height aligns the coordinates to fit them and inset applies a inset for the border points inwards
    Point GetUIAnchor(AnchorPosition anchor, float width = 0, float height = 0, float inset = 0);

    // Returns a reference to the global ui-root
    UIRoot& GetUIRoot();

    // Returns the current scale factor for x and y dimensions
    Point GetUIScaling();

} // namespace magique

#endif //MAGIQUE_UI_H