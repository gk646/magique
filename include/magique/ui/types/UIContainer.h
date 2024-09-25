#ifndef MAGIQUE_UI_CONTAINER_H
#define MAGIQUE_UI_CONTAINER_H

#include <vector>
#include <magique/ui/UIObject.h>
#include <magique/internal/InternalTypes.h>
IGNORE_WARNING(4100)

//-----------------------------------------------
// UI Container - Base UI Class
//-----------------------------------------------
// .....................................................................
// A container for ui-objects that should be used whenever multiple ui elements should be together.
// Note: The position of all children is relative to the container position.
//       -> Moving the parent automatically moves all children!
// .....................................................................

namespace magique
{
    struct UIContainer : UIObject
    {
        // Constructor with absolute coordinates in the logical resolution
        UIContainer(float x, float y, float w, float h);
        ~UIContainer() override = default;

        // Submits the container (and all its children) to be rendered (and updated) this tick
        //      - transparency: controls the opacity of container and its children
        //      - scissor     : if set, everything outside the container bounds will not be visible (scissors mode)
        void render(float transparency = 1.0F, bool scissor = false);

    protected:
        // Draws the container itself
        void draw(const Rectangle& bounds) override {}

        // Updates the container itself
        void update(const Rectangle& bounds, bool isDrawn) override {}

    public:
        // Adds a new child at the given position relative to the container (sets the child position)
        // Pass new instance of your class new MyClass() - if valid name will be copied
        // Note: passed pointer is managed by the engine and must not be accessed anymore
        void addChild(float x, float y, UIObject* child, const char* name = nullptr);

        // Returns true the child associated with the given name or index was removed
        // Failure: returns wrong if no child with the given name or index exists
        bool removeChild(const char* name);
        bool removeChild(int index);

        // Returns a pointer to the child associated with the given name or index (if any)
        // Failure: returns nullptr if the name or index doesn't exist
        [[nodiscard]] UIObject* getChild(const char* name) const;
        [[nodiscard]] UIObject* getChild(int index) const;

        // Returns the amount of children within this container
        [[nodiscard]] int getChildrenCount() const;

    private:
        std::vector<internal::UIContainerEntry> children;
    };
} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_UI_CONTAINER_H