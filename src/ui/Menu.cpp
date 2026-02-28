#include <magique/ui/Menu.h>
#include <magique/util/Strings.h>

#include "magique/ui/UI.h"
#include "raylib/raylib.h"

namespace magique
{
    Menu::Menu() : UIContainer(Rect{}) {}

    Menu* Menu::addSubMenu(Menu* menu, std::string_view name)
    {
        if (menu != nullptr)
        {
            menu->parent = this;
            addChild(menu, name);
            return menu;
        }
        return nullptr;
    }

    bool Menu::removeSubMenu(std::string_view name) { return removeChild(name); }

    bool Menu::activateSubmenu(std::string_view name)
    {
        auto* child = static_cast<Menu*>(getChild(name));
        return activateSubmenu(child);
    }

    bool Menu::activateSubmenu(Menu* menu)
    {
        if (menu == nullptr)
        {
            return false;
        }
        for (auto& [name, child] : getChildren())
        {
            if (child == menu)
            {
                subMenu = static_cast<Menu*>(child);
                isActive = false;
                subMenu->activate();
                return true;
            }
        }
        return false;
    }

    void Menu::activateNested(std::string_view nestedMenu)
    {
        auto parts = StringSplit(nestedMenu, ':');
        Menu* menu = this;
        for (auto& child : parts)
        {
            if (!menu->activateSubmenu(child))
            {
                break;
            }
            else
            {
                menu = menu->getChild(child)->getAs<Menu>();
                menu->inactivateChildren();
            }
        }
    }


    void Menu::activateParent() const
    {
        if (parent != nullptr)
        {
            parent->activate();
        }
    }

    void Menu::activate()
    {
        inactivateChildren();
        isActive = true;
        subMenu = nullptr;
    }

    void Menu::disable() { isActive = false; }

    bool Menu::getIsTopLevel() const { return parent == nullptr; }

    bool Menu::getIsActive() const { return isActive; }

    void Menu::onDraw(const Rect& bounds)
    {
        if (subMenu != nullptr)
        {
            subMenu->draw();
        }
    }

    void Menu::onDrawUpdate(const Rect& bounds, bool wasDrawn)
    {
        UIContainer::onDrawUpdate(bounds, wasDrawn);
        setSize(UIGetTargetResolution());
    }

    void Menu::updateInputs(KeyboardKey key, GamepadButton button)
    {
        if (LayeredInput::IsKeyPressed(key) || LayeredInput::IsGamepadButtonPressed(0, button))
        {
            if (onExitRequest())
            {
                activateParent();
            }
            LayeredInput::ConsumeKey();
        }
    }

    void Menu::inactivateChildren()
    {
        for (auto& [name, child] : getChildren())
        {
            child->getAs<Menu>()->subMenu = nullptr;
            child->getAs<Menu>()->isActive = false;
            child->getAs<Menu>()->inactivateChildren();
        }
    }


} // namespace magique
