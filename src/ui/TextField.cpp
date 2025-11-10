// SPDX-License-Identifier: zlib-acknowledgement
#include <cstring>

#include <raylib/raylib.h>
#include <magique/ui/controls/TextField.h>
#include <magique/util/RayUtils.h>
#include <magique/ui/UI.h>
#include <magique/util/Math.h>

#include "external/raylib-compat/rcore_compat.h"
#include "internal/utils/CollisionPrimitives.h"
#include "internal/globals/EngineConfig.h"
#include "magique/core/Draw.h"

namespace magique
{
    // Keybindings to avoid hardcoding
    static constexpr int MOVE_LEFT = KEY_LEFT;
    static constexpr int MOVE_RIGHT = KEY_RIGHT;
    static constexpr int MOVE_UP = KEY_UP;
    static constexpr int MOVE_DOWN = KEY_DOWN;
    static constexpr int BACKSPACE = KEY_BACKSPACE;
    static constexpr int DELETE = KEY_DELETE;
    static constexpr int PASTE = KEY_V;
    static constexpr int COPY = KEY_C;
    static constexpr int SELECT = KEY_A;
    static constexpr int CUT = KEY_X;

    TextField::TextField(float x, float y, float w, float h, ScalingMode scaling) : UIObject(x, y, w, h, scaling) {}

    TextField::TextField(float w, float h, Anchor anchor, ScalingMode scaling) : UIObject(w, h, anchor, 0.0F, scaling) {}

    std::string& TextField::getText() { return text; }

    void TextField::setText(const char* newText)
    {
        MAGIQUE_ASSERT(newText != nullptr, "Passed null");
        text = newText;
    }

