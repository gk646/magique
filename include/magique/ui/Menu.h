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
//  - Always full screen
//      -
// .....................................................................

namespace magique
{

    struct MenuManager final
    {
        // Returns the top menu object for this state
        // If not present an empty one will be created
        Menu& getMenu(GameState state);
    };

    struct Menu : UIObject
    {

        void addSubMenu(Menu* menu, const char* name);

        // Returns true if this menu is a top menu (no parents)
        bool isTop() const;

    private:
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
