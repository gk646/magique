#include <magique/gamedev/InputAction.h>

namespace magique
{
    InputAction InputAction::FromKeybind(const Keybind& bind)
    {
        InputAction action;
        action.isPressed = bind.isPressed();
        action.isDown = bind.isDown();
        action.isReleased = bind.isReleased();
        return action;
    }

    InputAction InputAction::FromKey(const KeyboardKey key)
    {
        InputAction action;
        action.isPressed = IsKeyPressed(key);
        action.isDown = IsKeyDown(key);
        action.isReleased = IsKeyReleased(key);
        return action;
    }

    InputAction InputAction::FromKey(MouseButton button)
    {
        InputAction action;
        action.isPressed = IsMouseButtonPressed(button);
        action.isDown = IsMouseButtonDown(button);
        action.isReleased = IsMouseButtonReleased(button);
        return action;
    }

    InputAction InputAction::FromVirtual(const bool pressed, const bool down, const bool released)
    {
        InputAction action;
        action.isPressed = pressed;
        action.isDown = down;
        action.isReleased = released;
        return action;
    }

    bool InputAction::getIsDown() const { return isDown; }

    bool InputAction::getIsPressed() const { return isPressed; }

    bool InputAction::getIsReleased() const { return isReleased; }
} // namespace magique