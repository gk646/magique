#include <raylib/raylib.h>

#include <magique/ui/types/UIObject.h>
#include <magique/util/Logging.h>
#include <magique/core/Types.h>

#include "internal/globals/UIData.h"
#include "internal/headers/CollisionDetection.h"

namespace magique
{
    UIObject::UIObject() : id(global::UI_DATA.getNextID()) { global::UI_DATA.registerObject(this); }

    UIObject::UIObject(const float x, const float y, const float w, const float h) : UIObject()
    {
        setDimensions(x, y, w, h);
    }

    UIObject::UIObject(const AnchorPosition anchor, const UIObject& relativeTo, const float inset) : UIObject()
    {
        align(anchor, relativeTo, inset);
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(this); }

    void UIObject::align(const AnchorPosition anchor, const UIObject& relativeTo, const float inset)
    {
        const auto [relX, relY, relWidth, relHeight] = relativeTo.getBounds();
        const auto [myX, myY, myWidth, myHeight] = getBounds();
        float newX = relX;
        float newY = relY;

        switch (anchor)
        {
        case AnchorPosition::LEFT_TOP:
            newX += inset;
            newY += inset;
            break;
        case AnchorPosition::LEFT_MID:
            newX += inset;
            newY += (relHeight / 2) - (myHeight / 2);
            break;
        case AnchorPosition::LEFT_BOTTOM:
            newX += inset;
            newY += relHeight - myHeight - inset;
            break;
        case AnchorPosition::CENTER_TOP:
            newX += (relWidth / 2) - (myWidth / 2);
            newY += inset;
            break;
        case AnchorPosition::CENTER_MID:
            newX += (relWidth / 2) - (myWidth / 2);
            newY += (relHeight / 2) - (myHeight / 2);
            break;
        case AnchorPosition::CENTER_BOTTOM:
            newX += (relWidth / 2) - (myWidth / 2);
            newY += relHeight - myHeight - inset;
            break;
        case AnchorPosition::RIGHT_TOP:
            newX += relWidth - myWidth - inset;
            newY += inset;
            break;
        case AnchorPosition::RIGHT_MID:
            newX += relWidth - myWidth - inset;
            newY += (relHeight / 2) - (myHeight / 2);
            break;
        case AnchorPosition::RIGHT_BOTTOM:
            newX += relWidth - myWidth - inset;
            newY += relHeight - myHeight - inset;
            break;
        }

        setDimensions(newX, newY);
    }

    void UIObject::setDimensions(const float x, const float y, const float w, const float h)
    {
        px = x / MAGIQUE_UI_RESOLUTION_X;
        py = y / MAGIQUE_UI_RESOLUTION_Y;
        pw = w / MAGIQUE_UI_RESOLUTION_X;
        ph = h / MAGIQUE_UI_RESOLUTION_Y;
    }

    void UIObject::setZIndex(const int newZ) { z = static_cast<uint16_t>(newZ); }

    void UIObject::setShown(const bool val) { isShown = val; }

    Rectangle UIObject::getBounds() const
    {
        const auto [sx, sy] = global::UI_DATA.getScreenDims();
        return {px * sx, py * sy, pw * sx, ph * sy};
    }

    int UIObject::getZIndex() const { return z; }

    bool UIObject::getIsShown() const { return isShown; }

    bool UIObject::getIsHovered() const
    {
        const auto& ui = global::UI_DATA;
        const auto [mx, my] = ui.getMousePos();
        const auto [rx, ry, rw, rh] = getBounds();

        if (!PointIntersectsRect(mx, my, rx, ry, rw, rh))
            return false;

        const auto it = std::ranges::find(ui.sortedObjects, this);
        if (it == ui.sortedObjects.end()) [[unlikely]]
        {
            LOG_ERROR("Internal Error: UIObject not found");
            return false;
        }

        // Iterate from the beginning of the vector to the current object's position
        for (auto iter = ui.sortedObjects.begin(); iter != it; ++iter)
        {
            if ((*iter)->getIsShown())
            {
                const auto [ox, oy, ow, oh] = (*iter)->getBounds();
                if (PointIntersectsRect(mx, my, ox, oy, ow, oh))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool UIObject::getIsPressed(const int button) const { return IsMouseButtonPressed(button) && getIsHovered(); }

} // namespace magique