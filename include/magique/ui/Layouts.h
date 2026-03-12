#ifndef MAGIQUE_LAYOUTS_H
#define MAGIQUE_LAYOUTS_H

#include <magique/core/Types.h>

//===============================================
// Layouts
//===============================================
// .....................................................................
// Layouts help you to easily organize and partition space
// Each layout does one simple step to partition space. By chaining them more, complex layouts can be achieved.
// This allows to procedurally describe layouts from top to bottom (as you start with the whole area).
//
// Example: (This creates a layout with 4 equally sized rectangles in the corners)
// Rect whole{0,0,10,10};
// VerticalSplit vertSplit{whole};
// HorizontalSplit leftHalf{vertSplit.left()};
// HorizontalSplit rightHalf{vertSplit.right()};
// .....................................................................

namespace magique
{

    struct Layout
    {
        Layout() = default;
        Layout(const Rect& area);

        const Rect& area() const;

        // Automatically converted to rect where needed
        operator Rect() const;
        operator Rectangle() const;

    private:
        Rect bounds{};
    };


    // Splits the given area into two pieces by making a vertical slice at the given width (in percent of the total width)
    // gap specifies the width of a gap in the middle that's kept free - for left side on the right, for right side on the left
    struct VSplit : Layout
    {
        VSplit(const Rect& area, float where = 0.5F, float gap = 0.0F);

        Layout left() const;
        Layout right() const;

    private:
        float gap = 0.0F;
        float split = 0.5F;
    };


    // Splits the given area into two pieces by making a horizontal slice at the given height (in percent of the total height)
    struct HSplit : Layout
    {
        HSplit(const Rect& area, float where = 0.5F);

        Layout upper() const;
        Layout lower() const;

        float split = 0.5F;
    };

    struct GridLayout : Layout
    {
        GridLayout(const Rect& area, Point itemSize, float gap = 2.0F);

        // Returns the bounds of the i-th item in the grid
        Layout item(int index) const;

    private:
        Point itemSize;
        float gap = 2.0F;
    };

} // namespace magique

#endif // MAGIQUE_LAYOUTS_H
