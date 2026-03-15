#include <cmath>
#include <magique/ui/Layouts.h>

namespace magique
{
    Layout::Layout(const Rect& area) : bounds(area) {}

    const Rect& Layout::area() const { return bounds; }

    Layout::operator Rect() const { return bounds; }

    Layout::operator Rectangle() const { return bounds; }

    VSplit::VSplit(const Rect& area, float where, float gap) : Layout(area), gap(gap), split(where) {}

    Layout VSplit::left() const
    {
        auto ret = area();
        ret.width = ret.width * (split - gap / 2.0F);
        return ret.floor();
    }

    Layout VSplit::right() const
    {
        auto ret = area();
        ret.x += ret.width * (split + gap / 2.0F);
        ret.width = area().width * (1.0F - split - gap / 2.0F);
        return ret.floor();
    }

    HSplit::HSplit(const Rect& area, float where) : Layout(area), split(where) {}

    Layout HSplit::upper() const
    {
        auto ret = area();
        ret.height = ret.height * split;
        return ret.floor();
    }

    Layout HSplit::lower() const
    {
        auto ret = area();
        ret.y += ret.height * split;
        ret.height = ret.height * (1.0F - split);
        return ret.floor();
    }

    GridLayout::GridLayout(const Rect& area, Point itemSize, float gap) : Layout(area), itemSize(itemSize), gap(gap) {}

    Layout GridLayout::item(int index) const
    {
        Point pos = gap;
        int perRow = (int)std::floor((area().x - gap) / itemSize.x);
        int row = index / perRow;
        int col = index % perRow;
        return Rect{pos + Point{(float)col * itemSize.x, (float)row * itemSize.y}, itemSize};
    }


} // namespace magique
