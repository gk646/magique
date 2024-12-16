// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Core.h>
#include <magique/assets/AssetManager.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/DynamicCollisionData.h"

namespace magique
{
    GameState GetGameState()
    {
        if (global::ENGINE_DATA.gameState == GameState{INT32_MAX})
        {
            LOG_WARNING("Calling GetGameState() but game state is not set");
            return GameState{INT32_MAX};
        }
        return global::ENGINE_DATA.gameState;
    }

    void SetGameState(const GameState newGameState)
    {
        if (global::ENGINE_DATA.gameState != newGameState)
        {
            if (global::ENGINE_DATA.stateCallback)
            {
                global::ENGINE_DATA.stateCallback(global::ENGINE_DATA.gameState, newGameState);
            }
            global::ENGINE_DATA.gameState = newGameState;
        }
    }

    void SetGameStateChangeCallback(const std::function<void(GameState oldState, GameState newState)>& func)
    {
        global::ENGINE_DATA.stateCallback = func;
    }

    void SetUpdateDistance(const int distance)
    {
        global::ENGINE_CONFIG.entityUpdateDistance = static_cast<float>(distance);
    }

    void SetEntityCacheDuration(const int ticks)
    {
        global::ENGINE_CONFIG.entityCacheDuration = static_cast<uint16_t>(ticks);
    }

    void AddToEntityCache(const entt::entity e)
    {
        global::ENGINE_DATA.entityUpdateCache[e] = global::ENGINE_CONFIG.entityCacheDuration;
    }

    void ClearEntityCache() { global::ENGINE_DATA.entityUpdateCache.clear(); }

    void SetEnableCollisionSystem(const bool value) { global::ENGINE_CONFIG.enableCollisionSystem = value; }

    void SetFont(const Font& font) { global::ENGINE_CONFIG.font = font; }

    const Font& GetFont() { return global::ENGINE_CONFIG.font; }

    void SetLightingMode(const LightingMode model) { global::ENGINE_CONFIG.lighting = model; }

    void SetEnableLightingSystem(const bool val) { global::ENGINE_CONFIG.enableLightingSystem = val; }

    //----------------- GET -----------------//

    const std::vector<entt::entity>& GetUpdateEntities() { return global::ENGINE_DATA.entityUpdateVec; }

    const std::vector<MapID>& GetLoadedMaps() { return global::ENGINE_DATA.loadedMaps; }

    // implemented in ECS.cpp cause of includes
    // const std::vector<entt::entity>& GetNearbyEntities(entt::entity entity, float radius){}

    const std::vector<entt::entity>& GetDrawEntities() { return global::ENGINE_DATA.drawVec; }


} // namespace magique