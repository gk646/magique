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
        Layout(const Rect& area) : area(area) {}

        const Rect& getArea() const { return area; }

        // Automatically converted to rect where needed
        operator Rect() const { return area; }
        operator Rectangle() const { return area; };

    private:
        Rect area{};
    };


    // Splits the given area into two pieces by making a vertical slice at the given width (in percent of the total width)
    struct VerticalSplit : Layout
    {
        VerticalSplit(const Rect& area, float where = 0.5F) : Layout(area), split(where) {};

        Layout left() const
        {
            auto ret = getArea();
            ret.width = ret.width * split;
            return ret;
        }

        Layout right() const
        {
            auto ret = getArea();
            ret.x += ret.width * split;
            ret.width = getArea().width * (1.0F - split);
            return ret;
        }

    private:
        float split = 0.5F;
    };

    // Splits the given area into two pieces by making a horizontal slice at the given height (in percent of the total height)
    struct HorizontalSplit : Layout
    {
        HorizontalSplit(const Rect& area, float where = 0.5F) : Layout(area), split(where) {};

        Layout upper() const
        {
            auto ret = getArea();
            ret.height = ret.height * split;
            return ret;
        }

        Layout lower() const
        {
            auto ret = getArea();
            ret.y += ret.height * split;
            ret.height = ret.height * (1.0F - split);
            return ret;
        }

    private:
        float split = 0.5F;
    };


} // namespace magique

#endif // MAGIQUE_LAYOUTS_H
