#ifndef MAGIQUE_UI_OBJECT_H
#define MAGIQUE_UI_OBJECT_H

#include <magique/core/Types.h>
#include <magique/internal/Macros.h>
IGNORE_WARNING(4100) // unreferenced formal parameter

//-----------------------------------------------
// UIObject
//-----------------------------------------------
// .....................................................................
// The UIObject is the base class of all ui elements.
// UIObjects can be positioned in multiple ways:
//  - At construction:
//      - Specify a position - will stay at this position until changed (does not adjust to the objects size!)
//      - Specify an anchor - will then be anchored until changed (scales with the objects dimensions dynamically)
//  - At runtime (these method need to be called again in order to react to changes):
//      - Use align() with a anchor position to align the object inside another object
//      - Use align() with a direction to align the object outside another object
//
// Note: Check the ScalingMode enum (core/Types.h) for more info on how scaling is applied
// Note: If you declare a UIObject statically you have to use trackObject() for it to be updated
// .....................................................................

namespace magique
{
    struct UIObject
    {
        // Creates the object from absolute dimensions in the logical UI resolution (see ui/UI.h)
        // Optionally specify an anchor point the object is anchored to and a scaling mode
        UIObject(float x, float y, float w, float h, ScalingMode scaling = ScalingMode::FULL);
        UIObject(float w, float h, ScalingMode scaling = ScalingMode::FULL);
        UIObject(AnchorPosition anchor, float w, float h, ScalingMode scaling = ScalingMode::FULL);

        //----------------- CORE -----------------//

        // Immediately calls onDraw() and renders the object - additionally tracks the rendering internally
        void draw();

    protected:
        // Controls how the object is visualized - should only be used to draw
        // Called with getBounds()
        virtual void onDraw(const Rectangle& bounds) {}

        // Controls how the object is updated - called automatically at the end of each update tick
        // The call order is sorted after draw order dynamically - the elements on top are updated first
        //      - wasDrawn: if the object was drawn last tick
        // Note: This allows for objects to be updated regardless if they are drawn or not (background task,...)
        // Note: All UIContainers are updated separately and before objects
        virtual void onUpdate(const Rectangle& bounds, bool wasDrawn) {}

        // Same as onUpdate() but called at the beginning of the draw tick (each render tick)
        // This is essential for real time components (like dragging windows)
        virtual void onDrawUpdate(const Rectangle& bounds) {}

    public:
        // Returns the bounds of this object
        [[nodiscard]] Rectangle getBounds() const;

        // Sets a new position for this object - values are scaled to the CURRENT resolution
        void setPosition(float x, float y);

        // Sets new dimensions for this object - values are scaled to the CURRENT resolution
        // Note: Negative values will be ignored
        void setSize(float width, float height);

        // Aligns this object inside the given object according to the anchor point - 'inset' moves the position inwards
        // Note: See ui/UI.h for a detailed description where the anchor points are
        void align(AnchorPosition alignAnchor, const UIObject& relativeTo, float inset = 0.0F);

        // Aligns the object around the given anchor object - offset is applied in the given direction
        // Note: See the Direction enum (core/Types.h) for more info how this alignment happens
        void align(Direction direction, const UIObject& relativeTo, float offset = 0.0F);

        // Returns true if the cursor is over the object
        [[nodiscard]] bool getIsHovered() const;

        // Returns true if specified mouse button is pressed while the object is hovered
        [[nodiscard]] bool getIsClicked(int mouseButton) const;

        // Controls the anchor position of the object - set to AnchorPosition::NONE in order to un-anchor the object
        // Default: NONE
        void setAnchor(AnchorPosition anchor);
        [[nodiscard]] AnchorPosition getAnchor() const;

        // Controls the scaling mode of the object
        // Note: Check the ScalingMode enum (core/Types.h) for more info on how scaling is applied
        // Default: FULL
        void setScalingMode(ScalingMode scaling);
        [[nodiscard]] ScalingMode getScalingMode() const;

        // Returns true if the object was drawn in the last tick
        [[nodiscard]] bool getWasDrawn() const;

        // Note: Only needs to be called for statically declared objects (e.g. objects created at program startup, globals)
        // Those object are not tracked internally and thus might not be updated automatically - no effect on tracked objects
        void trackObject();

        virtual ~UIObject();

    private:
        float px = 0, py = 0, pw = 0, ph = 0;         // Percent values for the dimensions
        ScalingMode scaleMode = ScalingMode::FULL;    // How the object scales with different screen dimensions
        AnchorPosition anchor = AnchorPosition::NONE; // Where (and if) the object is anchored to on the screen
        bool wasDrawnLastTick = false;
        bool isContainer = false;
        friend struct UIData;
        friend struct Window;
    };

} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_UI_OBJECT_H