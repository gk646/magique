#ifndef MAGIQUE_INPUTSYSTEM_H
#define MAGIQUE_INPUTSYSTEM_H

namespace magique
{
    inline char emptyKeys[MAX_KEYBOARD_KEYS]{};
    inline char emptyButtons[MAX_MOUSE_BUTTONS]{};

    inline bool HasKeyEventHappened()
    {
        if (memcmp(CORE.Input.Keyboard.currentKeyState, emptyKeys, MAX_KEYBOARD_KEYS) != 0)
        {
            return true;
        }
        return memcmp(CORE.Input.Keyboard.currentKeyState, CORE.Input.Keyboard.previousKeyState, MAX_KEYBOARD_KEYS) != 0;
    }

    inline bool HasMouseEventHappened()
    {
        if (memcmp(CORE.Input.Mouse.currentButtonState, emptyButtons, MAX_MOUSE_BUTTONS) != 0)
        {
            return true;
        }
        return memcmp(CORE.Input.Mouse.currentButtonState, CORE.Input.Mouse.previousButtonState, MAX_MOUSE_BUTTONS) != 0;
    }

    inline void InputSystem(const entt::registry& registry)
    {
        // Should be a subtle optimization
        // Do some initial checks to avoid calling event functions every tick

        const bool invokeKey = GetKeyPressed() != 0 || GetCharPressed() != 0 || HasKeyEventHappened();

        const bool invokeMouse = CORE.Input.Mouse.previousPosition.x != CORE.Input.Mouse.currentPosition.x ||
            CORE.Input.Mouse.previousPosition.y != CORE.Input.Mouse.currentPosition.y || HasMouseEventHappened();

        if (invokeKey && invokeMouse)
        {
            for (const auto e : global::ENGINE_DATA.entityUpdateVec)
            {
                if (registry.all_of<ScriptC>(e)) [[likely]]
                {
                    InvokeEvent<onKeyEvent>(e);
                    InvokeEvent<onMouseEvent>(e);
                }
            }
        }
        else if (invokeKey)
        {
            for (const auto e : global::ENGINE_DATA.entityUpdateVec)
            {
                if (registry.all_of<ScriptC>(e)) [[likely]]
                {
                    InvokeEvent<onKeyEvent>(e);
                }
            }
        }
        else if (invokeMouse)
        {
            for (const auto e : global::ENGINE_DATA.entityUpdateVec)
            {
                if (registry.all_of<ScriptC>(e)) [[likely]]
                {
                    InvokeEvent<onMouseEvent>(e);
                }
            }
        }
    }
} // namespace magique


#endif //MAGIQUE_INPUTSYSTEM_H