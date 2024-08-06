#include <magique/core/Core.h>
#include <magique/assets/AssetManager.h>
#include <magique/util/Jobs.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/TextureAtlas.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/ShaderData.h"
#include "internal/globals/PerformanceData.h"

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

        for (int i = 0; i < ENTITIES_2 + 1; ++i)
        {
            global::TEXTURE_ATLASES.emplace_back();
        }

        global::ENGINE_CONFIG.init();
        global::SHADERS.init(); // Loads the shaders and
        global::ENGINE_DATA.camera.zoom = 1.0F;
        InitJobSystem();
        LOG_INFO("Initialized magique %s", MAGIQUE_VERSION);
        return true;
    }

    //----------------- SET -----------------//

    void SetShowPerformanceOverlay(const bool val) { global::ENGINE_CONFIG.showPerformanceOverlay = val; }

    //void SetCameraEntity(entt::entity entity) { } // implemented in ECS.cpp cause of includes

    void SetUpdateDistance(const int distance)
    {
        global::ENGINE_CONFIG.entityUpdateDistance = static_cast<float>(distance);
    }

    void SetCameraViewPadding(const int distance)
    {
        global::ENGINE_CONFIG.cameraViewPadding = static_cast<float>(distance);
    }

    void SetManualCameraOffset(const float x, const float y) { global::ENGINE_CONFIG.manualCamOff = {x, y}; }

    void SetEntityCacheDuration(const int ticks)
    {
        global::ENGINE_CONFIG.entityCacheDuration = static_cast<uint16_t>(ticks);
    }

    void AddToEntityCache(const entt::entity e)
    {
        global::ENGINE_DATA.entityUpdateCache[e] = global::ENGINE_CONFIG.entityCacheDuration;
    }

    void ClearEntityCache() { global::ENGINE_DATA.entityUpdateCache.clear(); }

    void SetEngineFont(const Font& font) { global::ENGINE_CONFIG.font = font; }

    void SetLightingMode(const LightingMode model) { global::ENGINE_CONFIG.lighting = model; }

    void SetStaticWorldBounds(const Rectangle& rectangle) { global::ENGINE_CONFIG.worldBounds = rectangle; }

    //----------------- GET -----------------//

    const std::vector<entt::entity>& GetUpdateEntities() { return global::ENGINE_DATA.entityUpdateVec; }

    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones() { return global::ENGINE_DATA.loadedMaps; }

    const Camera2D& GetCamera() { return global::ENGINE_DATA.camera; }

    const std::vector<entt::entity>& GetDrawEntities() { return global::ENGINE_DATA.drawVec; }

    MapID GetCameraMap() { return global::ENGINE_DATA.cameraMap; }

    Vector2 GetCameraPosition() { return global::ENGINE_DATA.camera.target; }

    Rectangle GetCameraBounds()
    {
        const auto pad = global::ENGINE_CONFIG.cameraViewPadding;
        const auto& [offset, target, rotation, zoom] = global::ENGINE_DATA.camera;

        const float camLeft = target.x - offset.x / zoom - pad;
        const float camTop = target.y - offset.y / zoom - pad;
        const float camWidth = offset.x * 2 / zoom + (pad * 2);
        const float camHeight = offset.y * 2 / zoom + (pad * 2);

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

    void SetShowHitboxes(const bool val) { global::ENGINE_CONFIG.showHitboxes = val; }

    void SetBenchmarkTicks(const int ticks) { global::ENGINE_CONFIG.benchmarkTicks = ticks; }

    void ResetBenchmarkTimes()
    {
        global::PERF_DATA.drawTimes.clear();
        global::PERF_DATA.logicTimes.clear();
    }


} // namespace magique