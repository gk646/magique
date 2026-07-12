// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UI_CONTAINER_H
#define MAGIQUE_UI_CONTAINER_H

#include <functional>
#include <span>
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
// Note: Also contains VerticalContainer˛- auto expands vertically
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
        UIContainer(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode mode = ScalingMode::FULL);

    protected:
        // Controls how the container including all of its children are visualized!
        // Note: It's the containers responsibility to draw all of its children!
        void onDraw(const Rect& bounds) override {}

    public:
        // Adds a new child to the container with an optional name identifier
        // Pass a new instance of your class new MyClass() - the name will be copied if specified
        // Note: the container takes ownership of the child pointer
        // Returns: the added child if successful, otherwise nullptr
        UIObject* addChild(UIObject* child, std::string_view name = {});

        template <typename T, typename... Args>
        T& emplaceChild(Args&&... args);

        // Adds many children with no names
        void addChildren(const std::initializer_list<UIObject*>& newChildren);
        void addChildren(std::span<UIObject*> newChildren);

        // Returns true the child associated with the given name or index was removed
        // Failure: returns wrong if no child with the given name or index exists
        bool removeChild(std::string_view name);
        bool removeChild(int index);
        bool removeChild(UIObject* child);

        // Returns a pointer to the child associated with the given name (if any)
        // Failure: returns nullptr if the name doesn't exist
        UIObject* getChild(std::string_view name) const;
        UIObject* getChild(int index) const;

        // Returns a vector that contains all current children
        const std::vector<ContainerChild>& getChildren() const;

        ~UIContainer() override = default;

    private:
        std::vector<ContainerChild> children;
    };

    // Stacks the given children vertically and automatically adjusts its size to fit
    // The alignment of the content can either be centered, left or right
    // Note: This is useful to use as content for a ScrollPane or in Menus (when stacking buttons or controls)
    // Note: Automatically fits its size to the content (if its larger than base size)
    struct VerticalContainer : UIContainer
    {
        VerticalContainer(const Rect& bounds = {}) : UIContainer(bounds) {}

        // Sets/gets the horizontal alignment of the children - only determines if LEFT | CENTER | RIGHT
        void setHorizontalAlign(Anchor align = Anchor::TOP_CENTER);
        Anchor getHorizontalAlign() const;

        float getGap() const;
        void setGap(float gap = 3.0F);

        // If true draws them in reverse
        // Note: This is useful when drawing dropdowns so the list doesn't get overdrawn by controls below
        void setReverseDraw(bool reverse = true);

    protected:
        void onDraw(const Rect& bounds) override;

    private:
        Anchor anchor = Anchor::TOP_CENTER;
        float gap = 3.0F;
        bool reverseDraw = false;
    };

    struct HorizontalContainer : UIContainer
    {
        // Sets/gets the vertical alignment of the children - only determines if UP | MID | BOTTOM
        void setHorizontalAlign(Anchor align = Anchor::MID_CENTER);
        Anchor getHorizontalAlign() const;

        float getGap() const;
        void setGap(float gap = 3.0F);

    protected:
        void onDraw(const Rect& bounds) override;

    private:
        Anchor anchor = Anchor::MID_CENTER;
        float gap = 3.0F;
    };


} // namespace magique


/// IMPLEMENTATION


namespace magique
{
    template <typename T, typename... Args>
    T& UIContainer::emplaceChild(Args&&... args)
    {
        return *static_cast<T*>(addChild(new T(std::forward<Args>(args)...)));
    }
} // namespace magique


#endif // MAGIQUE_UI_CONTAINER_H
