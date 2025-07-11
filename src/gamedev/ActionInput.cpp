#include <magique/gamedev/ActionInput.h>

namespace magique
{
    ActionInput ActionInput::FromKeybind(const Keybind& bind)
    {
        ActionInput action;
        action.isPressed = bind.isPressed();
        action.isDown = bind.isDown();
        action.isReleased = bind.isReleased();
        return action;
    }

    ActionInput ActionInput::FromKey(const KeyboardKey key)
    {
        ActionInput action;
        action.isPressed = IsKeyPressed(key);
        action.isDown = IsKeyDown(key);
        action.isReleased = IsKeyReleased(key);
        return action;
    }

    ActionInput ActionInput::FromKey(MouseButton button)
    {
        ActionInput action;
        action.isPressed = IsMouseButtonPressed(button);
        action.isDown = IsMouseButtonDown(button);
        action.isReleased = IsMouseButtonReleased(button);
        return action;
    }

    ActionInput ActionInput::FromVirtual(const bool pressed, const bool down, const bool released)
    {
        ActionInput action;
        action.isPressed = pressed;
        action.isDown = down;
        action.isReleased = released;
        return action;
    }

    bool ActionInput::getIsDown() const { return isDown; }

    bool ActionInput::getIsPressed() const { return isPressed; }

    bool ActionInput::getIsReleased() const { return isReleased; }
} // namespace magique