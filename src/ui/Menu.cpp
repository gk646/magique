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

    bool Menu::switchToSubmenu(const char* name)
    {
        auto* child = (Menu*)getChild(name);
        return switchToSubmenu(child);
    }

    bool Menu::switchToSubmenu(Menu* menu)
    {
        if (menu == nullptr)
        {
            return false;
        }
        for (auto& [name, child] : getChildren())
        {
            if (child == menu)
            {
                subMenu = (Menu*)child;
                subMenu->isActive = true;
                isActive = false;
                return true;
            }
        }
        return false;
    }

    void Menu::switchToSubmenu(const std::initializer_list<std::string>& menus)
    {
        Menu* menu = this;
        for (auto& child : menus)
        {
            if (!menu->switchToSubmenu(child.c_str()))
            {
                break;
            }else
            {
                menu = menu->getChild(child.c_str())->getAs<Menu>();
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

    void Menu::onDraw(const Rectangle& bounds)
    {
        if (subMenu != nullptr)
        {
            subMenu->draw();
        }
    }

    void Menu::onDrawUpdate(const Rectangle& bounds, bool wasDrawn)
    {
        UIContainer::onDrawUpdate(bounds, wasDrawn);
        setSize(UIGetTargetResolution());
    }

    void Menu::updateInputs()
    {
        if (LayeredInput::IsKeyPressed(KEY_ESCAPE))
        {
            switchToParent();
            LayeredInput::ConsumeKey();
        }
    }

} // namespace magique
