#ifndef MAGIQUE_UI_CONTAINER_H
#define MAGIQUE_UI_CONTAINER_H

#include <vector>
#include <magique/ui/UIObject.h>
#include <magique/internal/InternalTypes.h>
IGNORE_WARNING(4100)

//-----------------------------------------------
// UIContainer
//-----------------------------------------------
// .....................................................................
// A container for ui-objects that are referenced by name. This is useful to organize ui elements with multiple components
// Note: UIContainer is a subclass of UIObject and has its functionality (onDraw(), onUpdate(), ...)
// .....................................................................

namespace magique
{
    struct UIContainer : UIObject
    {
        // Creates the container from absolute dimensions in the logical UI resolution
        // Optionally specify an anchor point the object is anchored to and a scaling mode
        UIContainer(float x, float y, float w, float h, ScalingMode scaling = ScalingMode::FULL);
        UIContainer(float w, float h, ScalingMode scaling = ScalingMode::FULL);
        UIContainer(AnchorPosition anchor, float w, float h, ScalingMode scaling = ScalingMode::FULL);

        // Controls how the container including all of its children are visualized!
        // Note: It's the containers responsibility to draw all of its children!
        void onDraw(const Rectangle &bounds) override{}

    protected:
        // Adds a new child to the container with an optional name identifier
        // Pass a new instance of your class new MyClass() - the name will be copied if specified
        // Note: the container takes ownership of the child pointer
        void addChild(UIObject* child, const char* name = nullptr);

        // Returns true the child associated with the given name or index was removed
        // Failure: returns wrong if no child with the given name or index exists
        bool removeChild(const char* name);
        bool removeChild(int index);

        // Returns a pointer to the child associated with the given name (if any)
        // Failure: returns nullptr if the name doesn't exist
        [[nodiscard]] UIObject* getChild(const char* name) const;

        // Returns a vector that contains all current children
        [[nodiscard]] const std::vector<UIObject*>& getChildren() const;

        ~UIContainer() override = default;

    private:
        std::vector<UIObject*> children;
        std::vector<internal::UIContainerMapping> nameMapping;
    };
} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_UI_CONTAINER_H