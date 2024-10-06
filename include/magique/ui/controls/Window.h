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
//      - Window Buttons: window buttons can be added manually
//|----------------------|
//| TopBar               |
//|----------------------|
//|                      |
//|                      |
//| Body                 |
//|----------------------|
// Note: The window is a subclass of the UIContainer and has all its functionality
// Note: Dragging the window is done automatically - everything inside the topBar is considered draggable (except buttons)
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
        // Note: Dragging the window is done automatically - everything inside the TopBar is draggable
        void onUpdate(const Rectangle& bounds, bool wasDrawn) override {}

    public:
        // Returns the bounds of the body of this window
        [[nodiscard]] Rectangle getBodyBounds() const;

        // Returns the bounds of the top bar
        [[nodiscard]] Rectangle getTopBarBounds() const;

        //----------------- Window Buttons -----------------//

        // Adds a new window button to this window with the given name - anchored inside the window
        void addWindowButton(WindowButton* window, const char* name, AnchorPosition anchor);

        // Adds a new window button to this window with the given name - relative to an existing button (given by its name)
        void addWindowButton(WindowButton* window, const char* name, Direction direction, const char* relativeTo);

        // Returns the button identified by the given name
        // Failure: return nullptr if the name does not exist
        WindowButton* getWindowButton(const char* name);

    private:
        void drawDefault(); // Default visuals

        Point lastMousePos{};
        float moverHeightP = 10.0F;
    };

    struct WindowButton : UIObject
    {
    };


} // namespace magique

UNIGNORE_WARNING()

#endif //MAGIQUE_WINDOW_H