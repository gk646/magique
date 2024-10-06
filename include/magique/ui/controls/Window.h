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
// .....................................................................

namespace magique
{
    struct Window : UIContainer
    {
        // Creates a new window from coordinates in the logical UI resolution
        // If not specified the top bar is 10% of the total height
        Window(float x, float y, float w, float h, float topBarHeight = 0.0);

    protected:
        // Controls how the window including all of its children are visualized
        //      - bounds: the total bounds of the object - equal to getBounds()
        void onDraw(const Rectangle& bounds) override { drawDefault(); }

        // Controls how the window is updated - called automatically at the end of each update tick
        //      - bounds: the total bounds of the object - equal to getBounds()
        //      - wasDrawn: if the object was drawn last tick
        void onUpdate(const Rectangle& bounds, bool wasDrawn) override
        {
            // if(this == GetWindowManager().getHoveredWindow())  // When using the window manager
            updateDrag(getTopBarBounds());
        }

    public:
        // Returns the bounds window body
        [[nodiscard]] Rectangle getBodyBounds() const;

        // Returns the bounds of the window top bar
        [[nodiscard]] Rectangle getTopBarBounds() const;

        // Returns true if the window is currently dragged
        [[nodiscard]] bool getIsDragged() const;

        // Makes the window draggable in the given area and polls drag for this tick for the given mouse click
        // Note: Uses UIInput and consumes the input if successfully dragged
        void updateDrag(const Rectangle& area, int mouseButton = MOUSE_BUTTON_LEFT);

    private:
        void drawDefault() const; // Default visuals
        Point clickOffset{};
        float moverHeightP = 0.10F;
        bool isDragged = false;
    };

} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_WINDOW_H