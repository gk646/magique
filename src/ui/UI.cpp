#include <magique/ui/UI.h>
#include <magique/internal/Macros.h>

#include "internal/globals/UIData.h"

namespace magique
{
    template <>
    void* GetUIObject(const char* name)
    {
        const auto it = global::UI_DATA.objectMap.find(name);
        if (it == global::UI_DATA.objectMap.end())
        {
            return nullptr;
        }
        return it->second;
    }

    void AddUIObject(const char* name, const GameState gameState, UIObject* object, const UILayer layer)
    {
        ASSERT(gameState != GameState(INT32_MAX),
                 "Using the default gamestate! Make your own enum and use SetGameState()");
        ASSERT(object != nullptr, "Passing nullptr");
        ASSERT(name != nullptr, "Passing nullptr");
        ASSERT(layer <= UILayer::ROOT, "Passing invalid layer");
        global::UI_DATA.registerObject(name, gameState, object, layer);
    }

    bool RemoveUIObject(const char* name, const GameState gameState)
    {
        auto& ui = global::UI_DATA;
        const auto it = ui.objectMap.find(name);
        if (it == ui.objectMap.end())
        {
            return false;
        }

        return global::UI_DATA.unregisterObject(it->second, gameState);
    }

    void SetLoadingScreen(LoadingScreen* loadingScreen)
    {

    }

    Point GetUIAnchor(const AnchorPosition anchor, const float width, const float height, const float inset)
    {
        Point point{};
        switch (anchor)
        {
        case AnchorPosition::TOP_LEFT:
            point = {inset, inset};
            break;
        case AnchorPosition::TOP_CENTER:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, inset};
            break;
        case AnchorPosition::TOP_RIGHT:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, inset};
            break;
        case AnchorPosition::MID_LEFT:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::MID_CENTER:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::MID_RIGHT:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::BOTTOM_LEFT:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::BOTTOM_CENTER:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::BOTTOM_RIGHT:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        }
        const auto [sx, sy] = global::UI_DATA.getScaling();
        point.x *= sx;
        point.y *= sy;
        return point;
    }

    float GetScaled(const float val) { return global::UI_DATA.scaleX * val; }

    Point GetUIScaling() { return global::UI_DATA.getScaling(); }


} // namespace magique