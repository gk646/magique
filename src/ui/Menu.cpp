#include <magique/ui/Menu.h>

#include "magique/ui/UI.h"
#include "raylib/raylib.h"

namespace magique
{
    Menu::Menu() : UIContainer(0, 0) {}

    void Menu::addSubMenu(Menu* menu, const char* name)
    {
        if (menu != nullptr)
        {
            menu->parent = this;
            addChild(menu, name);
        }
    }

    bool Menu::removeSubMenu(const char* name) { return removeChild(name); }

    void Menu::switchToSubmenu(const char* name)
    {
        auto* child = (Menu*)getChild(name);
        switchToSubmenu(child);
    }

    void Menu::switchToSubmenu(Menu* menu)
    {
        if (menu == nullptr)
        {
            return;
        }
        for (auto* child : getChildren())
        {
            if (child == menu)
            {
                subMenu = (Menu*)child;
                subMenu->isActive = true;
                isActive = false;
                return;
            }
        }
    }

    void Menu::switchToParent()
    {
        if (parent != nullptr)
        {
            isActive = false;
            subMenu = nullptr;
            parent->subMenu = nullptr;
            parent->isActive = true;
        }
    }

    bool Menu::getIsTopLevel() const { return parent == nullptr; }

    bool Menu::getIsActive() const { return isActive; }

    void Menu::onDrawUpdate(const Rectangle& bounds, bool wasDrawn)
    {
        UIContainer::onDrawUpdate(bounds, wasDrawn);
        setSize(UIGetTargetResolution());
    }

    void Menu::updateInputs()
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            switchToParent();
        }
    }

} // namespace magique
