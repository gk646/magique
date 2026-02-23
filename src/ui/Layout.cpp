#include <magique/ui/Layouts.h>

namespace magique
{
    Layout::Layout(const Rect& area) : area(area) {}

    const Rect& Layout::getArea() const { return area; }

    Layout::operator Rect() const { return area; }

    Layout::operator Rectangle() const { return area; }

    VerticalSplit::VerticalSplit(const Rect& area, float where, float gap) : Layout(area), gap(gap), split(where) {}

    Layout VerticalSplit::left() const
    {
        auto ret = getArea();
        ret.width = ret.width * (split - gap / 2.0F);
        return ret;
    }

    Layout VerticalSplit::right() const
    {
        auto ret = getArea();
        ret.x += ret.width * (split + gap / 2.0F);
        ret.width = getArea().width * (1.0F - split - gap / 2.0F);
        return ret;
    }

    HorizontalSplit::HorizontalSplit(const Rect& area, float where) : Layout(area), split(where) {}

    Layout HorizontalSplit::upper() const
    {
        auto ret = getArea();
        ret.height = ret.height * split;
        return ret;
    }

    Layout HorizontalSplit::lower() const
    {
        auto ret = getArea();
        ret.y += ret.height * split;
        ret.height = ret.height * (1.0F - split);
        return ret;
    }
} // namespace magique
