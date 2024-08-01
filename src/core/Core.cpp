#include <magique/core/Core.h>
#include <magique/assets/AssetManager.h>
#include <magique/util/Jobs.h>

#include "internal/globals/Configuration.h"
#include "internal/globals/TextureAtlas.h"
#include "internal/globals/LogicTickData.h"
#include "internal/globals/DrawTickData.h"
#include "internal/globals/ShaderEngine.h"


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

        global::CONFIGURATION.init();
        global::SHADERS.init(); // Loads the shaders and
        InitJobSystem();
        LOG_INFO("Initialized magique %s", MAGIQUE_VERSION);
        return true;
    }

    //----------------- SET -----------------//

    void SetShowPerformanceOverlay(const bool val) { global::CONFIGURATION.showPerformanceOverlay = val; }

    //void SetCameraEntity(entt::entity entity) { } // implemented in ECS.cpp cause of includes

    void SetUpdateDistance(const int distance)
    {
        global::CONFIGURATION.entityUpdateDistance = static_cast<float>(distance);
    }

    void SetCameraViewPadding(const int distance)
    {
        global::CONFIGURATION.cameraViewPadding = static_cast<float>(distance);
    }

    void SetManualCameraOffset(const float x, const float y) { global::CONFIGURATION.manualCamOff = {x, y}; }

    void SetEntityCacheDuration(const int ticks) { global::CONFIGURATION.entityCacheDuration = ticks; }

    void AddToEntityCache(const entt::entity e)
    {
        global::LOGIC_TICK_DATA.entityUpdateCache[e] = global::CONFIGURATION.entityCacheDuration;
    }

    void SetEngineFont(const Font& font) { global::CONFIGURATION.font = font; }

    void SetLightingMode(const LightingMode model) { global::CONFIGURATION.lighting = model; }

    void SetStaticWorldBounds(const Rectangle& rectangle) { global::CONFIGURATION.worldBounds = rectangle; }

    //----------------- GET -----------------//

    const vector<entt::entity>& GetUpdateEntities() { return global::LOGIC_TICK_DATA.entityUpdateVec; }

    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones() { return global::LOGIC_TICK_DATA.loadedMaps; }

    const std::vector<entt::entity>& GetDrawEntities() { return global::LOGIC_TICK_DATA.drawVec; }

    MapID GetCameraMap() { return global::LOGIC_TICK_DATA.cameraMap; }

    Vector2 GetCameraPosition() { return global::DRAW_TICK_DATA.camera.target; }

    Rectangle GetCameraBounds()
    {
        const auto pad = global::CONFIGURATION.cameraViewPadding;
        const auto& [offset, target, rotation, zoom] = global::LOGIC_TICK_DATA.camera;

        const float camLeft = target.x - offset.x / zoom - pad;
        const float camTop = target.y - offset.y / zoom - pad;
        const float camWidth = offset.x * 2 / zoom + (pad * 2);
        const float camHeight = offset.y * 2 / zoom + (pad * 2);

        return {camLeft, camTop, camWidth, camHeight};
    }

    Rectangle GetCameraNativeBounds()
    {
        const auto& [offset, target, rotation, zoom] = global::LOGIC_TICK_DATA.camera;

        const float camLeft = target.x - offset.x / zoom;
        const float camTop = target.y - offset.y / zoom;
        const float camWidth = offset.x * 2 / zoom;
        const float camHeight = offset.y * 2 / zoom;

        return {camLeft, camTop, camWidth, camHeight};
    }

    entt::entity GetCameraEntity() { return global::LOGIC_TICK_DATA.cameraEntity; }

    void SyncThreads() { global::LOGIC_TICK_DATA.lock(); }

    void UnSyncThreads() { global::LOGIC_TICK_DATA.unlock(); }

    void SetShowHitboxes(const bool val) { global::CONFIGURATION.showHitboxes = val; }

    void SetBenchmarkTicks(const int ticks) { global::CONFIGURATION.benchmarkTicks = ticks; }

} // namespace magique