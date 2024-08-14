#include <magique/ui/types/UIContainer.h>

namespace magique
{
    UIContainer::UIContainer(const GameState gameState, const float x, const float y, const float w, const float h,
                             const UILayer layer) : UIObject( x, y, w, h, layer)
    {
    }

    void UIContainer::drawChildren() const
    {
        for (auto* const obj : children)
        {

        }
    }

    void UIContainer::updateChildren() const
    {
        for (auto* const obj : children)
        {
          //  obj->update();
        }
    }

    void UIContainer::addChild(const char* name, UIObject* child)
    {
        children.push_back(child);
    }

} // namespace magique