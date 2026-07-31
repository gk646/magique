#include <cmath>
#include <magique/ui/Layouts.h>

namespace magique
{
    Layout::Layout(const Rect& area) : bounds(area) {}

    const Rect& Layout::area() const { return bounds; }

    Layout::operator Rect() const { return bounds; }

    Layout::operator Rectangle() const { return bounds; }

    VerticalSplit::VerticalSplit(const Rect& area, float where, float gap) : Layout(area), gap(gap), split(where) {}

    Layout VerticalSplit::left() const
    {
        auto ret = area();
        ret.width = ret.width * (split - gap / 2.0F);
        return ret.floor();
    }

    Layout VerticalSplit::right() const
    {
        auto ret = area();
        ret.x += ret.width * (split + gap / 2.0F);
        ret.width = area().width * (1.0F - split - gap / 2.0F);
        return ret.floor();
    }

    HorizontalSplit::HorizontalSplit(const Rect& area, float where, float gap) : Layout(area), gap(gap), split(where) {}

    Layout HorizontalSplit::upper() const
    {
        auto ret = area();
        ret.height = ret.height * (split - gap / 2.0F);
        return ret.floor();
    }

    Layout HorizontalSplit::lower() const
    {
        auto ret = area();
        ret.y += ret.height * (split + gap / 2.0F);
        ret.height = ret.height * (1.0F - split - gap / 2.0F);
        return ret.floor();
    }

    GridLayout::GridLayout(const Rect& area, Point itemSize, float gap) : Layout(area), itemSize(itemSize), gap(gap)
    {
        const auto gapW = area.width * gap;
        for (int i = 0; i < 500; i++)
        {
            if (i * itemSize.x + gapW + std::max(i - 1, 0) * gapW > area.width)
                break;
            perRow = i;
        }
    }

    GridLayout::GridLayout(const Rect& area, int itemsPerRow, int rows, float gap) :
        Layout(area), perRow(itemsPerRow), gap(gap)
    {
        const auto gapW = area.width * gap;
        itemSize.x = std::floor((area.width - gapW * (itemsPerRow - 1)) / itemsPerRow);
        itemSize.y = area.height / rows;
    }

    Layout GridLayout::item(int index) const
    {
        const auto gapW = area().width * gap;
        const auto itemW = itemSize.x + gapW;
        int row = index / perRow;
        int col = index % perRow;
        const auto gapSpace = std::max(col - 1, 0) * gapW;
        return Rect{area().pos() + Point{col * itemW + gapSpace, (float)row * itemSize.y}, itemSize};
    }

} // namespace magique
