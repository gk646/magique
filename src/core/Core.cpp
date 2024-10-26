#include <magique/core/Core.h>
#include <magique/assets/AssetManager.h>
#include <magique/util/JobSystem.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/TextureAtlas.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/DynamicCollisionData.h"
#include "internal/globals/ShaderData.h"

namespace magique
{
    bool InitMagique()
    {
        static bool initCalled = false;
        if (initCalled)
        {
            LOG_WARNING("Init called twice. Skipping...");
            return true;
        }
        initCalled = true;
        global::ENGINE_CONFIG.init();
        global::SHADERS.init(); // Loads the shaders and buffers
        global::ENGINE_DATA.camera.zoom = 1.0F;
        InitJobSystem();
        LOG_INFO("Initialized magique %s", MAGIQUE_VERSION);
        return true;
    }

    GameState GetGameState() { return global::ENGINE_DATA.gameState; }

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

    //----------------- SET -----------------//

    void SetShowPerformanceOverlay(const bool val) { global::ENGINE_CONFIG.showPerformanceOverlay = val; }

    // implemented in ECS.cpp cause of includes
    // void SetCameraEntity(entt::entity entity) { }
    // bool NearbyEntitiesContain(Point origin, float radius, entt::entity target) {}

    void SetUpdateDistance(const int distance)
    {
        global::ENGINE_CONFIG.entityUpdateDistance = static_cast<float>(distance);
    }

    void SetCameraCullPadding(const int distance)
    {
        global::ENGINE_CONFIG.cameraCullPadding = static_cast<float>(distance);
    }

    void SetCameraViewOffset(const float x, const float y) { global::ENGINE_CONFIG.cameraViewOff = {x, y}; }

    void SetCameraPositionOffset(float x, float y)
    {
        global::ENGINE_CONFIG.cameraPositionOff = {x, y};
    }

    void SetCameraSmoothing(const float smoothing) { global::ENGINE_CONFIG.cameraSmoothing = smoothing; }

    void SetEntityCacheDuration(const int ticks)
    {
        global::ENGINE_CONFIG.entityCacheDuration = static_cast<uint16_t>(ticks);
    }

    void AddToEntityCache(const entt::entity e)
    {
        global::ENGINE_DATA.entityUpdateCache[e] = global::ENGINE_CONFIG.entityCacheDuration;
    }

    void ClearEntityCache() { global::ENGINE_DATA.entityUpdateCache.clear(); }

    void SetEnableCollisionHandling(bool value) {global::ENGINE_CONFIG.handleCollisions = value;}

    void SetEngineFont(const Font& font) { global::ENGINE_CONFIG.font = font; }

    void SetLightingMode(const LightingMode model) { global::ENGINE_CONFIG.lighting = model; }

    //----------------- GET -----------------//

    const std::vector<entt::entity>& GetUpdateEntities() { return global::ENGINE_DATA.entityUpdateVec; }

    const std::vector<MapID>& GetLoadedMaps() { return global::ENGINE_DATA.loadedMaps; }

    // implemented in ECS.cpp cause of includes
    // const std::vector<entt::entity>& GetNearbyEntities(entt::entity entity, float radius){}

    Camera2D& GetCamera() { return global::ENGINE_DATA.camera; }

    const std::vector<entt::entity>& GetDrawEntities() { return global::ENGINE_DATA.drawVec; }

    MapID GetCameraMap()
    {
#ifdef MAGIQUE_DEBUG
        if (global::ENGINE_DATA.cameraMap == MapID(UINT8_MAX))
        {
            LOG_WARNING("No camera exists!");
        }
#endif
        return global::ENGINE_DATA.cameraMap;
    }

    Vector2 GetCameraPosition() { return global::ENGINE_DATA.camera.target; }

    Rectangle GetCameraBounds()
    {
        const auto pad = global::ENGINE_CONFIG.cameraCullPadding;
        const auto& [offset, target, rotation, zoom] = global::ENGINE_DATA.camera;

        const float halfWidth = offset.x / zoom;
        const float halfHeight = offset.y / zoom;

        const float camLeft = target.x - halfWidth - pad;
        const float camTop = target.y - halfHeight - pad;
        const float camWidth = 2 * halfWidth + (pad * 2);
        const float camHeight = 2 * halfHeight + (pad * 2);

        return {camLeft, camTop, camWidth, camHeight};
    }

    Rectangle GetCameraNativeBounds()
    {
        const auto& [offset, target, rotation, zoom] = global::ENGINE_DATA.camera;

        const float camLeft = target.x - offset.x / zoom;
        const float camTop = target.y - offset.y / zoom;
        const float camWidth = offset.x * 2 / zoom;
        const float camHeight = offset.y * 2 / zoom;

        return {camLeft, camTop, camWidth, camHeight};
    }

    entt::entity GetCameraEntity() { return global::ENGINE_DATA.cameraEntity; }

    GameConfig& GetGameConfig() { return global::ENGINE_DATA.gameConfig; }

} // namespace magique