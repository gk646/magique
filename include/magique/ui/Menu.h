#ifndef MAGEQUEST_MENU_H
#define MAGEQUEST_MENU_H

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

    struct Menu : UIObject
    {

        // Util
        void addSubMenu(Menu* menu, const char* name);
        bool removeSubMenu(const char* name);

        void switchToSubmenu(const char* name);
        void switchToSubmenu(Menu* menu);
        void switchToParent();

        // Returns true if this menu is a top menu (no parents)
        bool isTop() const;

    protected:
        // A menu doesnt draw anything per default
        void onDraw(const Rectangle& bounds) override {}

        void onUpdate(const Rectangle& bounds, bool wasDrawn) override { updateInputs(); }

        // Updates the default inputs:
        //      - ESC: Switch to parent
        // Consumes mouse and key as nothing should come after it
        void updateInputs();

    private:
        Menu(Menu* parent, const char* name);
        Menu* parent = nullptr;
        struct SubMenu final
        {
            const char* name;
            Menu* menu;
        };
        std::vector<SubMenu> subMenus;
    };
} // namespace magique
#endif //MAGEQUEST_MENU_H
