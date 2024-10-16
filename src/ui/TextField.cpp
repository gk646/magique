#include <raylib/raylib.h>

#include <magique/ui/controls/TextField.h>

#include "external/raylib/src/coredata.h"

namespace magique
{

    // Keybindings to avoid hardcoding
    static constexpr int MOVE_LEFT = KEY_LEFT;
    static constexpr int MOVE_RIGHT = KEY_RIGHT;
    static constexpr int MOVE_UP = KEY_UP;
    static constexpr int MOVE_DOWN = KEY_DOWN;
    static constexpr int BACKSPACE = KEY_BACKSPACE;
    static constexpr int DELETE = KEY_DELETE;

    TextField::TextField(float x, float y, float w, float h, ScalingMode scaling) : UIObject(x, y, w, h, scaling) {}

    TextField::TextField(float w, float h, Anchor anchor, ScalingMode scaling) : UIObject(w, h, anchor, scaling) {}

    const char* TextField::getCText() const { return text.c_str(); }

    const std::string& TextField::getText() const { return text; }

    bool TextField::getHasTextChanged()
    {
        const bool val = textChanged;
        textChanged = false;
        return val;
    }

    void TextField::setCursorStatus(const bool shown, const int delay)
    {
        showCursor = shown;
        blinkDelay = delay;
    }

    bool IsKeyPressedAnyWay(const int key) { return IsKeyPressed(key) || IsKeyPressedRepeat(key); }

    void TextField::updateInputs()
    {
        auto delChar = [](std::string& str, int& pos, bool infront) // Handles deleting upfront and behind
        {
            if (str.empty())
                return;

            if (infront)
            {
                if (pos == 0)
                    return;
                str.erase(pos - 1);
            }
            else
            {
                if (pos == static_cast<int>(str.size()))
                    return;
                str.erase(pos + 1);
            }
        };

        auto moveLineHorizontal = [](std::string& str, int& pos, const bool left)
        {
            if (left)
            {
                if (pos < static_cast<int>(str.size()))
                {
                    ++pos;
                }
            }
            else
            {
                if (pos > 0)
                    --pos;
            }
        };

        auto moveLineVertical = [](std::string& str, int& pos, bool up) {

        };

        if (IsKeyPressedAnyWay(MOVE_LEFT))
        {
            moveLineHorizontal(text, cursorPos, true);
            return;
        }
        if (IsKeyPressedAnyWay(MOVE_RIGHT))
        {
            moveLineHorizontal(text, cursorPos, false);
            return;
        }
        if (IsKeyPressedAnyWay(MOVE_UP))
        {
            moveLineHorizontal(text, cursorPos, true);
            return;
        }
        if (IsKeyPressedAnyWay(MOVE_DOWN))
        {
            moveLineHorizontal(text, cursorPos, true);
            return;
        }

        if (IsKeyPressedAnyWay(BACKSPACE))
        {
            delChar(text, cursorPos, true);
            return;
        }
        if (IsKeyPressedAnyWay(DELETE))
        {
            delChar(text, cursorPos, false);
            return;
        }

        // Iterate characters
        for (int i = 0; i < CORE.Input.Keyboard.charPressedQueueCount; ++i)
        {
            const char pressedChar = static_cast<char>(CORE.Input.Keyboard.charPressedQueue[i]);
            text.insert(text.begin() + cursorPos, pressedChar);
        }
    }

    void TextField::scaleSizeWithText(float fontSize, bool scaleHorizontal, bool scaleVertical) {}

    void TextField::drawDefault(const Rectangle& bounds)
    {
        DrawRectangleLinesEx(bounds, 2, GRAY);
        DrawRectangleRec(bounds, LIGHTGRAY);
        DrawText(getCText(), bounds.x, bounds.y, 15, BLACK);
    }

} // namespace magique