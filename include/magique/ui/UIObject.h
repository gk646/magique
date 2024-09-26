#ifndef MAGIQUE_UI_OBJECT_H
#define MAGIQUE_UI_OBJECT_H

#include <magique/core/Types.h>
#include <magique/internal/Macros.h>
IGNORE_WARNING(4100) // unreferenced formal parameter

//-----------------------------------------------
// UIObject
//-----------------------------------------------
// .....................................................................
// The UIObject is the base class of all ui elements. The rendering of objects happens deferred.
// You should call .render() on all objects that should be drawn in a tick. They are collected and sorted to be drawn
// in the correct order after Game::drawUI() returns.
//
// IMPORTANT: Render order is determined first by layer, then by draw call order.
//            If no layers are explicitly set, UIObjects will appear on screen in the order they are called to render!
//
// Note: Passed coordinates are assumed to be in the logical UI resolution of 1920x1080 (see ui/UI.h for more info)
// .....................................................................

namespace magique
{
    struct UIObject
    {
        // Creates the object from absolute dimensions in the logical UI resolution
        UIObject(float x, float y, float w = 50, float h = 50);

        //----------------- CORE -----------------//

        // Submits the UIObject to be rendered (and updated) this tick
        // transparency         - controls the opacity of the whole UIObject
        // scissor              - if set everything outside the object bounds will not be visible (scissors mode)
        void render(float transparency = 1.0F, bool scissor = false);

    protected:
        // Called every update tick automatically BEFORE it is drawn
        //      - isDrawn: if the object will also be drawn this tick
        // Note: This allows for components to be updated regardless if they are drawn or not (background task,...)
        virtual void update(const Rectangle& bounds, bool isDrawn) {}

        // Called after Game::drawUI() returns to draw the object if the object has been submitted with render()
        // Note: The call is skipped if the object is fully covered by an opaque object or is not on the screen
        virtual void draw(const Rectangle& bounds) {}

    public:
        //----------------- UTIL -----------------//

        // Returns the bounds of this object
        [[nodiscard]] Rectangle getBounds() const;

        // Sets new dimensions from the given values inside the logical UI resolution
        // Note: Negative values will be ignored
        void setDimensions(float x, float y, float w = -1.0F, float h = -1.0F);

        // Aligns the object after the anchor position relative to the given ui object
        void align(AnchorPosition anchor, const UIObject& relativeTo, float inset = 0.0F);

        // Returns true if the cursor is over the object and is not obstructed by any other object
        [[nodiscard]] bool getIsHovered() const;

        // Returns true if the mouse button is pressed while the object is not obstructed by another object
        [[nodiscard]] bool getIsClicked(int mouseButton) const;

        // Controls the layer explicitly
        // Note: Per default all elements are in the same layer, and thus they appear in the order they are called
        //       -> If explicit layers are set they will override this behavior!
        void setLayer(UILayer layer);
        UILayer getLayer() const;

        // Controls if the ui object is considered to be opaque (solid, not see through)
        // All objects fully covered by an opaque object are considered not visible and won't be drawn!
        // Default : false
        void setOpaque(bool val);
        bool getIsOpaque() const;

        // Sets the visibility status of the object
        // Note: if the object is set to be not visible it will NOT be rendered even when submitted to do so!
        // Default: true
        void setVisible(bool val);
        bool getIsVisible() const;

        virtual ~UIObject();

    private:
        float px = 0, py = 0, pw = 0, ph = 0; // Percent values for the dimensions
        uint16_t id = 0;                      // Unique id
        UILayer layer = UILayer::MEDIUM;      // Layer of the object
        AnchorPosition anchor;
        bool isOpaque = false;                // If the object is solid and not see-through
        bool isVisible = true;
        friend struct UIData;
    };

} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_UI_OBJECT_H