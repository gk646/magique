// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UIContainer.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{

    UIContainer::UIContainer(Rect bounds, Anchor anchor, Point inset, ScalingMode mode) :
        UIObject(bounds, anchor, inset, mode)
    {
    }

    UIObject* UIContainer::addChild(UIObject* child, std::string_view name)
    {
        auto compFunc = [&](auto& e)
        {
            return e.object == child || (!name.empty() && e.name == name);
        };
        if (std::ranges::find_if(children, compFunc) != children.end())
        {
            return nullptr;
        }
        children.push_back({std::string{!name.empty() ? name : ""}, child});
        return child;
    }

    void UIContainer::addChildren(const std::initializer_list<UIObject*>& newChildren)
    {
        for (auto e : newChildren)
        {
            addChild(e);
        }
    }

    void UIContainer::addChildren(std::span<UIObject*> newChildren)
    {
        for (auto e : newChildren)
        {
            addChild(e);
        }
    }

    bool UIContainer::removeChild(std::string_view name)
    {
        MAGIQUE_ASSERT(!name.empty(), "Name must be non-null");
        return std::erase_if(children, [name](auto& e) { return e.name == name; }) > 0;
    }

    bool UIContainer::removeChild(const int index)
    {
        MAGIQUE_ASSERT(index >= 0, "index cannot be negative");
        if (index >= static_cast<int>(children.size()))
        {
            LOG_WARNING("Child with index %d is out of bounds", index);
            return false;
        }
        children.erase(children.begin() + index);
        return true;
    }

    bool UIContainer::removeChild(UIObject* child)
    {
        return std::erase_if(children, [&](auto& e) { return e.object == child; }) > 0;
    }

    UIObject* UIContainer::getChild(std::string_view name) const
    {
        MAGIQUE_ASSERT(!name.empty(), "Name must be non-null");
        auto it = std::ranges::find_if(children, [&](auto& e) { return e.name == name; });
        if (it != children.end())
        {
            return it->object;
        }
        return nullptr;
    }

    UIObject* UIContainer::getChild(const int index) const
    {
        if (index >= static_cast<int>(children.size()))
        {
            LOG_WARNING("Child with index %d is out of bounds", index);
            return nullptr;
        }
        return children[index].object;
    }

    const std::vector<ContainerChild>& UIContainer::getChildren() const { return children; }

    void VerticalContainer::onDraw(const Rect& bounds)
    {
        float height = gap;
        for (auto [name, ptr] : getChildren())
        {
            ptr->align(anchor, *this, {0, height});
            if (!reverseDraw)
                ptr->draw();
            height += ptr->getBounds().height + gap;
        }

        if (reverseDraw)
        {
            for (auto it = getChildren().rbegin(); it != getChildren().rend(); ++it)
            {
                it->object->draw();
            }
        }

        const auto start = getStartBounds();
        setSize({std::max(bounds.width, start.width), std::max(height, start.height)});
    }

    void VerticalContainer::setHorizontalAlign(Anchor align)
    {
        switch (align)
        {
        case Anchor::NONE:
            break;
        case Anchor::TOP_LEFT:
        case Anchor::MID_LEFT:
        case Anchor::BOTTOM_LEFT:
            anchor = Anchor::TOP_LEFT;
            break;
        case Anchor::TOP_CENTER:
        case Anchor::MID_CENTER:
        case Anchor::BOTTOM_CENTER:
            anchor = Anchor::TOP_CENTER;
            break;
        case Anchor::TOP_RIGHT:
        case Anchor::MID_RIGHT:
        case Anchor::BOTTOM_RIGHT:
            anchor = Anchor::TOP_RIGHT;
            break;
        }
    }

    Anchor VerticalContainer::getHorizontalAlign() const { return anchor; }

    float VerticalContainer::getGap() const { return gap; }

    void VerticalContainer::setGap(float newGap) { gap = newGap; }

    void VerticalContainer::setReverseDraw(bool reverse) { reverseDraw = reverse; }


} // namespace magique
