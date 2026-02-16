// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Core.h>
#include <magique/assets/AssetImport.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/DynamicCollisionData.h"

namespace magique
{
    Game& GameGetInstance()
    {
        MAGIQUE_ASSERT(global::ENGINE_DATA.gameInstance != nullptr, "Called too early");
        return *global::ENGINE_DATA.gameInstance;
    }

    GameState GetGameState()
    {
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

    void SetGameStateChangeCallback(const GameStateCallback& func)
    {
        global::ENGINE_DATA.stateCallback = func;
    }

    void SetEntityUpdateRange(const int distance)
    {
        global::ENGINE_CONFIG.entityUpdateDistance = static_cast<float>(distance);
    }

    int GetEntityUpdateRange() { return (int)global::ENGINE_CONFIG.entityUpdateDistance; }

    void SetEntityCacheDuration(const int ticks)
    {
        global::ENGINE_CONFIG.entityCacheDuration = static_cast<uint16_t>(ticks);
    }

    void AddToEntityCache(const entt::entity e)
    {
        global::ENGINE_DATA.entityUpdateCache[e] = global::ENGINE_CONFIG.entityCacheDuration;
    }

    void ClearEntityCache() { global::ENGINE_DATA.entityUpdateCache.clear(); }

    bool IsInEntityCache(entt::entity e) { return global::ENGINE_DATA.entityUpdateCache.contains(e); }

    void SetEnableCollisionSystem(const bool value) { global::ENGINE_CONFIG.enableCollisionSystem = value; }

    void SetEngineFont(const Font& font) { global::ENGINE_CONFIG.font = font; }

    const Font& GetEngineFont() { return global::ENGINE_CONFIG.font; }

    float GetEngineTime() { return global::ENGINE_DATA.engineTime; }

    uint32_t GetEngineTick() { return global::ENGINE_DATA.engineTicks; }

    //----------------- GET -----------------//

    const std::vector<entt::entity>& GetUpdateEntities() { return global::ENGINE_DATA.entityUpdateVec; }

    const std::vector<MapID>& GetActiveMaps() { return global::ENGINE_DATA.loadedMaps; }

    // implemented in ECS.cpp cause of includes
    // const std::vector<entt::entity>& GetNearbyEntities(entt::entity entity, float radius){}

    const std::vector<entt::entity>& GetDrawEntities() { return global::ENGINE_DATA.drawVec; }

    void SetPlayerEntity(entt::entity entity) { global::ENGINE_DATA.playerEntity = entity; }

    entt::entity GetPlayerEntity() { return global::ENGINE_DATA.playerEntity; }


} // namespace magique