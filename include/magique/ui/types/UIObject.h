#ifndef MAGIQUE_UIOBJECT_H
#define MAGIQUE_UIOBJECT_H

#include <magique/core/Types.h>

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

        // Doesnt initialize the dimensions - use setDimensions()
        explicit UIObject(GameState gameState, UILayer layer = UILayer::MEDIUM);

        // Aligns the object object after the anchor position relative to the given ui object
        UIObject(AnchorPosition anchor, const UIObject& relativeTo, float inset = 0.0F);

        // Creates the object from absolute dimensions in the logical UI resolution
        UIObject(GameState gameState, float x, float y, float w, float h, UILayer layer = UILayer::MEDIUM);

        virtual ~UIObject();

        //----------------- CORE -----------------//

        // Called each update tick automatically - DONT call manually
        virtual void draw() {}

        // Called each render tick automatically - DONT call manually
        virtual void update() {}

        // Aligns the object object after the anchor position relative to the given ui object
        void align(AnchorPosition anchor, const UIObject& relativeTo, float inset = 0.0F);

        // Attempt to click on this object - passed the mouse coordinates
        // Returns true if the click was successful
        virtual bool click(float x, float y) { return false; }

        // Attempt to hover over this object - pos is absolute
        virtual bool hover() { return false; }

        //----------------- EVENTS -----------------//

        // Called when this object is added as children
        virtual void onAdd() {}

        // Called when its object is move - passed the new absolute x and y coordinates
        virtual void onMove(float x, float y) {}

        //----------------- SETTERS -----------------//

        // Sets the z index manually - its automatically set when the object is added to a container
        void setLayer(UILayer layer);

        // Sets the visibility of the object
        void setShown(bool val);

        // Sets new dimensions from the given values inside the logical UI resolution
        // Note: Negative values will be ignored
        void setDimensions(float x, float y, float w = -1.0F, float h = -1.0F);

        //----------------- GETTERS -----------------//

        // Returns true if the object should be drawn
        [[nodiscard]] bool getIsShown() const;

        // Returns the z index
        [[nodiscard]] UILayer getLayer() const;

        // Returns the bounds of this object
        [[nodiscard]] Rectangle getBounds() const;

        // Returns true if the mouse is over the object and the object is not obstructed by another object
        [[nodiscard]] bool getIsHovered() const;

        // Returns true if the mouse button is pressed while the object is not obstructed by another object
        [[nodiscard]] bool getIsPressed(int mouseButton) const;

    private:
        float px = 0, py = 0, pw = 0, ph = 0; // Percent values for the dimensions
        uint16_t id = 0;                      // Unique id
        uint8_t instances = 0;                // How many time it exists across gamestates
        uint8_t layer = 1;                    // Layer of the object - automatically set when added to a container
        bool isShown = true;                  // Visibility status - starts with True
        friend struct UIData;
    };

} // namespace magique
#endif //MAGIQUE_UIOBJECT_H