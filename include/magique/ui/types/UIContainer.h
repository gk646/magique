#ifndef MAGIQUE_UICONTAINER_H
#define MAGIQUE_UICONTAINER_H

#include <vector>
#include <magique/ui/types/UIObject.h>

//-----------------------------------------------
// UI Container - Base UI Class
//-----------------------------------------------
// .....................................................................
// A container for ui-objects. The position of all children is relative to the container position.
// Moving the parent automatically moves all children!
// .....................................................................

namespace magique
{
    struct UIContainer : UIObject
    {
        // Constructor with absolute coordinates in the logical resolution
        UIContainer(float x, float y, float w, float h);
        ~UIContainer() override = default;

        // Submits the container (and all its children) to be rendered (and updated) this tick
        // transparency         - controls the opacity of container and its children
        // scissor              - if set everything outside the container bounds will not be visible (scissors mode)
        void render(float transparency = 1.0F, bool scissor = false);

    protected:
        // Draws the container itself
        void draw(const Rectangle& bounds) override {}

        // Updates the container itself
        void update(const Rectangle& bounds, bool isDrawn) override {}

    public:
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