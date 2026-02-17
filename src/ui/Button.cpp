// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/ui/controls/Button.h>
#include <magique/ui/UI.h>

#include "internal/utils/CollisionPrimitives.h"
#include "internal/globals/EngineConfig.h"
#include "magique/core/Engine.h"
#include "magique/core/Draw.h"
#include "magique/util/RayUtils.h"

namespace magique
{
    Button::Button(const float x, const float y, const float w, const float h, ScalingMode mode) :
        UIObject(x, y, w, h, mode)
    {
    }

    Button::Button(const float w, const float h, const Anchor anchor, Point inset) : UIObject(w, h, anchor, inset) {}

    void Button::setOnClick(const ClickFunc& func) { clickFunc = func; }

    void Button::setDisabled(bool value) { isDisabled = value; }

    bool Button::getIsDisabled() const { return isDisabled; }

    void Button::setHoverText(const char* value) { hoverText = value; }

    const char* Button::getHoverText() const { return hoverText.c_str(); }

    void Button::updateActions(const Rectangle& bounds)
    {
        if (getIsHovered())
        {
            // Ensure click started within button
            if (getIsHovered())
            {
                for (int i = 0; i < MOUSE_BUTTON_BACK + 1; ++i) // All mouse buttons
                {
                    if (IsMouseButtonPressed(i) && !isDisabled)
                    {
                        onClick(bounds, i);
                        if (clickFunc)
                        {
                            clickFunc(bounds, i);
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

    void Button::drawDefault(const Rectangle& bounds)
    {
        const auto& theme = global::ENGINE_CONFIG.theme;
        const auto mouseDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        const Color body = isHovered && mouseDown ? theme.backSelected : isHovered ? theme.backLight : theme.backDark;
        const Color outline = isHovered && mouseDown ? theme.backLight : isHovered ? theme.backDark : theme.backDark;
        DrawRectangleRounded(bounds, 0.1F, 20, body);
        DrawRectangleRoundedLinesEx(bounds, 0.1F, 20, 2, outline);
        drawHoverText(EngineGetFont(), UIGetScaled(15), DARKGRAY, GRAY, WHITE);
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

    TextButton::TextButton(const char* text, ScalingMode mode) : Button(0, 0, 50, 10, mode), text(text)
    {
    }

    void TextButton::fitToText(const Font& font, float size)
    {
        setSize(Point{MeasureTextEx(font, text.c_str(), size, 1.0F)} + Point{2});
    }

    std::string& TextButton::getText() { return text; }

    const std::string& TextButton::getText() const { return text; }

    void TextButton::drawDefault(const Rectangle& bounds)
    {
        auto size = UIGetScaled(15);
        const auto& font = EngineGetFont();
        fitToText(font, size);
        Button::drawDefault(bounds);
        DrawTextCenteredRect(font, text.c_str(), size, bounds, 1.0F, global::ENGINE_CONFIG.theme.textActive);
    }


} // namespace magique
