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
        void addSubMenu(Menu* menu, std::string_view name);
        bool removeSubMenu(std::string_view name);

        // Sets the given submenu active
        bool activateSubmenu(std::string_view name);
        bool activateSubmenu(Menu* menu);

        // Splits the string at ":" and tries to call activateSubmenu for each entry with new submenu that is activated
        // Allows to switch into deeper menus from the top
        // Note: switchToNested("Play:Lobby") - first switches to Play menu, then switches to Lobby inside the Play menu
        void activateNested(std::string_view nestedMenu);

        // Gives control back to the parent
        void activateParent();

        // Sets this menu active - sets all children inactive
        void activate();

        // Returns true if this menu is a top menu (no parents)
        bool getIsTopLevel() const;

        // Returns true if this menu is currently active
        bool getIsActive() const;

    protected:
        // Needs to be called (as the first thing) in all implementing versions
        // After that you should return if the current menu is not active (getIsActive())
        void onDraw(const Rect& bounds) override;

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn && getIsActive())
                updateInputs();
        }

        void onDrawUpdate(const Rect& bounds, bool wasDrawn) override;

        // Updates the default inputs:
        //      - ESC: Switch to parent
        // Consumes mouse and key as nothing should come after it
        void updateInputs(KeyboardKey key = KEY_ESCAPE, GamepadButton button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);


        // Called when menu is active and exit button is pressed to switch to parent
        // If returns false action will be blocked
        virtual bool onExitRequest() { return true; }

    private:
        void inactivateChildren();
        Menu* parent = nullptr;
        Menu* subMenu = nullptr;
        bool isActive = true;
    };
} // namespace magique

#endif // MAGEQUEST_MENU_H