    void TextField::setHint(const char* newHint)
    {
        if (newHint != nullptr)
        {
            hint = newHint;
        }
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

    void TextField::fitBoundsToText(float size, const Font& font, float spacing)
    {
        const auto dims = MeasureTextEx(font, text.c_str(), size, spacing);
        const auto startDims = getStartDimensions();
        setSize(std::max(dims.x + 4, startDims.x), std::max(dims.y + 2, startDims.y));
    }

    static bool IsKeyPressedAnyWay(const int key) { return IsKeyPressed(key) || IsKeyPressedRepeat(key); }

    bool TextField::updateInputs()
    {
        ++blinkCounter;
        if (blinkCounter > 2 * blinkDelay || (IsKeyPressedAnyWay(KEY_LEFT) || IsKeyPressedAnyWay(KEY_RIGHT)))
        {
            blinkCounter = 0;
        }
        const auto bounds = getBounds();
        const auto mouse = GetMousePos();
        if (LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            isFocused = PointToRect(mouse.x, mouse.y, bounds.x, bounds.y, bounds.width, bounds.height);
        }

        if (!isFocused)
        {
            return false;
        }

        const auto anyEvent = updateControls();
        bool anyInput = false;
        // Iterate characters
        for (int i = 0; i < GetCharPressedQueueCount(); ++i)
        {
            const char pressedChar = static_cast<char>(GetCharPressedQueue()[i]);
            if (hasSelection())
            {
                removeSelectionContent();
            }
            text.insert(text.begin() + cursorPos, pressedChar);
            ++cursorPos;
            anyInput = true;
        }
        return anyInput || anyEvent;
    }

    void TextField::drawText(const float fontSize, const Color color, const Color cursor, const Font& font,
                             const float spacing)
    {
        if (isFocused)
        {
            updateSelection(fontSize, font, spacing);
        }
        const auto bounds = getBounds();
        const Point tPos = getTextPos(bounds, fontSize);

        if (!isFocused && text.empty() && hint != nullptr)
        {
            DrawTextEx(font, hint, tPos.v(), fontSize, spacing, ColorAlpha(color, 0.75));
        }
        else
        {
            DrawTextEx(font, text.c_str(), tPos.v(), fontSize, spacing, color);
        }

        if (hasSelection())
        {
            const auto highlight = ColorAlpha(cursor, 0.45);
            DrawTextHighlight(selectionStart, selectionEnd, font, text.c_str(), tPos.v(), fontSize, spacing, highlight);
        }

        if (!isFocused || blinkCounter > blinkDelay || !showCursor)
        {
            return;
        }

        const auto* lineStart = text.data() + currLineStart;
        const auto cursorOffX = MeasureTextUpTo(lineStart, cursorPos - currLineStart, font, fontSize, spacing);
        const auto cursorOffY = (fontSize + static_cast<float>(GetTextLineSpacing())) * static_cast<float>(cursorLine);
        DrawTextEx(font, "|", {tPos.x + cursorOffX, tPos.y + cursorOffY}, fontSize, spacing, cursor);
    }

    void TextField::drawDefault(const Rectangle& bounds)
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const Color body = getIsFocused() ? theme.backSelected : getIsHovered() ? theme.backLight : theme.backDark;
        const Color outline = theme.backDark;
        DrawRectangleRounded(bounds, 0.1F, 20, body);
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 20, 2, outline);
        drawText(14, getIsFocused() ? theme.textActive : theme.textPassive, theme.textPassive);
    }

    bool TextField::pollControls()
    {
        const int size = static_cast<int>(text.size());
        if (IsKeyPressedAnyWay(MOVE_LEFT))
        {
            resetSelection();
            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                if (cursorPos - 1 < 0)
                {
                    return false;
                }
                if (isspace(text[cursorPos - 1]))
                {
                    --cursorPos;
                    return true;
                }
                for (int i = cursorPos - 1; i > -1; --i)
                {
                    if (isspace(text[i]))
                    {
                        cursorPos = i + 1;
                        return true;
                    }
                    if (i == 0)
                    {
                        cursorPos = 0;
                        return true;
                    }
                }
            }
            else if (cursorPos > 0)
            {
                --cursorPos;
                return true;
            }
        }
        else if (IsKeyPressedAnyWay(MOVE_RIGHT))
        {
            resetSelection();
            if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                if (isspace(text[cursorPos]))
                {
                    ++cursorPos;
                    return true;
                }
                for (int i = cursorPos; i < size; ++i)
                {
                    if (isspace(text[i]))
                    {
                        cursorPos = i;
                        return true;
                    }

                    if (i == size - 1)
                    {
                        cursorPos = i + 1;
                        return true;
                    }
                }
            }
            else if (cursorPos < size)
            {
                ++cursorPos;
                return true;
            }
        }
        else if (IsKeyPressedAnyWay(MOVE_UP))
        {
            resetSelection();
            if (cursorPos - 1 < 0)
            {
                return false;
            }
            for (int i = cursorPos - 1; i > -1; --i)
            {
                if (text[i] == '\n')
                {
                    cursorPos = i;
                    return true;
                }
            }
        }
        else if (IsKeyPressedAnyWay(MOVE_DOWN))
        {
            resetSelection();
            for (int i = cursorPos; i < size; ++i)
            {
                if (text[i] == '\n')
                {
                    cursorPos = i + 1;
                    return true;
                }
            }
        }
        else if (IsKeyPressedAnyWay(BACKSPACE))
        {
            if (hasSelection())
            {
                return removeSelectionContent();
            }
            if (cursorPos == 0)
            {
                return false;
            }
            text.erase(cursorPos - 1, 1);
            --cursorPos;
            return true;
        }
        else if (IsKeyPressedAnyWay(DELETE))
        {
            if (hasSelection())
            {
                return removeSelectionContent();
            }
            if (cursorPos == size)
            {
                return false;
            }
            text.erase(cursorPos, 1);
            return true;
        }
        else if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            auto copy = [&]()
            {
                if (hasSelection())
                {
                    auto start = &text[selectionStart];
                    auto temp = text[selectionEnd];
                    text[selectionEnd] = '\0';
                    SetClipboardText(start);
                    text[selectionEnd] = temp;
                }
            };
            if (IsKeyPressedAnyWay(PASTE)) // Paste
            {
                removeSelectionContent();
                const auto* clipboard = GetClipboardText();
                if (clipboard != nullptr)
                {
                    text.insert(cursorPos, clipboard);
                    cursorPos += static_cast<int>(strlen(clipboard));
                    return true;
                }
            }
            else if (IsKeyPressedAnyWay(COPY)) // Copy selection
            {
                copy();
            }
            else if (IsKeyPressedAnyWay(CUT))
            {
                copy();
                if (hasSelection())
                {
                    removeSelectionContent();
                    return true;
                }
            }
            else if (IsKeyPressedAnyWay(SELECT))
            {
                selectionStart = 0;
                selectionEnd = (int)text.size();
            }
        }
        return false;
    }

    bool TextField::updateControls()
    {
        // Clamp as safety at the beginning
        cursorPos = clamp(cursorPos, 0, static_cast<int>(text.size()));

        // Parses controls and returns true if cursor or text have changed
        if (pollControls())
        {
            textChanged = true;
            updateLineState();
            return true;
        }

        if (IsKeyPressedAnyWay(KEY_ENTER))
        {
            if (!onEnterPressed(IsKeyDown(KEY_LEFT_CONTROL), IsKeyDown(KEY_LEFT_SHIFT), IsKeyDown(KEY_LEFT_ALT)))
            {
                removeSelectionContent();
                text.insert(text.begin() + cursorPos, '\n');
                textChanged = true;
                ++cursorPos;
                updateLineState();
                return true;
            }
        }
        return false;
    }

    void TextField::updateSelection(float fSize, const Font& font, float spacing)
    {
        auto dragStart = GetDragStartPosition();
        if (dragStart == -1)
        {
            return;
        }

        const auto bounds = getBounds();
        const Point textStart = getTextPos(bounds, fSize);
        const float lineHeight = fSize + GetTextLineSpacing();

        auto findTextPos = [&](Point pos) -> int
        {
            // Adjust position relative to text start
            Point relPos = pos - textStart;
            relPos.floor();
            const int mouseLine = clamp((int)std::floor(relPos.y / lineHeight), 0, lineCount - 1);

            const int size = static_cast<int>(text.size());
            int lineCounter = 0;
            int lineStart = 0;

            for (int i = 0; i <= size; ++i)
            {
                if (text[i] == '\n')
                {
                    if (lineCounter == mouseLine) // Beyond end of line
                    {
                        return i;
                    }
                    lineCounter++;
                    lineStart = i + 1;
                    continue;
                }

                // On the target line
                if (lineCounter == mouseLine)
                {
                    const auto textWidth =
                        MeasureTextUpTo(text.c_str() + lineStart, i - lineStart, font, fSize, spacing);
                    if (textWidth > relPos.x)
                    {
                        return std::max(0, i - 1);
                    }
                }
            }
            if (lineCounter == mouseLine)
            {
                return size;
            }
            return 0;
        };

        selectionStart = findTextPos(dragStart);
        selectionEnd = findTextPos(GetMousePos());

        if (selectionStart > selectionEnd)
        {
            std::swap(selectionStart, selectionEnd);
        }
        cursorPos = selectionStart;
        updateLineState();
        if (selectionStart == selectionEnd)
        {
            resetSelection();
        }
    }

    void TextField::updateLineState()
    {
        currLineStart = 0;
        cursorLine = 0;
        lineCount = 1;
        const int size = static_cast<int>(text.size());
        for (int i = 0; i < size; ++i)
        {
            if (text[i] == '\n')
            {
                if (i < cursorPos)
                {
                    ++cursorLine;
                    currLineStart = i;
                }
                ++lineCount;
            }
        }
    }

    Point TextField::getTextPos(const Rectangle& bounds, float fontSize) const
    {
        const auto textHeight = (int)fontSize * lineCount + GetTextLineSpacing() * (lineCount - 1);
        return Point{bounds.x + 2, bounds.y + (bounds.height - (float)textHeight) / 2.0F}.floor();
    }

    bool TextField::removeSelectionContent()
    {
        if (!hasSelection())
        {
            return false;
        }
        text.erase(selectionStart, selectionEnd - selectionStart);
        cursorPos = selectionStart;
        resetSelection();
        return true;
    }

    void TextField::resetSelection()
    {
        selectionStart = -1;
        selectionEnd = -1;
    }

    bool TextField::hasSelection() const { return selectionStart != -1; }

} // namespace magique
