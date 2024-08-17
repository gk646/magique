#include <magique/ui/types/UIContainer.h>

#include "internal/globals/UIData.h"

namespace magique
{
    UIContainer::UIContainer(const float x, const float y, const float w, const float h) : UIObject(x, y, w, h) {}

    void UIContainer::render(const float transparency, const bool scissor)
    {
        global::UI_DATA.addRenderObject(*this, transparency, scissor);
    }

    void UIContainer::addChild(const char* name, UIObject* child) { children.push_back(child); }

} // namespace magique