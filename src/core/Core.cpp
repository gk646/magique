#include <magique/core/Core.h>
#include <magique/assets/AssetManager.h>
#include <magique/util/JobSystem.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/TextureAtlas.h"
#include "internal/globals/EngineData.h"
#include "internal/globals/DynamicCollisionData.h"
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
        global::ENGINE_CONFIG.init();
        global::SHADERS.init(); // Loads the shaders and
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

    // void SetCameraEntity(entt::entity entity) { } // implemented in ECS.cpp cause of includes

    void SetUpdateDistance(const int distance)
    {
        global::ENGINE_CONFIG.entityUpdateDistance = static_cast<float>(distance);
    }

    void SetCameraViewPadding(const int distance)
    {
        global::ENGINE_CONFIG.cameraViewPadding = static_cast<float>(distance);
    }

    void SetManualCameraOffset(const float x, const float y) { global::ENGINE_CONFIG.manualCamOff = {x, y}; }

    void SetCameraSmoothing(float smoothing)
    {

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

    void SetEngineFont(const Font& font) { global::ENGINE_CONFIG.font = font; }

    void SetLightingMode(const LightingMode model) { global::ENGINE_CONFIG.lighting = model; }

    //----------------- GET -----------------//

    const std::vector<entt::entity>& GetUpdateEntities() { return global::ENGINE_DATA.entityUpdateVec; }

    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones() { return global::ENGINE_DATA.loadedMaps; }

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
        const auto pad = global::ENGINE_CONFIG.cameraViewPadding;
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

    void SetShowHitboxes(const bool val) { global::ENGINE_CONFIG.showHitboxes = val; }

    void SetBenchmarkTicks(const int ticks) { global::ENGINE_CONFIG.benchmarkTicks = ticks; }

    void ResetBenchmarkTimes()
    {
#if MAGIQUE_PROFILING == 1
        global::PERF_DATA.drawTimes.clear();
        global::PERF_DATA.logicTimes.clear();
#endif
    }

    void DrawHashGridDebug()
    {
        auto& grid = global::DY_COLL_DATA.hashGrid;
        int half = MAGIQUE_COLLISION_CELL_SIZE / 2;
        for (int i = 0; i < 50; ++i)
        {
            for (int j = 0; j < 50; ++j)
            {
                int x = i * MAGIQUE_COLLISION_CELL_SIZE;
                int y = j * MAGIQUE_COLLISION_CELL_SIZE;

                DrawRectangleLines(x, y, MAGIQUE_COLLISION_CELL_SIZE, MAGIQUE_COLLISION_CELL_SIZE, LIGHTGRAY);
                auto id = GetCellID(x / grid.getCellSize(), y / grid.getCellSize());
                const auto it = grid.cellMap.find(id);
                if (it != grid.cellMap.end())
                {
                    const auto count = static_cast<int>(grid.dataBlocks[grid.cellMap[id]].count);
                    const auto color = count > grid.getBlockSize() ? RED : GREEN;
                    DrawText(std::to_string(count).c_str(), x + half, y + half, 20, color);
                }
            }
        }
    }


} // namespace magique