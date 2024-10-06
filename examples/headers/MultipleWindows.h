#ifndef MAGIQUE_MULTIPLE_WINDOWS_H
#define MAGIQUE_MULTIPLE_WINDOWS_H

#include <magique/core/Game.h>
#include <magique/ui/WindowManager.h>
#include <magique/ui/controls/Window.h>
#include <magique/util/RayUtils.h>

//-----------------------------------------------
// Multiple Windows Example
//-----------------------------------------------
// .....................................................................
// This example showcases how to manage multiple windows, specifically:
//      - Create them
//      - Add window buttons
//      - Move them around / show and hide them
//      - Change their layer
// .....................................................................

using namespace magique;

// Draws the window - method for all windows to avoid duplication
void DrawWindow(const Window& window, Color color)
{
    const auto topBar = window.getTopBarBounds();
    if (CheckCollisionPointRec(GetMousePosition(), topBar))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) // Pressed
        {
            DrawRectangleRounded(topBar, 0.2F, 30, DARKGRAY);
            DrawRectangleRoundedLinesEx(topBar, 0.2F, 30, 2, GRAY);
        }
        else // Hovered
        {
            DrawRectangleRounded(topBar, 0.2F, 30, GRAY);
            DrawRectangleRoundedLinesEx(topBar, 0.2F, 30, 2, DARKGRAY);
        }
    }
    else // Idle
    {
        DrawRectangleRounded(topBar, 0.2F, 30, color);
        DrawRectangleRoundedLinesEx(topBar, 0.2F, 30, 2, GRAY);
    }

    const auto body = window.getBodyBounds();
    DrawRectangleRounded(body, 0.2F, 30, color);
    DrawRectangleRoundedLinesEx(body, 0.2F, 30, 2, GRAY);
}

struct BlueWindow final : Window
{
    BlueWindow(float x, float y, float w, float h) : Window(x, y, w, h, 50) {}
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, BLUE); }
};

struct RedWindow final : Window
{
    RedWindow(float x, float y, float w, float h) : Window(x, y, w, h, 50) {}
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, RED); }
};

struct PurpleWindow final : Window
{
    PurpleWindow(float x, float y, float w, float h) : Window(x, y, w, h, 50) {}
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, PURPLE); }
};

struct GreenWindow final : Window
{
    GreenWindow(float x, float y, float w, float h) : Window(x, y, w, h, 50) {}
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, GREEN); }
};

struct Test final : Game
{
    void onLoadingFinished() override
    {
        float width = 500;
        float height = 600;
        auto& wManager = GetWindowManager();
        Window* newWindow = new BlueWindow(GetRandomFloat(0, 1000), GetRandomFloat(0, 500), width, height);
        wManager.addWindow(newWindow, "BlueWindow");
        newWindow = new RedWindow(GetRandomFloat(0, 1000), GetRandomFloat(0, 500), width, height);
        wManager.addWindow(newWindow, "RedWindow");
        newWindow = new PurpleWindow(GetRandomFloat(0, 1000), GetRandomFloat(0, 500), width, height);
        wManager.addWindow(newWindow, "PurpleWindow");
        newWindow = new GreenWindow(GetRandomFloat(0, 1000), GetRandomFloat(0, 500), width, height);
        wManager.addWindow(newWindow, "GreenWindow");
    }

    void updateGame(GameState gameState) override
    {

    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        GetWindowManager().draw();
    }
};

#endif //MAGIQUE_MULTIPLE_WINDOWS_H