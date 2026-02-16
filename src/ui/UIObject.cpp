// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>
#include <magique/ui/UIObject.h>
#include <magique/core/Types.h>

#include "internal/globals/UIData.h"
#include "magique/util/RayUtils.h"

namespace magique
{
    UIObject::UIObject(const float x, const float y, const float w, const float h, const ScalingMode scaling)
    {
        const auto& ui = global::UI_DATA;
        pBounds = {x, y, w, h};
        if (scaling != ScalingMode::ABSOLUTE)
        {
            pBounds.x /= ui.sourceRes.x;
            pBounds.y /= ui.sourceRes.y;
            pBounds.w /= ui.sourceRes.x;
            pBounds.h /= ui.sourceRes.y;
        }
        setScalingMode(scaling);
        setStartPosition(pBounds.pos());
        setStartDimensions(pBounds.size());
        global::UI_DATA.registerObject(this);
    }

    UIObject::UIObject(const float w, const float h, const Anchor anchor, const Point inset, const ScalingMode scaling) :
        UIObject(0, 0, w, h, scaling)
    {
        setAnchor(anchor, inset);
    }

    void UIObject::draw()
    {
        global::UI_DATA.registerDrawCall(this, isContainer);
        const auto bounds = getBounds();
        onDraw(bounds);
        if (global::UI_DATA.showHitboxes)
        {
            DrawRectangleLinesEx(bounds, 1, BLUE);
        }
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(this); }

    //----------------- UTIL -----------------//

    Rectangle UIObject::getBounds() const
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
        auto bounds = getBounds();
        return {bounds.x, bounds.y};
    }

    void UIObject::setSize(Point size)
    {
        const auto& dims = global::UI_DATA.targetRes;
        auto coords = size;
        if (scaleMode != ScalingMode::ABSOLUTE)
        {
            coords /= dims;
        }
        pBounds.w = coords.x;
        pBounds.h = coords.y;
    }

    void UIObject::align(const Anchor alignAnchor, const UIObject& relativeTo, Point alignInset)
    {
        const auto [relX, relY, relWidth, relHeight] = relativeTo.getBounds();
        const auto [myX, myY, myWidth, myHeight] = getBounds();
        Point pos = {relX, relY};
        alignInset = UIGetScaled(alignInset);
        switch (alignAnchor)
        {
        case Anchor::TOP_LEFT:
            pos.x += alignInset.x;
            pos.y += alignInset.y;
            break;
        case Anchor::TOP_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += alignInset.y;
            break;
        case Anchor::TOP_RIGHT:
            pos.x += (relWidth - myWidth) - alignInset.x;
            pos.y += alignInset.y;
            break;
        case Anchor::MID_LEFT:
            pos.x += alignInset.x;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::MID_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::MID_RIGHT:
            pos.x += (relWidth - myWidth) - alignInset.x;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::BOTTOM_LEFT:
            pos.x += alignInset.x;
            pos.y += relHeight - myHeight + alignInset.y;
            break;
        case Anchor::BOTTOM_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += relHeight - myHeight + alignInset.y;
            break;
        case Anchor::BOTTOM_RIGHT:
            pos.x += (relWidth - myWidth) - alignInset.x;
            pos.y += relHeight - myHeight + alignInset.y;
            break;
        case Anchor::NONE:
            break;
        }
        setPosition(pos);
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

    bool UIObject::getIsHovered() const { return CheckCollisionMouseRect(getBounds()); }

    bool UIObject::getIsClicked(const int button) const { return IsMouseButtonPressed(button) && getIsHovered(); }

    bool UIObject::getIsPressed(int mouseButton) const { return IsMouseButtonDown(mouseButton) && getIsHovered(); }

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

    void UIObject::setStartPosition(const Point& pos) { startPos = pos; }

    Point UIObject::getStartPosition() const
    {
        const auto& ui = global::UI_DATA;
        return startDims * ui.sourceRes;
    }

    void UIObject::setStartDimensions(const Point& dims) { startDims = dims; }

    Point UIObject::getStartDimensions() const
    {
        const auto& ui = global::UI_DATA;
        if (scaleMode == ScalingMode::ABSOLUTE)
        {
            return startDims;
        }
        return startDims * ui.sourceRes;
    }

    void UIObject::beginBoundsScissor() const
    {
        const auto bounds = getBounds();
        BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    }

} // namespace magique
