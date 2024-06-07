#include <magique/core/Core.h>

#include "core/CoreData.h"

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

        for (int i = 0; i < CUSTOM_2 + 1; ++i)
        {
            TEXTURE_ATLASES.emplace_back(BLANK);
        }

        CONFIGURATION.font = GetFontDefault();

        if (CONFIGURATION.font.texture.id == 0)
        {
            LOG_ERROR("Failed to load default font");
            return false;
        }

        SHADERS.light = LoadShaderFromMemory(lightVert, lightFrag);
        SHADERS.shadow = LoadShaderFromMemory(shadowVert, shadowFrag);


        return true;
    }

    //----------------- SET -----------------//

    void SetCameraEntity(entt::entity entity)
    {

    }

    void SetUpdateDistance(int distance) { CONFIGURATION.entityUpdateDistance = static_cast<float>(distance); }

    void SetCameraViewPadding(int distance) { CONFIGURATION.cameraViewPadding = static_cast<float>(distance); }

    void SetEntityCacheDuration(int ticks) { CONFIGURATION.entityCacheDuration = ticks; }

    void AddToUpdateCache(entt::entity e) { LOGIC_TICK_DATA.entityUpdateCache[e] = CONFIGURATION.entityCacheDuration; }

    //----------------- GET -----------------//

    const vector<entt::entity>& GetUpdateEntities() { return LOGIC_TICK_DATA.entityUpdateVec; }

    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones() { return LOGIC_TICK_DATA.loadedMaps; }

    const std::vector<entt::entity>& GetDrawEntities() { return LOGIC_TICK_DATA.drawVec; }

    MapID GetCameraMap() { return LOGIC_TICK_DATA.cameraMap; }

    Vector2 GetCameraPosition() { return LOGIC_TICK_DATA.camera.target; }

    inline Rectangle GetCameraBounds()
    {
        const auto pad = CONFIGURATION.cameraViewPadding;
        auto& camera = LOGIC_TICK_DATA.camera;
        auto& target = camera.target;
        auto& offset = camera.offset;
        return {target.x - offset.x - pad, target.y - offset.y - pad, offset.x * 2 + pad * 2, offset.y * 2 + pad * 2};
    }

    DrawTickData& GetDrawTickData() { return DRAW_TICK_DATA; }

    void SyncThreads() { LOGIC_TICK_DATA.lock(); }

    void UnSyncThreads() { LOGIC_TICK_DATA.unlock(); }


} // namespace magique