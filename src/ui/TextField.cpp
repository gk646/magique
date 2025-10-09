// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>
#include <magique/ui/controls/TextField.h>
#include <magique/util/RayUtils.h>
#include <magique/ui/UI.h>
#include <magique/util/Math.h>

#include "external/raylib-compat/rcore_compat.h"
#include "internal/utils/CollisionPrimitives.h"
#include "internal/globals/EngineConfig.h"

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

    TextField::TextField(float w, float h, Anchor anchor, ScalingMode scaling) : UIObject(w, h, anchor, 0.0F, scaling) {}

    const std::string& TextField::getText() const { return text; }

    void TextField::setText(const char* newText)
    {
        MAGIQUE_ASSERT(newText != nullptr, "Passed null");
        text = newText;
    }

    void TextField::setHint(const char* newHint)
    {
        MAGIQUE_ASSERT(newHint != nullptr, "Passed null");
        hint = newHint;
    }

    bool TextField::pollTextHasChanged()
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

    bool TextField::getIsFocused() const { return isFocused; }

    int TextField::getLineCount() const { return lineCount; }

    static bool IsKeyPressedAnyWay(const int key) { return IsKeyPressed(key) || IsKeyPressedRepeat(key); }

    bool TextField::updateInputs()
    {
        ++blinkCounter;
        if (blinkCounter > 2 * blinkDelay)
            blinkCounter = 0;
        const auto bounds = getBounds();
        const auto mouse = GetMousePos();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isFocused = PointToRect(mouse.x, mouse.y, bounds.x, bounds.y, bounds.width, bounds.height);
        }

        if (!isFocused)
            return false;

        const auto anyEvent = updateControls();
        bool anyInput = false;
        // Iterate characters
        for (int i = 0; i < GetCharPressedQueueCount(); ++i)
        {
            const char pressedChar = static_cast<char>(GetCharPressedQueue()[i]);
            text.insert(text.begin() + cursorPos, pressedChar);
            ++cursorPos;
            anyInput = true;
        }
        return anyInput || anyEvent;
    }

    void TextField::drawText(const float fontSize, const Color color, const Color cursor, const Font& font,
                             const float spacing) const
    {
        const auto bounds = getBounds();
        const Vector2 tPos = {bounds.x + 2, bounds.y + (bounds.height - fontSize) / 2.0F};

        if (!isFocused && text.empty() && hint != nullptr)
        {
            DrawTextEx(font, hint, tPos, fontSize, spacing, ColorAlpha(color, 0.65));
        }
        else
        {
            DrawTextEx(font, text.c_str(), tPos, fontSize, spacing, color);
        }

        if (!isFocused || blinkCounter > blinkDelay)
        {
            return;
        }

        const auto* lineStart = text.data() + currLineStart;
        const auto cursorOffX = MeasureTextUpTo(lineStart, cursorPos - currLineStart, font, fontSize, spacing);
        const auto cursorOffY = (fontSize + static_cast<float>(GetTextLineSpacing())) * static_cast<float>(cursorLine);
        DrawTextEx(font, "|", {tPos.x + cursorOffX, tPos.y + cursorOffY}, fontSize, spacing, cursor);
    }

    void TextField::drawDefault(const Rectangle& bounds) const
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const Color body = getIsFocused() ? theme.backSelected : getIsHovered() ? theme.backLight : theme.backDark;
        const Color outline = theme.backDark;
        DrawRectangleRounded(bounds, 0.1F, 20, body);
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 20, 2, outline);
        drawText(14, getIsFocused() ? theme.textActive : theme.textPassive, theme.textPassive);
    }

    bool TextField::updateControls()
    {
        // Finds the current line start
        auto findLine = [](std::string& str, const int cursorPos, int& currLineStart, int& currLine, int& lineCount)
        {
            currLineStart = 0;
            currLine = 0;
            lineCount = 0;
            const int size = static_cast<int>(str.size());
            for (int i = 0; i < size; ++i)
            {
                if (str[i] == '\n')
                {
                    if (i < cursorPos)
                    {
                        ++currLine;
                        currLineStart = i;
                    }
                    ++lineCount;
                }
            }
        };

        // Clamp as safety at the beginning
        cursorPos = clamp(cursorPos, 0, static_cast<int>(text.size()));

        // Parses controls and returns true if cursor or text have changed
        auto parseControls = [](std::string& str, int& pos)
        {
            const int size = static_cast<int>(str.size());

            if (IsKeyPressedAnyWay(MOVE_LEFT))
            {
                if (IsKeyDown(KEY_LEFT_CONTROL))
                {
                    if (pos - 1 < 0)
                        return false;
                    if (isspace(str[pos - 1]))
                    {
                        --pos;
                        return true;
                    }
                    for (int i = pos - 1; i > -1; --i)
                    {
                        if (isspace(str[i]))
                        {
                            pos = i + 1;
                            return true;
                        }
                        if (i == 0)
                        {
                            pos = 0;
                            return true;
                        }
                    }
                }
                else if (pos > 0)
                {
                    --pos;
                    return true;
                }
                return false;
            }

            if (IsKeyPressedAnyWay(MOVE_RIGHT))
            {
                if (IsKeyDown(KEY_LEFT_CONTROL))
                {
                    if (isspace(str[pos]))
                    {
                        ++pos;
                        return true;
                    }
                    for (int i = pos; i < size; ++i)
                    {
                        if (isspace(str[i]))
                        {
                            pos = i;
                            return true;
                        }

                        if (i == size - 1)
                        {
                            pos = i + 1;
                            return true;
                        }
                    }
                }
                else if (pos < size)
                {
                    ++pos;
                    return true;
                }
                return false;
            }

            if (IsKeyPressedAnyWay(MOVE_UP))
            {
                if (pos - 1 < 0)
                    return false;
                for (int i = pos - 1; i > -1; --i)
                {
                    if (str[i] == '\n')
                    {
                        pos = i;
                        return true;
                    }
                }
                return false;
            }

            if (IsKeyPressedAnyWay(MOVE_DOWN))
            {
                for (int i = pos; i < size; ++i)
                {
                    if (str[i] == '\n')
                    {
                        pos = i + 1;
                        return true;
                    }
                }
                return false;
            }

            if (str.empty())
                return false;

            if (IsKeyPressedAnyWay(BACKSPACE))
            {
                if (pos == 0)
                    return false;
                str.erase(pos - 1, 1);
                --pos;
                return true;
            }

            if (IsKeyPressedAnyWay(DELETE))
            {
                if (pos == size)
                    return false;
                str.erase(pos, 1);
                return true;
            }
            return false;
        };

        if (parseControls(text, cursorPos))
        {
            textChanged = true;
            findLine(text, cursorPos, currLineStart, cursorLine, lineCount);
            return true;
        }

        if (IsKeyPressedAnyWay(KEY_ENTER))
        {
            if (!onEnterPressed(IsKeyDown(KEY_LEFT_CONTROL), IsKeyDown(KEY_LEFT_SHIFT), IsKeyDown(KEY_LEFT_ALT)))
            {
                text.insert(text.begin() + cursorPos, '\n');
                textChanged = true;
                ++cursorPos;
                findLine(text, cursorPos, currLineStart, cursorLine, lineCount);
                return true;
            }
        }
        return false;
    }


} // namespace magique