// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/UI.h>
#include <magique/ui/UIObject.h>
#include <magique/core/Types.h>

#include "internal/globals/UIData.h"
#include "internal/utils/CollisionPrimitives.h"

namespace magique
{
    UIObject::UIObject(const float x, const float y, const float w, const float h, const ScalingMode scaling)
    {
        const auto& ui = global::UI_DATA;
        px = x / ui.sourceRes.x;
        py = y / ui.sourceRes.y;
        if (w >= 0)
            pw = w / ui.sourceRes.x;
        if (h >= 0)
            ph = h / ui.sourceRes.y;
        setScalingMode(scaling);
        setStartPosition({px, py});
        setStartDimensions({pw, ph});
        global::UI_DATA.registerObject(this);
    }

    UIObject::UIObject(const float w, const float h, const Anchor anchor, const float inset, const ScalingMode scaling) :
        UIObject(0, 0, w, h, scaling)
    {
        setAnchor(anchor, inset);
    }

    void UIObject::draw()
    {
        onDraw(getBounds());
        global::UI_DATA.registerDrawCall(this, isContainer);
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(this); }

    //----------------- UTIL -----------------//

    void UIObject::align(const Anchor alignAnchor, const UIObject& relativeTo, float inset)
    {
        const auto [relX, relY, relWidth, relHeight] = relativeTo.getBounds();
        const auto [myX, myY, myWidth, myHeight] = getBounds();
        Point pos = {relX, relY};
        inset = GetScaled(inset);
        switch (alignAnchor)
        {
        case Anchor::TOP_LEFT:
            pos.x += inset;
            pos.y += inset;
            break;
        case Anchor::TOP_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += inset;
            break;
        case Anchor::TOP_RIGHT:
            pos.x += (relWidth - myWidth) - inset;
            pos.y += inset;
            break;
        case Anchor::MID_LEFT:
            pos.x += inset;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::MID_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::MID_RIGHT:
            pos.x += (relWidth - myWidth) - inset;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::BOTTOM_LEFT:
            pos.x += inset;
            pos.y += (relHeight - myWidth) - inset;
            break;
        case Anchor::BOTTOM_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += (relHeight - myWidth) - inset;
            break;
        case Anchor::BOTTOM_RIGHT:
            pos.x += (relWidth - myWidth) - inset;
            pos.y += (relHeight - myWidth) - inset;
            break;
        case Anchor::NONE:
            return;
        }
        setPosition(pos.x, pos.y);
    }

    void UIObject::align(const Direction direction, const UIObject& relativeTo, float offset)
    {
        const auto otherBounds = relativeTo.getBounds();
        const auto bounds = getBounds();
        Point pos = {otherBounds.x, otherBounds.y};
        offset = GetScaled(offset);
        switch (direction)
        {
        case Direction::LEFT:
            pos.x -= bounds.width + offset;
            break;
        case Direction::RIGHT:
            pos.x += bounds.width + offset;
            break;
        case Direction::UP:
            pos.y -= bounds.height + offset;
            break;
        case Direction::DOWN:
            pos.y += bounds.height + offset;
            break;
        }
        setPosition(pos.x, pos.y);
    }

    Rectangle UIObject::getBounds() const
    {
        const auto& ui = global::UI_DATA;
        const auto [sx, sy] = ui.targetRes;
        Rectangle bounds{px, py, pw, ph};
        switch (scaleMode)
        {
        case ScalingMode::FULL:
            bounds.x *= sx;
            bounds.y *= sy;
            bounds.width *= sx;
            bounds.height *= sy;
            break;
        case ScalingMode::KEEP_RATIO:
            bounds.x *= sx;
            bounds.y *= sy;
            bounds.width = pw * ui.sourceRes.x / ui.sourceRes.y * sy;
            bounds.height *= sy;
            break;
        case ScalingMode::NONE:
            bounds.x *= ui.sourceRes.x;
            bounds.y *= ui.sourceRes.y;
            bounds.width *= ui.sourceRes.x;
            bounds.height *= ui.sourceRes.y;
            break;
        }

        if (anchor != Anchor::NONE)
        {
            auto val = GetScaled(inset);
            const auto pos = GetUIAnchor(anchor, bounds.width, bounds.height, val);
            bounds.x = pos.x;
            bounds.y = pos.y;
        }
        return bounds;
    }

    void UIObject::setPosition(float x, float y)
    {
        const auto& dims = global::UI_DATA.targetRes;
        px = x / dims.x;
        py = y / dims.y;
    }

    void UIObject::setSize(float width, float height)
    {
        const auto& dims = global::UI_DATA.targetRes;
        if (width >= 0)
            pw = width / dims.x;
        if (height >= 0)
            ph = height / dims.y;
    }

    bool UIObject::getIsHovered() const
    {
        const auto& [mx, my] = global::UI_DATA.mouse;
        const auto [rx, ry, rw, rh] = getBounds();
        return PointToRect(mx, my, rx, ry, rw, rh);
    }

    bool UIObject::getIsClicked(const int button) const { return IsMouseButtonPressed(button) && getIsHovered(); }

    Anchor UIObject::getAnchor() const { return anchor; }

    float UIObject::getInset() const { return inset; }

    void UIObject::setAnchor(const Anchor newAnchor, const float inset)
    {
        anchor = newAnchor;
        this->inset = inset;
    }

    void UIObject::setScalingMode(const ScalingMode newScaling) { scaleMode = newScaling; }

    ScalingMode UIObject::getScalingMode() const { return scaleMode; }

    bool UIObject::getWasDrawn() const { return wasDrawnLastTick; }

    void UIObject::setStartPosition(const Point& pos) { startPos = pos; }

    const Point& UIObject::getStartPosition() const { return startPos; }

    void UIObject::setStartDimensions(const Point& dims) { startDims = dims; }

    const Point& UIObject::getStartDimensions() const { return startDims; }

} // namespace magique