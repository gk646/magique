#include <magique/ui/types/UIContainer.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{

    bool ContainsEntry(const std::vector<internal::UIContainerEntry>& vec, const char* name, UIObject* obj)
    {
        for (const auto& e : vec)
        {
            if (strcmp(name, e.name) == 0)
            {
                return false;
            }
            if (e.object == obj)
            {
                return false;
            }
        }
        return true;
    }

    UIContainer::UIContainer(const float x, const float y, const float w, const float h) : UIObject(x, y, w, h) {}

    void UIContainer::render(const float transparency, const bool scissor)
    {
        global::UI_DATA.addRenderObject(*this, transparency, scissor);
    }

    void UIContainer::addChild(const char* name, UIObject* child)
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");
        const int len = static_cast<int>(strlen(name));
        if (len + 1 > MAGIQUE_UI_OBJECT_NAME_LEN)
        {
            LOG_WARNING("Given name is longer than configured!: %s", name);
            return;
        }
        if (ContainsEntry(children, name, child))
        {
            LOG_WARNING("Given name or child is already present in this container!: %s", name);
        }
        internal::UIContainerEntry entry{};
        std::memcpy(entry.name, name, len);
        entry.name[MAGIQUE_UI_OBJECT_NAME_LEN - 1] = '\0';
        entry.object = child;
        children.push_back(entry);
    }

    void UIContainer::removeChild(const char* name)
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");

        const auto it = std::ranges::find_if(children, [&](const internal::UIContainerEntry& entry)
                               { return strncmp(entry.name, name, MAGIQUE_UI_OBJECT_NAME_LEN) == 0; });

        if (it != children.end())
        {
            LOG_INFO("Removing child with name: %s", name);
            children.erase(it);
        }
        else
        {
            LOG_WARNING("Child with name '%s' not found in container", name);
        }
    }

    UIObject* UIContainer::getChild(const char* name) const
    {
        MAGIQUE_ASSERT(name != nullptr, "Name must be non-null");

        const auto it =
            std::ranges::find_if(children, [&](const internal::UIContainerEntry& entry)
                                 { return strncmp(entry.name, name, MAGIQUE_UI_OBJECT_NAME_LEN) == 0; });

        if (it != children.end())
        {
            LOG_INFO("Child with name '%s' found", name);
            return it->object;
        }

        LOG_WARNING("Child with name '%s' not found", name);
        return nullptr;
    }


} // namespace magique