#ifndef MAGIQUE_UIOBJECT_H
#define MAGIQUE_UIOBJECT_H

#include <magique/core/Types.h>

//-----------------------------------------------
// UIObject
//-----------------------------------------------
// .....................................................................
// The UIObject is the base class of all ui elements. The rendering of objects happens deffered.
// You should call .render() on all objects that should be drawn this tick. They are collected and sorted to be drawn
// in the correct order after Game::drawUI() returns.
//
// IMPORTANT: Render order is determined first by layer, then by draw call order.
//            If no layers are explicitly set, UIObjects will appear on screen in the order they are drawn.
//
// Note: Passed coordinates are assumged to be in the logical UI resolution of 1920x1080 (see ui/UI.h for more info)
// .....................................................................

namespace magique
{
    struct UIObject
    {
        //----------------- CONSTRUCTORS -----------------//

        // Creates the object from absolute dimensions in the logical UI resolution
        UIObject(float x, float y, float w, float h, UILayer layer = UILayer::MEDIUM);

        // Doesnt initialize the dimensions - use setDimensions()
        explicit UIObject(UILayer layer = UILayer::MEDIUM);

        // Aligns the object object after the anchor position relative to the given ui object
        UIObject(AnchorPosition anchor, const UIObject& relativeTo, float inset = 0.0F);

        //----------------- CORE -----------------//

        // Submits the UIObject to be rendered this tick
        void render(float transparency = 1.0F);

    protected:
        // Called each update tick automatically BEFORE draw()
        // isDrawn      - if the object will be drawn this tick aswell
        // Note: This allows components that are not rendered to be updated aswell
        virtual void update(const Rectangle& bounds, bool isDrawn) {}

        // Called after Game::drawUI() returns to draw the object
        // Note: Not called if: Object is fully covered by an opaque object / object is not on the screen
        virtual void draw(const Rectangle& bounds) {}

    public:
        // Aligns the object after the anchor position relative to the given ui object
        void align(AnchorPosition anchor, const UIObject& relativeTo, float inset = 0.0F);

        //----------------- UTIL -----------------//

        // Returns the bounds of this object
        [[nodiscard]] Rectangle getBounds() const;

        // Sets new dimensions from the given values inside the logical UI resolution
        // Note: Negative values will be ignored
        void setDimensions(float x, float y, float w = -1.0F, float h = -1.0F);

        // Returns true if the cursor is over the object and is not obstructed by any other object
        [[nodiscard]] bool getIsHovered() const;

        // Returns true if the mouse button is pressed while the object is not obstructed by another object
        [[nodiscard]] bool getIsPressed(int mouseButton) const;

        // Controls the layer explicitly
        // Note: Per default all elements are in the same layer and thus they appear in the order they are called
        //       -> If explicit layers are set they will override that behavior!
        void setLayer(UILayer layer);
        UILayer getLayer() const;

        // Controls if the ui object is considered to be opaque (solid, not see through)
        // All objects fully covered by a opaque one are thus not visible and dont have to be drawn!
        // Default : false
        void setOpaque(bool val);
        bool getIsOpaque() const;

        virtual ~UIObject();

    private:
        float px = 0, py = 0, pw = 0, ph = 0; // Percent values for the dimensions
        uint16_t id = 0;                      // Unique id
        uint8_t instances = 0;                // How many time it exists across gamestates
        uint8_t layer = 1;                    // Layer of the object - automatically set when added to a container
        friend struct UIData;
    };

} // namespace magique
#endif //MAGIQUE_UIOBJECT_H