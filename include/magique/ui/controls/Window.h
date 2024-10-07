#ifndef MAGIQUE_WINDOW_H
#define MAGIQUE_WINDOW_H

#include <raylib/raylib.h>
#include <magique/ui/types/UIContainer.h>
IGNORE_WARNING(4100)

//-----------------------------------------------
// Window
//-----------------------------------------------
// .....................................................................
// Generic window class which can be subclassed to achieve custom visuals.
// Windows are meant to be used with the window manager (see ui/WindowManager.h) - it handles visibility and layering
// A window consists out of 3 main parts:
//      - Body: Everything that's not the top bar (mover) is considered body
//      - TopBar: The area starting from the top to the topBarHeight - area is automatically draggable
//|----------------------|
//| TopBar               |
//|----------------------|
//|                      |
//|                      |
//| Body                 |
//|----------------------|
//
// IMPORTANT: Its recommended to use the ui/WindowManager.h to manage windows
//            Use its accessor to determine window overlap and layering (e.g. getHoveredWindow(), ...)
// Note: The window is a subclass of the UIContainer and has all its functionality
// Note: Window buttons (close button, maximize, minimize, ...) have to be added manually as children - anchor them accordingly
// Note: Everything requiring real time updates like dragging, should be called in the draw method as it runs every draw tick!
// .....................................................................

namespace magique
{
    struct Window : UIContainer
    {
        // Creates a new window from coordinates in the logical UI resolution
        // If not specified the top bar is 10% of the total height
        Window(float x, float y, float w, float h, float topBarHeight = 0.0);

    protected:
        // Same as ui/UIContainer.h
        // Note: The window is responsible to draw all its children!
        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Same as ui/UIObject.h
        void onDrawUpdate(const Rectangle& bounds) override { updateDrag(getTopBarBounds()); }

    public:
        // Returns the bounds window body
        [[nodiscard]] Rectangle getBodyBounds() const;

        // Returns the bounds of the window top bar
        [[nodiscard]] Rectangle getTopBarBounds() const;

        // Returns true if the window is currently dragged
        [[nodiscard]] bool getIsDragged() const;

        // Returns true if the window was clicked in the given area with the given mouse button
        // Does not check if anything is in front - use the ui/WindowManager.h
        // Note: Uses UIInput to check for click - if true probably want to call UIInput::Consume()
        bool updateDrag(const Rectangle& area, int mouseButton = MOUSE_BUTTON_LEFT);

    private:
        void drawDefault(const Rectangle& bounds) const; // Default visuals
        Point clickOffset{};
        float moverHeightP = 0.10F;
        bool isDragged = false;
    };

} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_WINDOW_H