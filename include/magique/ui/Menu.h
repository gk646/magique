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

// Note: "getIsActive()" NEEDS to be check when drawing and updating (as menu switching takes place in updateInputs()
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

        void switchToSubmenu(const char* name);
        void switchToSubmenu(Menu* menu);
        void switchToParent();

        // Returns true if this menu is a top menu (no parents)
        bool getIsTopLevel() const;
        // Returns true if this menu is currently active
        bool getIsActive() const;

    protected:
        void onDraw(const Rectangle& bounds) override
        {
            if (subMenu != nullptr)
            {
                subMenu->draw();
            }
        }

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

#endif //MAGEQUEST_MENU_H
