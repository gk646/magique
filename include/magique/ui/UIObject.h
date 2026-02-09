// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_UI_OBJECT_H
#define MAGIQUE_UI_OBJECT_H

#include <magique/core/Types.h>
#include <magique/internal/Macros.h>
M_IGNORE_WARNING(4100) // unreferenced formal parameter

//===============================================
// UIObject
//===============================================
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
        UIObject(float w, float h, Anchor anchor = Anchor::NONE, Point inset = {},
                 ScalingMode scaling = ScalingMode::FULL);

        //================= CORE =================//

        // Immediately calls onDraw() and renders the object - additionally tracks the rendering internally
        void draw();

    protected:
        // Controls how the object is visualized - should only be used to draw
        // Called with getBounds()
        virtual void onDraw(const Rectangle& bounds) {}

        // Controls how the object is updated - called automatically before each update tick
        // The call order is sorted after draw order dynamically - the elements on top (drawn last) are updated first
        //      - wasDrawn: if the object was drawn last tick
        // Note: This allows for objects to be updated regardless if they are drawn or not (background task,...)
        // Note: All UIContainers are updated separately and before objects
        virtual void onUpdate(const Rectangle& bounds, bool wasDrawn) {}

        // Same as onUpdate() but called at the beginning of the draw tick (before the update tick)
        // This is essential for real time behavior of components (like dragging windows)
        virtual void onDrawUpdate(const Rectangle& bounds, bool wasDrawn) {}

        // Called each time the window wasn't drawn prior but drawn now (before its drawn this tick)
        virtual void onShown(const Rectangle& bounds) {}

        // Called each time the window was drawn but isn't anymore (at end of update tick if it wasn't drawn)
        virtual void onHide(const Rectangle& bounds) {}

    public:
        // Returns the bounds of this object
        [[nodiscard]] Rectangle getBounds() const;

        // Sets a new position for this object - values are scaled to the CURRENT (target) resolution
        void setPosition(const Point& pos);
        Point getPosition() const;

        // Sets new dimensions for this object - values are scaled to the CURRENT (target) resolution
        // Note: Negative values will be ignored
        void setSize(Point size);

        // Aligns this object inside the given object according to the anchor point - 'inset' moves the position inwards
        // Note: See ui/UI.h for a detailed description where the anchor points are
        // Note: Aligns the object once - use setAnchor() for permanent anchoring
        void align(Anchor alignAnchor, const UIObject& relativeTo, Point alignInset = {});

        // Aligns the object around the given anchor object - offset is applied in the given direction
        // Note: See the Direction enum (core/Types.h) for more info how this alignment happens
        // Note: Aligns the object once - use setAnchor() for permanent anchoring
        void align(Direction direction, const UIObject& relativeTo, Point offset = {});

        // Returns true if the cursor is over the object
        bool getIsHovered() const;

        // Returns true if mouse button is pressed while the object is hovered
        bool getIsClicked(int mouseButton = MOUSE_LEFT_BUTTON) const;

        // Returns true if mouse button is down while the object is hovered
        bool getIsPressed(int mouseButton = MOUSE_LEFT_BUTTON) const;

        // Controls the anchor position of the object on the screen - set to AnchorPosition::NONE in order to un-anchor the object
        // Note: Anchoring is updated each tick automatically
        // Default: NONE
        void setAnchor(Anchor anchor, Point inset = {});
        [[nodiscard]] Anchor getAnchor() const;
        [[nodiscard]] Point getInset() const;

        // Controls the scaling mode of the object
        // Note: Check the ScalingMode enum (core/Types.h) for more info on how scaling is applied
        // Default: FULL
        void setScalingMode(ScalingMode scaling);
        [[nodiscard]] ScalingMode getScalingMode() const;

        // Returns true if the object was drawn in the last tick
        [[nodiscard]] bool getWasDrawn() const;

        // Returns the UIObject cast to the given type
        template <typename T>
        T* getAs();

        // Sets or gets the start position - set per default to the initial position
        void setStartPosition(const Point& pos);
        [[nodiscard]] Point getStartPosition() const;

        // Sets or gets the start position - set per default to the initial position
        void setStartDimensions(const Point& dims);
        [[nodiscard]] Point getStartDimensions() const;

        // Starts a scissor mode with the current bounds - has to be stopped manually!
        void beginBoundsScissor() const;

        virtual ~UIObject();

    private:
        Rect pBounds;
        Point inset{};                        // Inset - offset towards the middle of the screen
        Point startPos{};                          // Default position
        Point startDims{};                         // Default dimensions
        ScalingMode scaleMode = ScalingMode::FULL; // How the object scales with different screen dimensions
        Anchor anchor = Anchor::NONE;              // Where (and if) the object is anchored to on the screen
        bool wasDrawnLastTick = false;
        bool drawnThisTick = false;
        bool isContainer = false;
        befriend(UIData, Window)
    };

} // namespace magique


// IMPLEMENTATION

namespace magique
{
    template <typename T>
    T* UIObject::getAs()
    {
        return static_cast<T*>(this);
    }
} // namespace magique


M_UNIGNORE_WARNING()

#endif //MAGIQUE_UI_OBJECT_H
