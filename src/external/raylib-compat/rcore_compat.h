// SPDX-License-Identifier: zlib-acknowledgement
#ifndef RCORE_COMPAT_H
#define RCORE_COMPAT_H

extern "C"
{
    int GetCharPressedQueueCount();
    int GetKeyPressedQueueCount();

    int* GetCharPressedQueue();

    char* GetCurrentKeyState();
    char* GetPreviousKeyState();

    char* GetCurrentButtonState();
    char* GetPreviousButtonState();


    int GetCurrentMousePositionX();
    int GetCurrentMousePositionY();

    int GetPreviousMousePositionX();
    int GetPreviousMousePositionY();

    int GetFBOWidth();
    int GetFBOHeight();

    int GetTextLineSpacing();

    void SetMousePositionDirect(float x, float y);

    char* GetCurrentGamepadState();
    char* GetPreviousGamepadState();
}

#endif // RCORE_COMPAT_H
