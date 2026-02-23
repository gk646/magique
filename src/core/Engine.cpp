// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Engine.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/EngineData.h"

namespace magique
{
    Game& GameGetInstance()
    {
        MAGIQUE_ASSERT(global::ENGINE_DATA.gameInstance != nullptr, "Called too early");
        return *global::ENGINE_DATA.gameInstance;
    }

    GameState EngineGetState() { return global::ENGINE_DATA.gameState; }

    void EngineSetState(const GameState newGameState)
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

    void EngineSetStateCallback(const GameStateCallback& func) { global::ENGINE_DATA.stateCallback = func; }

    void EngineSetUpdateRange(const int distance)
    {
        global::ENGINE_CONFIG.entityUpdateDist = static_cast<float>(distance);
    }

    int EngineGetUpdateRange() { return (int)global::ENGINE_CONFIG.entityUpdateDist; }

    void EngineSetCacheDuration(const int ticks)
    {
        global::ENGINE_CONFIG.entityCacheDuration = static_cast<uint16_t>(ticks);
    }

    void EngineAddToCache(const entt::entity e)
    {
        global::ENGINE_DATA.entityUpdateCache[e] = global::ENGINE_CONFIG.entityCacheDuration;
    }

    void EngineClearCache() { global::ENGINE_DATA.entityUpdateCache.clear(); }

    bool EngineIsCached(entt::entity e) { return global::ENGINE_DATA.entityUpdateCache.contains(e); }

    void EngineEnableCollision(const bool value) { global::ENGINE_CONFIG.enableCollisionSystem = value; }

    void EngineSetFont(const Font& font) { global::ENGINE_CONFIG.font = font; }

    const Font& EngineGetFont() { return global::ENGINE_CONFIG.font; }

    void EngineSetTheme(const Theme& theme) { global::ENGINE_CONFIG.theme = theme; }

    Theme EngineGetTheme() { return global::ENGINE_CONFIG.theme; }

    float EngineGetTime() { return global::ENGINE_DATA.engineTime; }

    uint32_t EngineGetTick() { return global::ENGINE_DATA.engineTicks; }

    //----------------- GET -----------------//

    const std::vector<entt::entity>& EngineGetUpdateEntities() { return global::ENGINE_DATA.entityUpdateVec; }

    const std::vector<MapID>& EngineGetActiveMaps() { return global::ENGINE_DATA.loadedMaps; }

    const std::vector<entt::entity>& EngineGetDrawEntities() { return global::ENGINE_DATA.drawVec; }

    void EngineSetPlayer(entt::entity entity) { global::ENGINE_DATA.playerEntity = entity; }

    entt::entity EngineGetPlayer() { return global::ENGINE_DATA.playerEntity; }

    bool EngineHasPlayer() { return global::ENGINE_DATA.playerEntity != entt::null; }

} // namespace magique
