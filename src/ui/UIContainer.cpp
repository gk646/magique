// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/types/UIContainer.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{

    UIContainer::UIContainer(const float x, const float y, const float w, const float h, const ScalingMode scaling) :
        UIObject(x, y, w, h, scaling)
    {
        global::UI_DATA.registerObject(this, true);
    }

    UIContainer::UIContainer(const float w, const float h, const Anchor anchor, const float inset,
                             const ScalingMode scaling) : UIObject(w, h, anchor, inset, scaling)
    {
        global::UI_DATA.registerObject(this, true);
    }

    UIObject* UIContainer::addChild(UIObject* child, const char* name)
    {
        for (const auto oldChild : children)
        {
            if (oldChild == child)
            {
                LOG_ERROR("Given child already exists");
                return nullptr;
            }
        }
        if (name != nullptr)
        {
            const int len = static_cast<int>(strlen(name));
            if (len + 1 > MAGIQUE_MAX_NAMES_LENGTH)
            {
                LOG_WARNING("Given name is longer than configured!: %s", name);
                return nullptr;
            }

            for (const auto& mapping : nameMapping)
            {
                if (strcmp(mapping.name, name) == 0)
                {
                    LOG_ERROR("Given name already exists");
                    return nullptr;
                }
            }
            internal::UIContainerMapping mapping;
            memcpy(mapping.name, name, len);
            mapping.name[MAGIQUE_MAX_NAMES_LENGTH - 1] = '\0';
            mapping.index = static_cast<int>(children.size());
            nameMapping.push_back(mapping);
        }
        children.push_back(child);
        return child;
    }

    bool UIContainer::removeChild(const char* name)
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");
        for (auto it = nameMapping.begin(); it != nameMapping.end(); ++it)
        {
            if (strcmp(it->name, name) == 0)
            {
                children.erase(children.begin() + it->index);
                nameMapping.erase(it);
                return true;
            }
        }
        LOG_WARNING("Child with name '%s' not found in container", name);
        return false;
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

    UIObject* UIContainer::getChild(const char* name) const
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");
        for (const auto& mapping : nameMapping)
        {
            if (strcmp(mapping.name, name) == 0)
            {
                return children[mapping.index];
            }
        }
        LOG_WARNING("No child named '%s' in this container", name);
        return nullptr;
    }

    UIObject* UIContainer::getChild(const int index) const
    {
        if (index >= static_cast<int>(children.size()))
        {
            LOG_WARNING("Child with index %d is out of bounds", index);
            return nullptr;
        }
        return children[index];
    }

    const std::vector<UIObject*>& UIContainer::getChildren() const { return children; }

} // namespace magique