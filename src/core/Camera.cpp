#include <cmath>

#include <raylib/raylib.h>
#include <magique/core/Camera.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/EngineData.h"
#include "magique/ecs/ECS.h"

namespace magique
{

    void CameraSetCullPadding(const int distance)
    {
        global::ENGINE_CONFIG.cameraCullPadding = static_cast<float>(distance);
    }

    void CameraSetViewOffset(const float x, const float y) { global::ENGINE_CONFIG.cameraViewOff = {x, y}; }

    void SetCameraTarget(const float x, const float y) { global::ENGINE_CONFIG.cameraPositionOff = {x, y}; }

    void CameraSetSmoothing(const float smoothing) { global::ENGINE_CONFIG.cameraSmoothing = smoothing; }

    Camera2D& CameraGet() { return global::ENGINE_DATA.camera; }

    MapID CameraGetMap()
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

    Vector2 CameraGetPosition() { return global::ENGINE_DATA.camera.target; }

    Rectangle CameraGetBounds()
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

    Rectangle CameraGetNativeBounds()
    {
        const auto& [offset, target, rotation, zoom] = global::ENGINE_DATA.camera;

        const float camLeft = target.x - offset.x / zoom;
        const float camTop = target.y - offset.y / zoom;
        const float camWidth = offset.x * 2 / zoom;
        const float camHeight = offset.y * 2 / zoom;

        return {camLeft, camTop, camWidth, camHeight};
    }

    bool IsInsideAnyActorViewBounds(Point point)
    {
        auto bounds = CameraGetBounds();

        if (CheckCollisionPointRec(point.v(), bounds))
        {
            return true;
        }

        for (const auto actor : GetView<ActorC>())
        {
            const auto& pos = GetComponent<PositionC>(actor);
            bounds = {pos.x - bounds.width / 2, pos.y - bounds.height / 2, bounds.width, bounds.height};
            if (CheckCollisionPointRec(point.v(), bounds))
            {
                return true;
            }
        }
        return false;
    }

    entt::entity CameraGetEntity() { return global::ENGINE_DATA.cameraEntity; }

    void CameraAddShakeImpulse(const Point direction, const float maxDistance, const float velocity, const float decay)
    {
        auto& shake = global::ENGINE_DATA.cameraShake;
        shake.decay = decay;
        shake.direction = shake.direction + direction;
        float const magnitude = sqrtf(shake.direction.x * shake.direction.x + shake.direction.y * shake.direction.y);
        shake.direction.x /= magnitude;
        shake.direction.y /= magnitude;
        shake.maxDist = maxDistance;
        shake.veloc = {velocity, velocity};
    }

} // namespace magique
