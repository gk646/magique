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
inline void DrawWindow(Window& window, Color color)
{
    const auto body = window.getBounds();
    DrawRectangleRounded(body, 0.1F, 30, color);
    DrawRectangleRoundedLinesEx(body, 0.1F, 30, 2, GRAY);

    const auto topBar = window.getTopBarBounds();
    // Check if the window is covered at the mouse position
    if (CheckCollisionPointRec(GetMousePos().v(), topBar) && !GetWindowManager().getIsCovered(&window))
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) // Pressed
        {
            DrawRectangleRounded(topBar, 0.2F, 30, DARKGRAY);
            DrawRectangleRoundedLinesEx(topBar, 0.1F, 30, 2, GRAY);
        }
        else // Hovered
        {
            DrawRectangleRounded(topBar, 0.1F, 30, GRAY);
            DrawRectangleRoundedLinesEx(topBar, 0.1F, 30, 2, DARKGRAY);
        }
    }
    else // Idle
    {
        DrawRectangleRounded(topBar, 0.1F, 30, color);
        DrawRectangleRoundedLinesEx(topBar, 0.1F, 30, 2, GRAY);
    }

    for (auto* child : window.getChildren())
    {
        child->align(Anchor::TOP_RIGHT, window);
        child->draw();
    }
}

// Update windows - method for all windows to avoid duplication
inline void UpdateWindow(Window* window)
{
    if (!GetWindowManager().getIsCovered(window)) // Check if the window is covered at the mouse position
    {
        auto* closeButton = window->getChild("CloseButton");
        if (closeButton->getIsClicked())
        {
            GetWindowManager().setShown(window, false);
            UIInput::Consume();
        }

        const bool res = window->updateDrag(window->getTopBarBounds()); // Returns true if is dragged
        if (res || UIInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && window->getIsHovered())
        {
            GetWindowManager().makeTopMost(window);
            UIInput::Consume(); // Consume input if clicked or dragged
        }
    }
}

constexpr float topBarHeight = 25;

struct CloseButton final : UIObject
{
    CloseButton() : UIObject(topBarHeight, topBarHeight) {}

    void onDraw(const Rectangle& bounds) override
    {
        const Vector2 topLeft = {bounds.x + 1, bounds.y + 1};
        const Vector2 bottomRight = {bounds.x + bounds.width - 1, bounds.y + bounds.height - 1};
        const Vector2 bottomLeft = {bounds.x, bounds.y + bounds.height - 1};
        const Vector2 topRight = {bounds.x + bounds.width - 1, bounds.y + 1};
        DrawLineEx(topLeft, bottomRight, 1, GRAY);
        DrawLineEx(bottomLeft, topRight, 1, GRAY);
        DrawRectangleLinesEx(bounds, 1, DARKGRAY);
    }
};

struct BlueWindow final : Window
{
    BlueWindow(float x, float y, float w, float h) : Window(x, y, w, h, topBarHeight)
    {
        addChild(new CloseButton(), "CloseButton");
    }
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, BLUE); }
    void onDrawUpdate(const Rectangle& bounds) override { UpdateWindow(this); }
};

struct RedWindow final : Window
{
    RedWindow(float x, float y, float w, float h) : Window(x, y, w, h, topBarHeight)
    {
        addChild(new CloseButton(), "CloseButton");
    }
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, RED); }
    void onDrawUpdate(const Rectangle& bounds) override { UpdateWindow(this); }
};

struct PurpleWindow final : Window
{
    PurpleWindow(float x, float y, float w, float h) : Window(x, y, w, h, topBarHeight)
    {
        addChild(new CloseButton(), "CloseButton");
    }
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, PURPLE); }
    void onDrawUpdate(const Rectangle& bounds) override { UpdateWindow(this); }
};

struct GreenWindow final : Window
{
    GreenWindow(float x, float y, float w, float h) : Window(x, y, w, h, topBarHeight)
    {
        addChild(new CloseButton(), "CloseButton");
    }
    void onDraw(const Rectangle& bounds) override { DrawWindow(*this, GREEN); }
    void onDrawUpdate(const Rectangle& bounds) override { UpdateWindow(this); }
};

struct Example final : Game
{
    Example() : Game("magique - Example: Multiple Windows") {}
    void onLoadingFinished() override
    {
        float width = 250;
        float height = 350;
        auto& wManager = GetWindowManager();
        Window* newWindow = new BlueWindow(GetRandomFloat(0, 555), GetRandomFloat(0, 555), width, height);
        wManager.addWindow(newWindow, "BlueWindow");
        newWindow = new RedWindow(GetRandomFloat(0, 555), GetRandomFloat(0, 555), width, height);
        wManager.addWindow(newWindow, "RedWindow");
        newWindow = new PurpleWindow(GetRandomFloat(0, 555), GetRandomFloat(0, 555), width, height);
        wManager.addWindow(newWindow, "PurpleWindow");
        newWindow = new GreenWindow(GetRandomFloat(0, 555), GetRandomFloat(0, 555), width, height);
        wManager.addWindow(newWindow, "GreenWindow");
    }

    void updateGame(GameState gameState) override
    {
        auto wManager = GetWindowManager();
        if (IsKeyPressed(KEY_SPACE))
        {
            for (auto* window : wManager.getWindows())
            {
                wManager.setShown(window, true);
            }
        }
        if (IsKeyPressed(KEY_ESCAPE))
        {
            for (auto* window : wManager.getWindows())
            {
                wManager.setShown(window, false);
            }
        }
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        GetWindowManager().draw();
        DrawText("Press SPACE to open all windows", 50, 50, 25, BLACK);
        DrawText("Press ESC to close all windows", 50, 100, 25, BLACK);
    }
};

#endif //MAGIQUE_MULTIPLE_WINDOWS_H