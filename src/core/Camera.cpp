#include <raylib/raylib.h>

#include <magique/core/Camera.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/EngineData.h"

namespace magique
{

    void SetCameraCullPadding(const int distance)
    {
        global::ENGINE_CONFIG.cameraCullPadding = static_cast<float>(distance);
    }

    void SetCameraViewOffset(const float x, const float y) { global::ENGINE_CONFIG.cameraViewOff = {x, y}; }

    void SetCameraPositionOffset(float x, float y) { global::ENGINE_CONFIG.cameraPositionOff = {x, y}; }

    void SetCameraSmoothing(const float smoothing) { global::ENGINE_CONFIG.cameraSmoothing = smoothing; }

    Camera2D& GetCamera() { return global::ENGINE_DATA.camera; }

    MapID GetCameraMap()
    {
#ifdef MAGIQUE_DEBUG
        if (global::ENGINE_DATA.cameraMap == static_cast<MapID>(UINT8_MAX))
        {
            LOG_WARNING("No camera exists!");
            return static_cast<MapID>(UINT8_MAX);
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

} // namespace magique