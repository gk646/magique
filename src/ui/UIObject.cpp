#include <raylib/raylib.h>

#include <magique/ui/UIObject.h>
#include <magique/util/Logging.h>
#include <magique/core/Types.h>

#include "internal/globals/UIData.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    UIObject::UIObject(const float x, const float y, const float w, const float h)
    {
        setDimensions(x, y, w, h);
        global::UI_DATA.registerObject(*this);
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(*this); }

    void UIObject::render(const float transparency, const bool scissor)
    {
        global::UI_DATA.addRenderObject(*this, transparency, scissor);
    }

    //----------------- UTIL -----------------//

    void UIObject::align(const AnchorPosition anchor, const UIObject& relativeTo, const float inset)
    {
        const auto [relX, relY, relWidth, relHeight] = relativeTo.getBounds();
        const auto [myX, myY, myWidth, myHeight] = getBounds();
        float newX = relX;
        float newY = relY;

        switch (anchor)
        {
        case AnchorPosition::TOP_LEFT:
            newX += inset;
            newY += inset;
            break;
        case AnchorPosition::MID_LEFT:
            newX += inset;
            newY += relHeight / 2 - myHeight / 2;
            break;
        case AnchorPosition::BOTTOM_LEFT:
            newX += inset;
            newY += relHeight - myHeight - inset;
            break;
        case AnchorPosition::TOP_CENTER:
            newX += (relWidth / 2) - (myWidth / 2);
            newY += inset;
            break;
        case AnchorPosition::MID_CENTER:
            newX += (relWidth / 2) - (myWidth / 2);
            newY += relHeight / 2 - myHeight / 2;
            break;
        case AnchorPosition::BOTTOM_CENTER:
            newX += (relWidth / 2) - (myWidth / 2);
            newY += relHeight - myHeight - inset;
            break;
        case AnchorPosition::TOP_RIGHT:
            newX += relWidth - myWidth - inset;
            newY += inset;
            break;
        case AnchorPosition::MID_RIGHT:
            newX += relWidth - myWidth - inset;
            newY += (relHeight / 2) - (myHeight / 2);
            break;
        case AnchorPosition::BOTTOM_RIGHT:
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
        if (w >= 0)
            pw = w / MAGIQUE_UI_RESOLUTION_X;
        if (h >= 0)
            ph = h / MAGIQUE_UI_RESOLUTION_Y;
    }

    Rectangle UIObject::getBounds() const
    {
        const auto [sx, sy] = UIData::getScreenDims();

        constexpr float aspectRatioBase = MAGIQUE_UI_RESOLUTION_X / MAGIQUE_UI_RESOLUTION_Y;
        float currentAspectRatio = static_cast<float>(sx) / sy;

        float x = px * sy * aspectRatioBase;
        float y = py * sy;
        float w = pw * sy * aspectRatioBase;
        float h = ph * sy;

        if (aspectRatioBase > currentAspectRatio)
        {
            float npw = w / sx;
            float npx = px - (npw - pw) / 2.0F;
            x = npx * sy * currentAspectRatio;
        }
        return {std::round(x), std::round(y), std::round(w), std::round(h)};
    }

    bool UIObject::getIsHovered() const
    {
        const auto& ui = global::UI_DATA;
        const auto [mx, my] = ui.getMousePos();
        const auto [rx, ry, rw, rh] = getBounds();

        if (!PointToRect(mx, my, rx, ry, rw, rh))
            return false;

        // For safety dont use operator[]
        //auto mapIt = ui.stateData.find(GetGameState());
        //ASSERT(mapIt != ui.stateData.end(), "Internal Error: No objects for this gamestate");
        std::vector<UIObject*> sortedObjects = {};

        const auto it = std::ranges::find(sortedObjects, this);
        if (it == sortedObjects.end()) [[unlikely]]
        {
            LOG_ERROR("Internal Error: UIObject not found");
            return false;
        }

        // Iterate from the beginning of the vector to the current object's position
        for (auto iter = sortedObjects.begin(); iter != it; ++iter)
        {
            if ((*iter))
            {
                const auto [ox, oy, ow, oh] = (*iter)->getBounds();
                if (PointToRect(mx, my, ox, oy, ow, oh))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool UIObject::getIsClicked(const int button) const { return IsMouseButtonPressed(button) && getIsHovered(); }

    void UIObject::setLayer(const UILayer newLayer) { layer = newLayer; }

    UILayer UIObject::getLayer() const { return layer; }

    void UIObject::setOpaque(const bool val) { isOpaque = val; }

    bool UIObject::getIsOpaque() const { return isOpaque; }

} // namespace magique