// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UI_CONTAINER_H
#define MAGIQUE_UI_CONTAINER_H

#include <vector>
#include <magique/ui/UIObject.h>

//===============================================
// UIContainer
//===============================================
// .....................................................................
// UIContainer allows to programmatically store and retrieve UIObjects
// As a rule of thumb:
//  - If you have only a few distinct classes that make up the whole thing use a UIObject with UIObject members
//  - If you have many members and need to access them programmatically (loops, ...) use UIContainer
// Note: UIContainer::onUpdate() is called before its children
// Note: UIContainer is a subclass of UIObject and has its functionality (onDraw(), onUpdate(), ...)
// .....................................................................

namespace magique
{
    struct ContainerChild
    {
        std::string name;
        UIObject* object = nullptr;
    };

    struct UIContainer : UIObject
    {
        // Creates the container from absolute dimensions in the logical UI resolution
        // Optionally specify an anchor point the object is anchored to and a scaling mode
        UIContainer(float x, float y, float w, float h, ScalingMode scaling = ScalingMode::FULL);
        UIContainer(float w, float h, Anchor anchor = Anchor::NONE, Point inset = {},
                    ScalingMode scaling = ScalingMode::FULL);

    protected:
        // Controls how the container including all of its children are visualized!
        // Note: It's the containers responsibility to draw all of its children!
        void onDraw(const Rectangle& bounds) override {}

    public:
        // Adds a new child to the container with an optional name identifier
        // Pass a new instance of your class new MyClass() - the name will be copied if specified
        // Note: the container takes ownership of the child pointer
        // Returns: the added child if successful, otherwise nullptr
        UIObject* addChild(UIObject* child, const char* name = nullptr);

        // Returns true the child associated with the given name or index was removed
        // Failure: returns wrong if no child with the given name or index exists
        bool removeChild(const char* name);
        bool removeChild(int index);
        bool removeChild(UIObject* child);

        // Returns a pointer to the child associated with the given name (if any)
        // Failure: returns nullptr if the name doesn't exist
        [[nodiscard]] UIObject* getChild(const char* name) const;
        [[nodiscard]] UIObject* getChild(int index) const;

        // Returns a vector that contains all current children
        [[nodiscard]] const std::vector<ContainerChild>& getChildren() const;

        ~UIContainer() override = default;

    private:
        std::vector<ContainerChild> children;
    };
} // namespace magique

#endif // MAGIQUE_UI_CONTAINER_H
