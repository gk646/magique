#include <magique/ui/types/UIContainer.h>

namespace magique
{
    UIContainer::UIContainer(const float percentX, const float percentY, const float percentHeight,
                             const float percentWidth) : UIObject(percentX, percentY, percentHeight, percentWidth)
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