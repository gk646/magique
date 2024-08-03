#ifndef MAGIQUE_UIOBJECT_H
#define MAGIQUE_UIOBJECT_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// UIObject
//-----------------------------------------------
// .....................................................................
// The UIObject is the base class of all ui elements. Its members are the percent values for its position, its unique id,
// a pointer to the parent (may be null) and a z-index.
// The z index controls at which layer the element is drawn. The closer to 0 means right in the screen and the bigger the
// z-index the further away you get from the screen. It's set automatically when adding it to a UIContainer but can be
// adjusted manually if needed.
// Note: You can just pass absolute coordinates (in the logical UI resolution of 1920x1080) and the it will
// automatically scale to the current resolution
// .....................................................................

namespace magique
{
    struct UIObject
    {
        //----------------- CONSTRUCTORS -----------------//
        //Note: name will be copied and only has to be valid until the constructor returns

        // Doesnt initialize the dimensions - use align()
        explicit UIObject();

        // Aligns the object object after the anchor position relative to the given ui object
        UIObject(AnchorPosition anchor, const UIObject& relativeTo, float inset = 0.0F);

        // Creates the object from absolute dimensions in the logical UI resolution
        UIObject(float x, float y, float w, float h);

        virtual ~UIObject();

        //----------------- CORE -----------------//

        // Called each tick on the main thread
        virtual void draw() {}

        // Called each tick
        // Thread: On the update thread
        virtual void update() {}

        // Aligns the object object after the anchor position relative to the given ui object
        void align(AnchorPosition anchor, const UIObject& relativeTo, float inset = 0.0F);

        // Attempt to click on this object
        // Returns true if the click was successful
        virtual bool click(const Vector2& pos) { return false; }

        // Attempt to hover over this object - pos is absolute
        virtual bool hover() { return false; }

        //----------------- EVENTS -----------------//

        // Called when this object is added as children
        virtual void onAdd() {}

        // Called when its moved or resized
        virtual void onMove() {}

        //----------------- SETTERS -----------------//

        // Sets the z index manually - its automatically set when the object is added to a container
        void setZIndex(int z);

        // Sets the visibility of the object
        void setShown(bool val);

        // Sets new dimensions for this object from given coordinates insdie the logical UI resolution
        // Note: Negative values will be ignored
        void setDimensions(float x, float y, float w = -1.0F, float h = -1.0F);

        //----------------- GETTERS -----------------//

        // Returns true if the object should be drawn
        [[nodiscard]] bool getIsShown() const;

        // Returns the z index
        [[nodiscard]] int getZIndex() const;

        // Returns the bounds of this object
        [[nodiscard]] Rectangle getBounds() const;

        // Returns true if the mouse is over the object and the object is not obstructed by another object
        [[nodiscard]] bool getIsHovered() const;

        // Returns true if the mouse button is pressed while the object is not obstructed by another object
        [[nodiscard]] bool getIsPressed(int mouseButton) const;

    private:
        UIObject* parent = nullptr;
        float px = 0, py = 0, pw = 0, ph = 0; // Percent values for the dimensions
        uint16_t id = 0;                      // Unique identifier
        uint16_t z = 1;                       // Z-index - automatically set when added to a container
        bool isShown = true;                  // Visibility status - starts with True
    };

} // namespace magique
#endif //MAGIQUE_UIOBJECT_H