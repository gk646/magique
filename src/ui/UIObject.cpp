#include <magique/ui/UI.h>
#include <magique/ui/UIObject.h>
#include <magique/core/Types.h>

#include "internal/globals/UIData.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{

    UIObject::UIObject(const float x, const float y, const float w, const float h, const ScalingMode scaling)
    {
        setDimensions(x, y, w, h);
        setScalingMode(scaling);
        global::UI_DATA.registerObject(this);
    }

    UIObject::UIObject(const float w, const float h, const ScalingMode scaling) : UIObject(0, 0, w, h, scaling) {}

    UIObject::UIObject(const AnchorPosition anchor, const float w, const float h, const ScalingMode scaling) :
        UIObject(0, 0, w, h, scaling)
    {
        setAnchor(anchor);
    }

    void UIObject::draw()
    {
        onDraw(getBounds());
        global::UI_DATA.registerDrawCall(this, isContainer);
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(this); }

    //----------------- UTIL -----------------//

    void UIObject::align(const AnchorPosition alignAnchor, const UIObject& relativeTo, const float inset)
    {
        const auto [relX, relY, relWidth, relHeight] = relativeTo.getBounds();
        const auto [myX, myY, myWidth, myHeight] = getBounds();
        Point pos = {relX, relY};
        switch (alignAnchor)
        {
        case AnchorPosition::TOP_LEFT:
            pos.x += inset;
            pos.y += inset;
            break;
        case AnchorPosition::TOP_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += inset;
            break;
        case AnchorPosition::TOP_RIGHT:
            pos.x += (relWidth - myWidth) - inset;
            pos.y += inset;
            break;
        case AnchorPosition::MID_LEFT:
            pos.x += inset;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case AnchorPosition::MID_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case AnchorPosition::MID_RIGHT:
            pos.x += (relWidth - myWidth) - inset;
            pos.y += (relHeight - myHeight) / 2.0F;
            break;
        case AnchorPosition::BOTTOM_LEFT:
            pos.x += inset;
            pos.y += (relHeight - myWidth) - inset;
            break;
        case AnchorPosition::BOTTOM_CENTER:
            pos.x += (relWidth - myWidth) / 2.0F;
            pos.y += (relHeight - myWidth) - inset;
            break;
        case AnchorPosition::BOTTOM_RIGHT:
            pos.x += (relWidth - myWidth) - inset;
            pos.y += (relHeight - myWidth) - inset;
            break;
        }
        // Apply scaling as setDimensions() expect values in logical resolution (1920x1080) - ours are in the current res
        const auto scaling = global::UI_DATA.getScaling();
        setDimensions(pos.x * scaling.x, pos.y * scaling.y);
    }

    void UIObject::align(const Direction direction, const UIObject& relativeTo, const float offset)
    {
        const auto otherBounds = relativeTo.getBounds();
        const auto bounds = getBounds();
        Point pos = {otherBounds.x, otherBounds.y};
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
        // Apply scaling as setDimensions() expect values in logical resolution (1920x1080) - ours are in the current res
        const auto scaling = global::UI_DATA.getScaling();
        setDimensions(pos.x * scaling.x, pos.y * scaling.y);
    }

    void UIObject::setDimensions(const float x, const float y, const float w, const float h)
    {
        px = x / MAGIQUE_UI_RESOLUTION_X;
        py = y / MAGIQUE_UI_RESOLUTION_Y;
        if (w >= 0)
            pw = w / MAGIQUE_UI_RESOLUTION_X;
        if (h >= 0)
            ph = h / MAGIQUE_UI_RESOLUTION_Y;
    }

    Rectangle UIObject::getBounds() const
    {
        const auto [sx, sy] = global::UI_DATA.getScreenDims();
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
            bounds.width = pw * MAGIQUE_UI_RESOLUTION_X / MAGIQUE_UI_RESOLUTION_Y * sy;
            bounds.height *= sy;
            break;
        case ScalingMode::NONE:
            bounds.x *= MAGIQUE_UI_RESOLUTION_X;
            bounds.y *= MAGIQUE_UI_RESOLUTION_Y;
            bounds.width *= MAGIQUE_UI_RESOLUTION_X;
            bounds.height *= MAGIQUE_UI_RESOLUTION_Y;
            break;
        }

        if (anchor != AnchorPosition::NONE)
        {
            const auto pos = GetUIAnchor(anchor, bounds.width, bounds.height);
            bounds.x = pos.x;
            bounds.y = pos.y;
        }
        return bounds;
    }

    bool UIObject::getIsHovered() const
    {
        const auto [mx, my] = global::UI_DATA.getMousePos();
        const auto [rx, ry, rw, rh] = getBounds();
        return PointToRect(mx, my, rx, ry, rw, rh);
    }

    bool UIObject::getIsClicked(const int button) const { return IsMouseButtonPressed(button) && getIsHovered(); }

    AnchorPosition UIObject::getAnchor() const { return anchor; }

    void UIObject::setAnchor(const AnchorPosition newAnchor) { anchor = newAnchor; }

    void UIObject::setScalingMode(const ScalingMode newScaling) { scaleMode = newScaling; }

    ScalingMode UIObject::getScalingMode() const { return scaleMode; }

    bool UIObject::getWasDrawn() const { return wasDrawn; }

    void UIObject::trackObject()
    {
        auto& uiData = global::UI_DATA;
        if (!uiData.objectsSet.contains(this))
            global::UI_DATA.registerObject(this, isContainer);
    }

} // namespace magique