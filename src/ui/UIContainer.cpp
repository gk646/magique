// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UIContainer.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{

    UIContainer::UIContainer(Rect bounds, Anchor anchor, Point inset, ScalingMode mode) :
        UIObject(bounds, anchor, inset, mode)
    {
        global::UI_DATA.registerObject(this, true);
    }

    UIObject* UIContainer::addChild(UIObject* child, const char* name)
    {
        auto compFunc = [&](auto& e)
        {
            return e.object == child || (name != nullptr && e.name == name);
        };
        if (std::ranges::find_if(children, compFunc) != children.end())
        {
            return nullptr;
        }
        children.push_back({name != nullptr ? name : "", child});
        return child;
    }

    bool UIContainer::removeChild(const char* name)
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");
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

    UIObject* UIContainer::getChild(const char* name) const
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");
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


} // namespace magique
