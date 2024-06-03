#ifndef MAGIQUE_UICONTAINER_H
#define MAGIQUE_UICONTAINER_H

#include <memory>
#include <vector>

#include <magique/ui/UIObject.h>

//-----------------------------------------------
// UI Container - Base UI Class
//-----------------------------------------------


namespace magique
{
    struct UIContainer : UIObject
    {
        std::vector<std::unique_ptr<UIObject>> children;

        virtual ~UIContainer() = default;
        virtual void reposition();
    };
} // namespace magique


#endif //MAGIQUE_UICONTAINER_H