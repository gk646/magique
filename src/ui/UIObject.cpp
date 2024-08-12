#include <raylib/raylib.h>

#include <magique/ui/types/UIObject.h>
#include <magique/util/Logging.h>
#include <magique/core/Types.h>

#include "internal/globals/UIData.h"
#include "internal/headers/CollisionPrimitives.h"

#include <magique/internal/Macros.h>

namespace magique
{

    UIObject::UIObject(GameState gameState, UILayer layer) : UIObject(gameState, 0,0,0,0,layer)
    {

    }

    UIObject::UIObject(const GameState gameState, const float x, const float y, const float w, const float h,
                       UILayer layer)
    {
        setDimensions(x, y, w, h);
    }

    UIObject::UIObject(const AnchorPosition anchor, const UIObject& relativeTo, const float inset) 
    {
        align(anchor, relativeTo, inset);
    }

    UIObject::~UIObject() { global::UI_DATA.unregisterObject(this, GameState(INT32_MAX)); }

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
            newY += (relHeight / 2) - (myHeight / 2);
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
            newY += (relHeight / 2) - (myHeight / 2);
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
        pw = w / MAGIQUE_UI_RESOLUTION_X;
        ph = h / MAGIQUE_UI_RESOLUTION_Y;
    }

    void UIObject::setLayer(const UILayer newLayer) { layer = static_cast<uint8_t>(newLayer); }

    void UIObject::setShown(const bool val) { isShown = val; }

    UILayer UIObject::getLayer() const { return static_cast<UILayer>(layer); }

    Rectangle UIObject::getBounds() const
    {
        const auto [sx, sy] = magique::UIData::getScreenDims();
        return {px * sx, py * sy, pw * sx, ph * sy};
    }

    bool UIObject::getIsShown() const { return isShown; }

    bool UIObject::getIsHovered() const
    {
        const auto& ui = global::UI_DATA;
        const auto [mx, my] = ui.getMousePos();
        const auto [rx, ry, rw, rh] = getBounds();

        if (!PointInRect(mx, my, rx, ry, rw, rh))
            return false;

        // For safety dont use operator[]
        auto mapIt = ui.stateData.find(GetGameState());
        ASSERT(mapIt != ui.stateData.end(), "Internal Error: No objects for this gamestate");
        auto& sortedObjects = mapIt->second;

        const auto it = std::ranges::find(sortedObjects, this);
        if (it == sortedObjects.end()) [[unlikely]]
        {
            LOG_ERROR("Internal Error: UIObject not found");
            return false;
        }

        // Iterate from the beginning of the vector to the current object's position
        for (auto iter = sortedObjects.begin(); iter != it; ++iter)
        {
            if ((*iter)->getIsShown())
            {
                const auto [ox, oy, ow, oh] = (*iter)->getBounds();
                if (PointInRect(mx, my, ox, oy, ow, oh))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool UIObject::getIsPressed(const int button) const { return IsMouseButtonPressed(button) && getIsHovered(); }

} // namespace magique