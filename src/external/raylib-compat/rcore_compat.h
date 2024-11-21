#ifndef RCORE_COMPAT_H
#define RCORE_COMPAT_H


extern "C" {

int GetCharPressedQueueCount();

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

Matrix GetScreenScale();
}

#endif // RCORE_COMPAT_H