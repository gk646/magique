#include <magique/core/Core.h>
#include <magique/assets/AssetManager.h>

#include "core/globals/Configuration.h"
#include "core/globals/TextureAtlas.h"
#include "core/globals/LogicTickData.h"
#include "core/globals/DrawTickData.h"
#include "core/globals/Shaders.h"
#include "graphics/Shaders.h"

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
            global::TEXTURE_ATLASES.emplace_back(BLANK);
        }

        auto& config = global::CONFIGURATION;

        config.font = GetFontDefault();

        if (config.font.texture.id == 0)
        {
            LOG_ERROR("Failed to load default font");
            return false;
        }

        auto& shaders = global::SHADERS;
        shaders.light = LoadShaderFromMemory(lightVert, lightFrag);
        shaders.shadow = LoadShaderFromMemory(shadowVert, shadowFrag);

        shaders.lightLightLoc = GetShaderLocation(shaders.light, "lightPos");
        shaders.lightColorLoc = GetShaderLocation(shaders.light, "lightColor");
        shaders.shadowLightLoc = GetShaderLocation(shaders.shadow, "lightPosition");
        shaders.mvpLoc = GetShaderLocation(shaders.shadow, "mvp");

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

    MapID GetCameraMap() { return global::DRAW_TICK_DATA.cameraMap; }

    Vector2 GetCameraPosition() { return global::DRAW_TICK_DATA.camera.target; }

    Rectangle GetCameraBounds()
    {
        const auto pad = global::CONFIGURATION.cameraViewPadding;
        auto& camera = global::DRAW_TICK_DATA.camera;
        auto& target = camera.target;
        auto& offset = camera.offset;
        return {target.x - offset.x - pad, target.y - offset.y - pad, offset.x * 2 + pad * 2, offset.y * 2 + pad * 2};
    }

    void SyncThreads() { global::LOGIC_TICK_DATA.lock(); }

    void UnSyncThreads() { global::LOGIC_TICK_DATA.unlock(); }


} // namespace magique