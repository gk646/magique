#include <magique/ui/types/UIContainer.h>

namespace magique
{
    UIContainer::UIContainer(const GameState gameState, const float x, const float y, const float w, const float h,
                             const UILayer layer) : UIObject(gameState, x, y, w, h, layer)
    {
    }

    void UIContainer::drawChildren() const
    {
        for (auto* const obj : children)
        {
            if (obj->getIsShown())
            {
                obj->draw();
            }
        }
    }

    void UIContainer::updateChildren() const
    {
        for (auto* const obj : children)
        {
            obj->update();
        }
    }

    void UIContainer::addChild(const char* name, UIObject* child)
    {
        child->onAdd();
        children.push_back(child);
    }

} // namespace magique