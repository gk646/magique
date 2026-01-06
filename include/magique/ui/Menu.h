#ifndef MAGEQUEST_MENU_H
#define MAGEQUEST_MENU_H

#include "magique/ui/UIContainer.h"
#include <vector>
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
//  - Always covers the full screen
// .....................................................................

namespace magique
{
    MenuManager& GetMenuManager();

    struct MenuManager final
    {
        // Creates a new top-level menu out of the given instance with the give name
        // Returns the passed instance
        Menu& createMenu(Menu* menu, const char* name);
    };

    struct Menu : UIContainer
    {
        // Util
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
        // A menu doesnt draw anything per default
        void onDraw(const Rectangle& bounds) override {}

        void onUpdate(const Rectangle& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateInputs();
        }

        // Updates the default inputs:
        //      - ESC: Switch to parent
        // Consumes mouse and key as nothing should come after it
        void updateInputs();

    private:
        Menu* parent = nullptr;
        struct SubMenu final
        {
            const char* name;
            Menu* menu;
        };
        std::vector<SubMenu> subMenus;
        bool isActive = false;
    };
} // namespace magique
#endif //MAGEQUEST_MENU_H
