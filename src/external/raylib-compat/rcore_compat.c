int GetCharPressedQueueCount() { return CORE.Input.Keyboard.charPressedQueueCount; }

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

int GetTextLineSpacing() { return 2; }


Matrix GetScreenScale() { return CORE.Window.screenScale; }