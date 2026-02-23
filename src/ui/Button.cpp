// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/ui/controls/Button.h>
#include <magique/ui/UI.h>
#include <magique/core/Engine.h>

#include "internal/globals/EngineConfig.h"
#include "magique/util/RayUtils.h"

namespace magique
{
    Button::Button(Rect bounds, const Anchor anchor, Point inset, ScalingMode mode) :
        UIObject(bounds, anchor, inset, mode)
    {
    }

    void Button::setOnClick(const ClickFunc& func) { clickFunc = func; }

    void Button::setDisabled(bool value) { isDisabled = value; }

    bool Button::getIsDisabled() const { return isDisabled; }

    void Button::setHoverText(const char* value) { hoverText = value; }

    const char* Button::getHoverText() const { return hoverText.c_str(); }

    void Button::updateActions(const Rect& bounds)
    {
        if (getIsHovered())
        {
            // Ensure click started within button
            if (getIsHovered())
            {

                for (int i = 0; i < MOUSE_BUTTON_MIDDLE + 1; ++i) // All mouse buttons
                {
                    if (LayeredInput::IsMouseButtonPressed(MouseButton(i)) && !isDisabled)
                    {
                        onClick(bounds, i);
                        if (clickFunc)
                        {
                            clickFunc(bounds, i);
                            LayeredInput::ConsumeMouse();
                        }
                    }
                }
            }
            if (!isHovered)
            {
                isHovered = true;
                onHover(bounds);
            }
        }
        else
        {
            isHovered = false;
        }
    }

    void Button::drawDefault(const Rect& bounds)
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const auto mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        const Color body = theme.getBodyColor(isHovered, isHovered && mouseDown);
        const Color outline = theme.getOutlineColor(isHovered, isHovered && mouseDown);
        DrawRectFrameFilled(bounds.floor(), body, outline);
        drawHoverText(EngineGetFont(), UIGetScaled(1), DARKGRAY, GRAY, WHITE);
    }

    void Button::drawHoverText(const Font& fnt, float size, Color back, Color outline, Color text) const
    {
        if (hoverText.empty() || !CheckCollisionMouseRect(getBounds()))
        {
            return;
        }

        auto mouse = GetMousePos();
        const auto dims = MeasureTextEx(fnt, hoverText.c_str(), size, 1.0F);
        Rectangle textRect = {mouse.x, mouse.y, dims.x + 4, dims.y + 2};
        DrawRectFrameFilled(textRect, back, outline);
        mouse += Point{1, 2};
        DrawTextEx(fnt, hoverText.c_str(), mouse, size, 1.0F, text);
    }

    TextButton::TextButton(const char* txt, Anchor anchor, Point inset, ScalingMode mod) :
        Button({}, anchor, inset, mod), text(txt)
    {
    }

    void TextButton::fitToText(const Font& font, float size)
    {
        setSize(Point{MeasureTextEx(font, text.c_str(), size, 1.0F)} + Point{4});
    }

    std::string& TextButton::getText() { return text; }

    const std::string& TextButton::getText() const { return text; }

    void TextButton::drawDefault(const Rect& bounds)
    {
        const auto& font = EngineGetFont();
        fitToText(font, font.baseSize);
        Button::drawDefault(bounds);
        DrawTextCenteredRect(font, text.c_str(), font.baseSize, bounds, 1.0F, global::ENGINE_CONFIG.theme.text);
    }


} // namespace magique
