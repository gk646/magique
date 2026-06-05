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

    Menu* Menu::getSubMenu(std::string_view name) const
    {
        for (auto& [child, ptr] : getChildren())
        {
            if (child == name)
                return (Menu*)ptr;
        }
        return nullptr;
    }

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

    void Menu::activateNested(std::string_view search)
    {
        if (activateSubmenu(search))
            return;

        for (auto& [name, ptr] : getChildren())
        {
            ptr->getAs<Menu>()->activateNested(search);
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
        if (getGamepadMapping() != nullptr)
            UISetGamepadMap(getGamepadMapping());

        inactivateChildren();
        isActive = true;
        subMenu = nullptr;

        Menu* parentPtr = parent;
        Menu* subMenuPtr = this;
        while (parentPtr != nullptr)
        {
            parentPtr->subMenu = subMenuPtr;
            parentPtr->isActive = false;

            subMenuPtr = parentPtr;
            parentPtr = parentPtr->parent;
        }
    }

    void Menu::disable() { isActive = false; }

    bool Menu::getIsTopLevel() const { return parent == nullptr; }

    bool Menu::getIsActive() const { return isActive; }

    bool Menu::getIsSubmenuActive() const { return subMenu != nullptr; }

    Menu* Menu::getParent() const { return parent; }

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
