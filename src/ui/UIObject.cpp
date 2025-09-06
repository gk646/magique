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
        global::UI_DATA.registerDrawCall(this, isContainer);
        onDraw(getBounds());
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(this); }

    //----------------- UTIL -----------------//

    void UIObject::align(const Anchor alignAnchor, const UIObject& relativeTo, Point inset)
    {
        const auto [relX, relY, relWidth, relHeight] = relativeTo.getBounds();
        const auto [myX, myY, myWidth, myHeight] = getBounds();
        Point pos = {relX, relY};
        inset = {GetScaled(inset.x), GetScaled(inset.y)};
        switch (alignAnchor)
        {
        case Anchor::TOP_LEFT:
            pos.x += inset.x;
            pos.y += inset.y;
            break;
        case Anchor::TOP_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += inset.y;
            break;
        case Anchor::TOP_RIGHT:
            pos.x += (relWidth - myWidth) - inset.x;
            pos.y += inset.y;
            break;
        case Anchor::MID_LEFT:
            pos.x += inset.x;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::MID_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::MID_RIGHT:
            pos.x += (relWidth - myWidth) - inset.x;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case Anchor::BOTTOM_LEFT:
            pos.x += inset.x;
            pos.y += (relHeight - myWidth) - inset.y;
            break;
        case Anchor::BOTTOM_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += (relHeight - myWidth) - inset.y;
            break;
        case Anchor::BOTTOM_RIGHT:
            pos.x += (relWidth - myWidth) - inset.x;
            pos.y += (relHeight - myWidth) - inset.y;
            break;
        case Anchor::NONE:
            return;
        }
        setPosition(pos.x, pos.y);
    }

    void UIObject::align(const Direction direction, const UIObject& relativeTo, Point offset)
    {
        const auto otherBounds = relativeTo.getBounds();
        Point pos = {otherBounds.x, otherBounds.y};
        offset = {GetScaled(offset.x), GetScaled(offset.y)};
        switch (direction)
        {
        case Direction::LEFT:
            pos.x -= offset.x;
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
        setPosition(pos.x, pos.y);
    }

    Rectangle UIObject::getBounds() const
    {
        const auto& ui = global::UI_DATA;
        Rectangle bounds{px, py, pw, ph};
        ui.scaleBounds(bounds, scaleMode, inset, anchor);
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

    bool UIObject::getIsHovered() const { return CheckMouseRect(getBounds()); }

    bool UIObject::getIsClicked(const int button) const { return IsMouseButtonPressed(button) && getIsHovered(); }

    bool UIObject::getIsPressed(int mouseButton) const
    {
        return IsMouseButtonDown(mouseButton) && CheckCollisionPointRec(GetDragStartPosition().v(), getBounds());
    }

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

    void UIObject::beginBoundsScissor()
    {
        const auto bounds = getBounds();
        BeginScissorMode((int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height);
    }

} // namespace magique