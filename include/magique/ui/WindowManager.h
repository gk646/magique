// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_WINDOW_MANAGER_H
#define MAGIQUE_WINDOW_MANAGER_H

#include <vector>
#include <magique/ui/UI.h>

//===============================================
// Window Manager
//===============================================
// .....................................................................
// This module allows to easily control multiple windows and their layering
// Windows are identified by either their pointer or by the name they were added with
// Note: the draw() function to draw the windows needs to be called MANUALLY - windows should then not be drawn individually!
// .....................................................................

namespace magique
{
    // Returns the global instance of the window manager
    WindowManager& WindowManagerGet();

    struct WindowManager final
    {
        // Draws all managed windows that are visible
        void draw();

        // Adds a new window to the window manager - is visible per default
        // Note: takes ownership of the pointer as long as its managed
        void addWindow(Window* window);

        //================= ACCESSORS =================//

        // Returns a managed window by name
        // Failure: returns nullptr if no window was added with that name
        Window* getWindow(const char* name);

        // Returns true if the window was successfully removed - gives back ownership of the window!
        // Failure: returns false if no window matched the identifier
        bool removeWindow(Window* window);

        // Sets the shown status of the given window to the given value
        void setShown(Window* window, bool shown);

        // Returns the shown status of the given window
        bool getIsShown(Window* window);

        // Toggles the shown status of the given window
        void toggleShown(Window* window);

        // Returns a vector that contains all managed windows - sorted from top to bottom, front to back
        const std::vector<Window*>& getWindows();

        //================= LAYERING =================//

        // Moves the first window in front of the second one
        // Note: This is not an immediate action - action will be saved and executed after the tick
        // When moving position at the wrong time immediately inifinte loops and other things can happen
        void moveInFrontOf(Window* moved, Window* inFrontOf);

        //================= UTIL =================//

        // Returns true if the given window is covered by any other another window at the given position
        bool getIsCovered(Window* window, Point pos = GetMousePosition());

        // Makes the given window the top most window
        void makeTopMost(Window* window);

        // Returns a pointer to the window top most window that is also shown
        // Failre: returns nullptr if non exists
        Window* getTopShownWindow();

        // Returns the first window "below" the mouse cursor - checks for layering
        // Failure: returns nullptr if no window area intersects the mouse position
        Window* getHoveredWindow();

        // Hides all registered windows
        void hideAll();

    private:
        void update();
        friend WindowManager& WindowManagerGet();
        friend void InternalUpdatePost();
    };

} // namespace magique

#endif // MAGIQUE_WINDOW_MANAGER_H
