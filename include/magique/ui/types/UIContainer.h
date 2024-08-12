#ifndef MAGIQUE_UICONTAINER_H
#define MAGIQUE_UICONTAINER_H

#include <vector>
#include <magique/ui/types/UIObject.h>

//-----------------------------------------------
// UI Container - Base UI Class
//-----------------------------------------------
// .....................................................................
// A container for ui-objects. All positions of its children are relative to it!
// Moving the parent automatically moves all children!
// .....................................................................

namespace magique
{
    struct UIContainer : UIObject
    {
        // Constructor with absolute coordinates in the logical resolution
        UIContainer(GameState gameState, float x, float y, float w, float h, UILayer layer = UILayer::MEDIUM);
        ~UIContainer() override = default;

        // Draws the container itself
        void draw() override {}

        // Updates the container itself
        void update() override {}

        //----------------- CHILDREN -----------------//

        // Draws all visible children
        void updateChildren() const;

        // Updates all children
        void drawChildren() const;

        // Adds a new child - pass a new Instance of your class with new MyClass()
        // Note: passed pointer is managed by the engine and must not be accessed anymore
        void addChild(const char* name, UIObject* child);

        // Removes the child associated with the given name (if any)
        void removeChild(const char* name);

        // Returns a pointer to the child associated with the given name (if any)
        // Failure: returns nullptr if the name doesnt exist
        UIObject* getChild(const char* name) const;

    private:
        std::vector<UIObject*> children;
    };
} // namespace magique


#endif //MAGIQUE_UICONTAINER_H