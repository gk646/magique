#ifndef MAGIQUE_INPUTSYSTEM_H
#define MAGIQUE_INPUTSYSTEM_H

#include <magique/ecs/InternalScripting.h>

#include "external/raylib/src/coredata.h"

namespace magique
{

    inline bool IsAnyKeyDown()
    {
        for (const char i : CORE.Input.Keyboard.previousKeyState)
        {
            if (i == 1)
            {
                return true; // At least one key is down
            }
        }

        for (const char i : CORE.Input.Keyboard.currentKeyState)
        {
            if (i == 1)
            {
                return true; // At least one key is down
            }
        }

        return false; // No keys are down
    }

    inline bool IsAnyMouseDown()
    {
        for (const char i : CORE.Input.Mouse.previousButtonState)
        {
            if (i != 0)
            {
                return true; // At least one key is down
            }
        }
        return false; // No key
    }

    inline void PollInputs(entt::registry& registry)
    {
        // Should be a subtle optimization
        // Do some initial checks to avoid calling event fun

        const bool invokeKey = GetKeyPressed() != 0 || GetCharPressed() != 0 || IsAnyKeyDown();

        const bool invokeMouse = CORE.Input.Mouse.previousPosition.x != CORE.Input.Mouse.currentPosition.x ||
            CORE.Input.Mouse.previousPosition.y != CORE.Input.Mouse.currentPosition.y || IsAnyMouseDown();

        if (invokeKey && invokeMouse)
        {
            const auto view = registry.view<ScriptC>();
            for (const auto e : view)
            {
                InvokeEvent<onKeyEvent>(e);
                InvokeEvent<onMouseEvent>(e);
            }
        }
        else if (invokeKey)
        {
            const auto view = registry.view<ScriptC>();
            for (const auto e : view)
            {
                InvokeEvent<onKeyEvent>(e);
            }
        }
        else if (invokeMouse)
        {
            const auto view = registry.view<ScriptC>();
            for (const auto e : view)
            {
                InvokeEvent<onMouseEvent>(e);
            }
        }
    }
} // namespace magique


#endif //MAGIQUE_INPUTSYSTEM_H