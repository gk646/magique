#include <cmath>

#include <raylib/raylib.h>
#include <magique/core/Camera.h>
#include <magique/gamedev/PathFinding.h>

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

    void CameraSetTargetOffset(const float x, const float y) { global::ENGINE_CONFIG.cameraPositionOff = {x, y}; }

    void CameraSetSmoothing(const float smoothing) { global::ENGINE_CONFIG.cameraSmoothing = smoothing; }

    Entity CameraGetEntity() { return global::ENGINE_DATA.cameraEntity; }

    Camera2D& CameraGet() { return global::ENGINE_DATA.camera; }

    Camera2D& CameraSetZoom(float zoom, float min, float max)
    {
        global::ENGINE_DATA.camera.zoom = std::clamp(zoom, min, max);
        return global::ENGINE_DATA.camera;
    }

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

    Point CameraGetPosition() { return global::ENGINE_DATA.camera.target; }

    Rect CameraGetBounds()
    {
        const auto pad = global::ENGINE_CONFIG.cameraCullPadding;
        const auto& [offset, target, rotation, zoom] = global::ENGINE_DATA.camera;

        const float halfWidth = offset.x / zoom;
        const float halfHeight = offset.y / zoom;

        const float camLeft = target.x - halfWidth - pad;
        const float camTop = target.y - halfHeight - pad;
        const float camWidth = 2 * halfWidth + (pad * 2);
        const float camHeight = 2 * halfHeight + (pad * 2);

        return Rect{camLeft, camTop, camWidth, camHeight}.floored();
    }

    Rect CameraGetNativeBounds()
    {
        const auto& [offset, target, rotation, zoom] = global::ENGINE_DATA.camera;

        const float camLeft = target.x - offset.x / zoom;
        const float camTop = target.y - offset.y / zoom;
        const float camWidth = offset.x * 2 / zoom;
        const float camHeight = offset.y * 2 / zoom;

        return Rect{camLeft, camTop, camWidth, camHeight}.floored();
    }

    bool CameraInLineOfSight(Entity entity)
    {
        const auto& pos = ComponentGet<PositionC>(entity);
        if (pos.map != CameraGetMap())
            return false;
        Point mid = pos.pos;
        const auto* col = ComponentTryGet<CollisionC>(entity);
        if (col != nullptr)
            mid += col->getMidOffset();
        return PathRayCast(CameraGetPosition(), mid, CameraGetMap());
    }

    bool CameraInsideAnyViewBounds(Point point)
    {
        auto bounds = CameraGetBounds();
        if (bounds.contains(point))
        {
            return true;
        }
        for (const auto actor : ComponentGetView<ActorC>())
        {
            const auto& pos = ComponentGet<PositionC>(actor);
            bounds = Rect::CenteredOn(pos.pos, bounds.size());
            if (bounds.contains(point))
            {
                return true;
            }
        }
        return false;
    }

    void CameraShakeImpulse(const Point dir, const float maxDistance, const float velocity, const float decay)
    {
        auto& shake = global::ENGINE_DATA.cameraShake;
        shake.decay = decay;
        shake.direction = shake.direction + dir.normalized();
        shake.direction.normalize();
        shake.maxDist = maxDistance;
        shake.veloc = velocity;
    }

    void internal::CameraUpdateShake()
    {
        auto& shake = global::ENGINE_DATA.cameraShake;
        if (shake.maxDist <= 0)
        {
            shake.offset = 0;
            return;
        }

        Point maxPositive = shake.direction * shake.maxDist;
        maxPositive = maxPositive.abs();
        if (shake.offset.magnitude() >= maxPositive.magnitude())
        {
            shake.up = !shake.up;
        }

        Point moveVec = shake.direction * shake.veloc;
        if (!shake.up)
            moveVec.invert();

        shake.offset += moveVec;
        shake.offset.x = std::clamp(shake.offset.x, -maxPositive.x, maxPositive.x);
        shake.offset.y = std::clamp(shake.offset.y, -maxPositive.y, maxPositive.y);
        shake.maxDist = std::max(shake.maxDist - (shake.decay / MAGIQUE_LOGIC_TICKS), 0.0F);

        auto& target = CameraGet().target;
        target = Point{target} + shake.offset;
    }
} // namespace magique
