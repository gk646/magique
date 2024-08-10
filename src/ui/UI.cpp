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
        auto& ui = global::UI_DATA;
        const auto it = ui.objectMap.find(name);
        if (it == ui.objectMap.end())
        {
            ui.objectMap.insert({name, object});
        }
        else if (it->second != object)
        {
            delete it->second;
            it->second = object;
        }

        M_ASSERT(gameState != GameState(INT32_MAX),
                 "Using the default gamestate! Make your own enum and use SetGameState()");

        ui.stateData[gameState].registerObject()
    }

    bool RemoveUIObject(const char* name, GameState gameState) {}

    void SetLoadingScreen(LoadingScreen* loadingScreen) {}

    Point GetUIAnchor(const AnchorPosition anchor, const float width, const float height, const float inset)
    {
        Point point{};
        switch (anchor)
        {
        case AnchorPosition::LEFT_TOP:
            point = {inset, inset};
            break;
        case AnchorPosition::CENTER_TOP:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, inset};
            break;
        case AnchorPosition::RIGHT_TOP:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, inset};
            break;
        case AnchorPosition::LEFT_MID:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::CENTER_MID:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::RIGHT_MID:
            point = {MAGIQUE_UI_RESOLUTION_X - width - inset, MAGIQUE_UI_RESOLUTION_Y / 2 - height / 2};
            break;
        case AnchorPosition::LEFT_BOTTOM:
            point = {inset, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::CENTER_BOTTOM:
            point = {MAGIQUE_UI_RESOLUTION_X / 2 - width / 2, MAGIQUE_UI_RESOLUTION_Y - height - inset};
            break;
        case AnchorPosition::RIGHT_BOTTOM:
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