// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>
#include <magique/ui/UIObject.h>
#include <magique/core/Types.h>
#include <magique/core/Draw.h>
#include <magique/core/Engine.h>
#include <magique/util/RayUtils.h>

#include "internal/globals/UIData.h"

namespace magique
{
    UIObject::UIObject(Rect bounds, const Anchor anchor, const Point inset, const ScalingMode scaling) :
        pBounds(bounds), startBounds(bounds)
    {
        const auto& ui = global::UI_DATA;
        if (scaling != ScalingMode::ABSOLUTE)
        {
            pBounds.x /= ui.sourceRes.x;
            pBounds.y /= ui.sourceRes.y;
            pBounds.width /= ui.sourceRes.x;
            pBounds.height /= ui.sourceRes.y;
        }
        setScalingMode(scaling);
        setAnchor(anchor, inset);
        global::UI_DATA.registerObject(this);
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(this); }

    void UIObject::draw()
    {
        auto& ui = global::UI_DATA;
        ui.registerDrawCall(this, isContainer);
        const auto bounds = getBounds();
        onDraw(bounds);
        if (ui.showHitboxes)
        {
            DrawRectangleLinesEx(bounds, 1, BLUE);
        }
    }

    Rect UIObject::getBounds() const
    {
        const auto& ui = global::UI_DATA;
        auto bounds = pBounds;
        ui.scaleBounds(bounds, scaleMode, inset, anchor);
        return bounds;
    }

    void UIObject::setPosition(const Point& pos)
    {
        const auto& dims = global::UI_DATA.targetRes;
        if (scaleMode == ScalingMode::ABSOLUTE)
        {
            pBounds = pos;
        }
        else
        {
            pBounds = pos / dims;
        }
    }

    Point UIObject::getPosition() const
    {
        const auto bounds = getBounds();
        return bounds.pos();
    }

    void UIObject::setSize(Point size)
    {
        const auto& dims = global::UI_DATA.targetRes;
        auto coords = size;
        if (scaleMode != ScalingMode::ABSOLUTE)
        {
            coords /= dims;
        }
        pBounds.width = coords.x;
        pBounds.height = coords.y;
    }

    void UIObject::setBounds(const Rect& dims)
    {
        setPosition(dims.pos());
        setSize(dims.size());
    }

    void UIObject::align(const Anchor alignAnchor, const UIObject& relativeTo, Point alignInset)
    {
        align(alignAnchor, relativeTo.getBounds(), alignInset);
    }

    void UIObject::align(const Direction direction, const UIObject& relativeTo, Point offset)
    {
        const auto otherBounds = relativeTo.getBounds();
        Point pos = {otherBounds.x, otherBounds.y};
        offset = {UIGetScaled(offset.x), UIGetScaled(offset.y)};
        switch (direction)
        {
        case Direction::LEFT:
            pos.x = pos.x - getBounds().width + offset.x;
            pos.y += offset.y;
            break;
        case Direction::RIGHT:
            pos.x += otherBounds.width + offset.x;
            pos.y += offset.y;
            break;
        case Direction::UP:
            pos.x += offset.x;
            pos.y -= offset.y;
            break;
        case Direction::DOWN:
            pos.x += offset.x;
            pos.y += otherBounds.height + offset.y;
            break;
        }
        setPosition(pos);
    }

    void UIObject::align(Anchor alignAnchor, const Rect& relativeTo, Point alignInset)
    {
        setPosition(UIGetAnchor(alignAnchor, relativeTo, getBounds().size(), UIGetScaled(alignInset)));
    }

    bool UIObject::getIsHovered(bool layered) const
    {
        return (!layered || !LayeredInput::GetIsMouseConsumed()) && getBounds().contains(GetMousePos());
    }

    bool UIObject::getIsClicked(const int button, bool layered) const
    {
        if (layered)
        {
            return LayeredInput::IsMouseButtonPressed(button) && getIsHovered();
        }
        else
        {
            return IsMouseButtonPressed(button) && getIsHovered(layered);
        }
    }

    bool UIObject::getIsPressed(int mouseButton, bool layered) const
    {
        if (layered)
        {
            return LayeredInput::IsMouseButtonDown(mouseButton) && getIsHovered();
        }
        else
        {
            return IsMouseButtonDown(mouseButton) && getIsHovered(layered);
        }
    }

    void UIObject::setAnchor(const Anchor newAnchor, const Point newInset)
    {
        anchor = newAnchor;
        inset = newInset;
    }

    Anchor UIObject::getAnchor() const { return anchor; }

    Point UIObject::getInset() const { return inset; }

    void UIObject::setScalingMode(const ScalingMode newScaling) { scaleMode = newScaling; }

    ScalingMode UIObject::getScalingMode() const { return scaleMode; }

    bool UIObject::getWasDrawn() const { return wasDrawnLastTick; }

    Rect UIObject::getStartBounds() const { return startBounds; }

    void UIObject::beginBoundsScissor() const
    {
        const auto bounds = getBounds();
        BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    }

    bool UIObject::getIsMenu() const { return isMenu; }

    void UIObject::setGamepadMapping(GamepadMapping* map) { mapping = map; }

    GamepadMapping* UIObject::getGamepadMapping() const { return mapping; }

    LabelledObject::LabelledObject(Rect size, std::string_view text, Direction direction, Anchor anchor, Point inset,
                                   ScalingMode scaling) :
        UIObject(size, anchor, inset, scaling), label(text), dir(direction)
    {
    }

    std::string_view LabelledObject::getText() const { return label; }

    void LabelledObject::setText(std::string_view text) { label = text; }

    void LabelledObject::setDirection(Direction direction) { dir = direction; }

    Direction LabelledObject::getDirection() const { return dir; }

    void LabelledObject::drawLabelDefault(const Rect& bounds) const
    {
        drawLabel(bounds, EngineGetFont(), EngineGetFont().baseSize);
    }

    void LabelledObject::drawLabel(const Rect& bounds, const Font& font, float fSize) const
    {
        if (label.empty())
            return;

        const float centerYOff = std::ceil((bounds.height - fSize) / 2.0F);
        switch (dir)
        {
        case Direction::LEFT:
            DrawTextRightBound(font, label, bounds.pos() + Point{-2, centerYOff}, fSize);
            break;
        case Direction::RIGHT:
            DrawTextEx(font, label.data(), bounds.topRight() + Point{2, centerYOff}, fSize, 1.0F, WHITE);
            break;
        case Direction::UP:
            DrawTextCentered(font, label, bounds.bottomMid() - Point{0, fSize + 4}, fSize);
            break;
        case Direction::DOWN:
            DrawTextCentered(font, label, bounds.topMid() + Point{0, 4}, fSize);
            break;
        }
    }


} // namespace magique
