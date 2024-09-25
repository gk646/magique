#include <magique/ui/types/UIContainer.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{

    bool ContainsEntry(const std::vector<internal::UIContainerEntry>& vec, const char* name, UIObject* obj)
    {
        for (const auto& e : vec)
        {
            if (name != nullptr && strcmp(name, e.name) == 0)
            {
                return true;
            }
            if (e.object == obj)
            {
                return true;
            }
        }
        return false;
    }

    UIContainer::UIContainer(const float x, const float y, const float w, const float h) : UIObject(x, y, w, h) {}

    void UIContainer::render(const float transparency, const bool scissor)
    {
        global::UI_DATA.addRenderObject(*this, transparency, scissor);
        for(const auto& e : children)
        {
            global::UI_DATA.addRenderObject(*e.object, transparency, scissor);
        }
    }

    void UIContainer::addChild(const float x, const float y, UIObject* child,const char* name)
    {
        internal::UIContainerEntry entry{};
        if (ContainsEntry(children, name, child))
        {
            LOG_WARNING("Given name or child is already present in this container!: %s", name);
            return;
        }
        if (name != nullptr)
        {
            const int len = static_cast<int>(strlen(name));
            if (len + 1 > MAGIQUE_UI_OBJECT_NAME_LEN)
            {
                LOG_WARNING("Given name is longer than configured!: %s", name);
                return;
            }
            std::memcpy(entry.name, name, len);
            entry.name[MAGIQUE_UI_OBJECT_NAME_LEN - 1] = '\0';
        }
        entry.object = child;
        children.push_back(entry);
    }

    bool UIContainer::removeChild(const char* name)
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");
        const auto it = std::ranges::find_if(children, [&](const internal::UIContainerEntry& entry)
                                             { return strncmp(entry.name, name, MAGIQUE_UI_OBJECT_NAME_LEN) == 0; });
        if (it != children.end())
        {
            LOG_INFO("Removing child with name: %s", name);
            children.erase(it);
            return true;
        }
        LOG_WARNING("Child with name '%s' not found in container", name);
        return false;
    }

    bool UIContainer::removeChild(const int index)
    {
        MAGIQUE_ASSERT(index >= 0, "Name cannot be negative");
        if (index >= children.size())
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
        const auto it = std::ranges::find_if(children, [&](const internal::UIContainerEntry& entry)
                                             { return strncmp(entry.name, name, MAGIQUE_UI_OBJECT_NAME_LEN) == 0; });
        if (it != children.end())
        {
            LOG_INFO("Child with name '%s' found", name);
            return it->object;
        }
        LOG_WARNING("Child with name '%s' not found", name);
        return nullptr;
    }

    UIObject* UIContainer::getChild(const int index) const
    {
        MAGIQUE_ASSERT(index >= 0, "Name cannot be negative");
        if (index >= children.size())
        {
            LOG_WARNING("Child with index %d is out of bounds", index);
            return nullptr;
        }
        return children[index].object;
    }

    int UIContainer::getChildrenCount() const { return static_cast<int>(children.size()); }

} // namespace magique