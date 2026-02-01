// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_INPUT_SYSTEM_H
#define MAGIQUE_INPUT_SYSTEM_H

namespace magique
{
    inline char emptyKeys[MAX_KEYBOARD_KEYS]{};
    inline char emptyButtons[MAX_MOUSE_BUTTONS]{};

    inline bool HasKeyEventHappened()
    {
        if (memcmp(GetCurrentKeyState(), emptyKeys, MAX_KEYBOARD_KEYS) != 0)
        {
            return true;
        }
        return memcmp(GetCurrentKeyState(), GetPreviousKeyState(), MAX_KEYBOARD_KEYS) != 0;
    }

    inline bool HasMouseEventHappened()
    {
        if (memcmp(GetCurrentButtonState(), emptyButtons, MAX_MOUSE_BUTTONS) != 0)
        {
            return true;
        }
        return memcmp(GetCurrentButtonState(), GetPreviousButtonState(), MAX_MOUSE_BUTTONS) != 0;
    }

    inline void InputSystem()
    {
        // Should be a subtle optimization
        // Do some initial checks to avoid calling event functions every tick

        const auto& data = global::ENGINE_DATA;
        const auto& config = global::ENGINE_CONFIG;

        if (config.isClientMode)
            return;

        const bool invokeKey =
            GetKeyPressedQueueCount() != 0 || GetCharPressedQueueCount() != 0 || HasKeyEventHappened();

        const bool invokeMouse = GetPreviousMousePositionX() != GetCurrentMousePositionX() ||
            GetPreviousMousePositionY() != GetCurrentMousePositionY() || HasMouseEventHappened();

        if (invokeKey && invokeMouse)
        {
            for (size_t i = 0; i < data.entityUpdateVec.size(); ++i)
            {
                const auto e = data.entityUpdateVec[i];
                if (data.isEntityScripted(e)) [[likely]]
                {
                    ScriptingInvokeEvent<onKeyEvent>(e);
                    ScriptingInvokeEvent<onMouseEvent>(e);
                }
            }
        }
        else if (invokeKey)
        {
            for (size_t i = 0; i < data.entityUpdateVec.size(); ++i)
            {
                const auto e = data.entityUpdateVec[i];
                if (data.isEntityScripted(e)) [[likely]]
                {
                    ScriptingInvokeEvent<onKeyEvent>(e);
                }
            }
        }
        else if (invokeMouse)
        {
            for (size_t i = 0; i < data.entityUpdateVec.size(); ++i)
            {
                const auto e = data.entityUpdateVec[i];
                if (data.isEntityScripted(e)) [[likely]]
                {
                    ScriptingInvokeEvent<onMouseEvent>(e);
                }
            }
        }
    }
} // namespace magique


#endif //MAGIQUE_INPUT_SYSTEM_H