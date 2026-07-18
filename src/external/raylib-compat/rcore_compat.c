// SPDX-License-Identifier: zlib-acknowledgement
int GetCharPressedQueueCount() { return CORE.Input.Keyboard.charPressedQueueCount; }
int GetKeyPressedQueueCount() { return CORE.Input.Keyboard.keyPressedQueueCount; }

int* GetCharPressedQueue() { return CORE.Input.Keyboard.charPressedQueue; }

char* GetCurrentKeyState() { return CORE.Input.Keyboard.currentKeyState; }
char* GetPreviousKeyState() { return CORE.Input.Keyboard.previousKeyState; }

char* GetCurrentButtonState() { return CORE.Input.Mouse.currentButtonState; }
char* GetPreviousButtonState() { return CORE.Input.Mouse.previousButtonState; }

int GetFBOWidth() { return CORE.Window.currentFbo.width; }

int GetFBOHeight() { return CORE.Window.currentFbo.width; }

int GetCurrentMousePositionX() { return CORE.Input.Mouse.currentPosition.x; }
int GetCurrentMousePositionY() { return CORE.Input.Mouse.currentPosition.y; }

int GetPreviousMousePositionX() { return CORE.Input.Mouse.previousPosition.x; }
int GetPreviousMousePositionY() { return CORE.Input.Mouse.previousPosition.y; }


void SetMousePositionDirect(float x, float y)
{
    CORE.Input.Mouse.currentPosition.x = x;
    CORE.Input.Mouse.currentPosition.y = y;
}

char* GetCurrentGamepadState() { return CORE.Input.Gamepad.currentButtonState[0]; }

char* GetPreviousGamepadState() { return CORE.Input.Gamepad.previousButtonState[0]; }

void TriggerMousePress(int button)
{
    CORE.Input.Mouse.currentButtonState[button] = 1;
    CORE.Input.Mouse.previousButtonState[button] = 0;
}

void TriggerMouseRelease(int button)
{
    CORE.Input.Mouse.currentButtonState[button] = 0;
    CORE.Input.Mouse.previousButtonState[button] = 1;
}

Vector2 GetMouseScale()
{
    return CORE.Input.Mouse.scale;
}


RenderTexture GetCurrentRenderTexture()
{
    RenderTexture2D texture = {0};
    if (!CORE.Window.usingFbo)
        return texture;
    texture.texture.width = CORE.Window.currentFbo.width;
    texture.texture.height = CORE.Window.currentFbo.height;
    texture.id = rlGetActiveFramebuffer();
    return texture;
}

