#ifndef MAGEQUEST_MENU_H
#define MAGEQUEST_MENU_H

#include "magique/ui/UIContainer.h"
#include <magique/fwd.hpp>
#include <magique/ui/UIObject.h>

//===============================================
// Menu
//===============================================
// .....................................................................
// Menu is a class that helps to build nested menu screen:
//  - Main Menu
//  - Settings Menu
//  - ...
//
// A menu has certain properties
//  - Always covers the full screen (its bounds are UI target resolution)
//  - A menu can be active or not (if its drawn and the "shown" one)
//      - If a submenu (or a submenu of a submenu) is active the menu itself is NOT active

// Note: "getIsActive()" NEEDS to be checked when drawing and updating (as menu switching takes place in updateInputs()
//        Also the base functions need to be called
// .....................................................................

namespace magique
{

    struct Menu : UIContainer
    {
        Menu();

        // Note: Adds the menu as children to the UIContainer
        void addSubMenu(Menu* menu, const char* name);
        bool removeSubMenu(const char* name);

        // Sets the given submenu active
        bool switchToSubmenu(const char* name);
        bool switchToSubmenu(Menu* menu);
        // Iterates through the vector and tries to switch to the submenu at each level and repeat
        // Allows to switch into deeper menus from the top
        void switchToSubmenu(const std::initializer_list<std::string>& menus);

        // Gives control back to the parent
        void switchToParent();

        // Returns true if this menu is a top menu (no parents)
        bool getIsTopLevel() const;
        // Returns true if this menu is currently active
        bool getIsActive() const;

    protected:
        // Needs to be called (as the first thing) in all implementing versions
        // After that you should return if the current menu is not active (getIsActive())
        void onDraw(const Rectangle& bounds) override;

        void onUpdate(const Rectangle& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateInputs();
        }

        void onDrawUpdate(const Rectangle& bounds, bool wasDrawn) override;

        // Updates the default inputs:
        //      - ESC: Switch to parent
        // Consumes mouse and key as nothing should come after it
        void updateInputs();

    private:
        Menu* parent = nullptr;
        Menu* subMenu = nullptr;
        bool isActive = true;
    };
} // namespace magique

#endif // MAGEQUEST_MENU_H
