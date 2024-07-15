#include <magique/core/Core.h>
#include <magique/assets/AssetManager.h>
#include <magique/util/Jobs.h>

#include "core/CoreData.h"
#include "core/globals/Configuration.h"
#include "core/globals/TextureAtlas.h"
#include "core/globals/LogicTickData.h"
#include "core/globals/DrawTickData.h"
#include "core/globals/Shaders.h"
#include "headers/Shaders.h"

static bool initCalled = false;

namespace magique
{
    bool InitMagique()
    {
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

        auto& config = global::CONFIGURATION;
        config.shadowResolution = {1280, 960};
        config.font = GetFontDefault();

        if (config.font.texture.id == 0)
        {
            LOG_ERROR("Failed to load default font");
            LOG_ERROR("Failed to initialize magique");
            return false;
        }

        auto& shaders = global::SHADERS;
        shaders.init();
        shaders.light = LoadShaderFromMemory(lightVert, lightFrag);
        shaders.shadow = LoadShaderFromMemory(shadowVert, shadowFrag);

        shaders.lightLightLoc = GetShaderLocation(shaders.light, "lightPos");
        shaders.lightColorLoc = GetShaderLocation(shaders.light, "lightColor");
        shaders.lightTypeLoc = GetShaderLocation(shaders.light, "lightType");
        shaders.lightIntensityLoc = GetShaderLocation(shaders.light, "intensity");

        shaders.shadowLightLoc = GetShaderLocation(shaders.shadow, "lightPosition");
        shaders.mvpLoc = GetShaderLocation(shaders.shadow, "mvp");

        const int threads = std::min(static_cast<int>(std::thread::hardware_concurrency()), 4);
        global::SCHEDULER = new Scheduler(threads);

        LOG_INFO("Initialized magique %s", MAGIQUE_VERSION);
        return true;
    }

    //----------------- SET -----------------//

    void SetCameraEntity(entt::entity entity) {}

    void SetUpdateDistance(int distance) { global::CONFIGURATION.entityUpdateDistance = static_cast<float>(distance); }

    void SetCameraViewPadding(int distance) { global::CONFIGURATION.cameraViewPadding = static_cast<float>(distance); }

    void SetEntityCacheDuration(int ticks) { global::CONFIGURATION.entityCacheDuration = ticks; }

    void AddToUpdateCache(entt::entity e)
    {
        global::LOGIC_TICK_DATA.entityUpdateCache[e] = global::CONFIGURATION.entityCacheDuration;
    }

    void SetFont(const Font& font) { global::CONFIGURATION.font = font; }

    void SetLightingModel(LightingModel model) { global::CONFIGURATION.lighting = model; }

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

    Scheduler& GetScheduler() { return *global::SCHEDULER; }


    void SyncThreads() { global::LOGIC_TICK_DATA.lock(); }

    void UnSyncThreads() { global::LOGIC_TICK_DATA.unlock(); }


} // namespace magique