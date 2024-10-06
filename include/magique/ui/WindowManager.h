#ifndef MAGIQUE_WINDOW_MANAGER_H
#define MAGIQUE_WINDOW_MANAGER_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Window Manager
//-----------------------------------------------
// .....................................................................
// This module allows to easily control multiple windows and their layering
// Windows are identified by either their pointer or by the name they were added with
// Note: the draw() function to draw the windows needs to be called MANUALLY - windows should then not be drawn individually!
// .....................................................................

namespace magique
{
    // Returns the global instance of the window manager
    WindowManager& GetWindowManager();

    struct WindowManager final
    {
        // Draws all managed windows that are visible
        void draw();

        // Adds a new window to the window manager - optionally register it with a name
        // Windows can be accessed in the order they are added as well (use getWindows())
        void addWindow(Window* window, const char* name);

        //----------------- ACCESSORS -----------------//

        // Returns a managed window by name or index
        // Failure: returns nullptr if no window was added with that name
        Window* getWindow(const char* name);
        Window* getWindow(int index);

        // Returns true if the window was successfully removed
        // Failure: returns false if no window matched the identifier
        bool removeWindow(Window* window);
        bool removeWindow(const char* name);

        // Sets the shown status of the given window to the given value
        void setShown(Window* window, bool shown);
        void setShown(const char* name, bool shown);

        // Returns the shown status of the given window
        bool getIsShown(Window* window);
        bool getIsShown(const char* name);

        // Toggles the shown status of the given window
        void toggleShown(Window* window);
        void toggleShown(const char* name);

        // Returns a vector that contains all managed windows
        const std::vector<Window*>& getWindows();

        //----------------- LAYERING -----------------//

        // Moves the first window in front of the second one
        void moveInFrontOf(Window* moved, Window* inFrontOf);
        void moveInFrontOf(const char* moved, const char* inFrontOf);

        //----------------- UTIL -----------------//



    private:
        WindowManager();
    };


} // namespace magique

#endif //MAGIQUE_WINDOW_MANAGER_H