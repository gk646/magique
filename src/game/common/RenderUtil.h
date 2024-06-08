#ifndef RENDERUTIL_H
#define RENDERUTIL_H


namespace magique
{
    inline void AssignCameraData(entt::registry& registry)
    {
        const auto view = registry.view<const CameraC, const PositionC>();
        auto& tickData = global::DRAW_TICK_DATA;
#if MAGIQUE_DEBUG == 1
        int count = 0;
#endif
        const float sWidth = GetScreenWidth();
        const float sHeight = GetScreenHeight();
        for (const auto e : view)
        {
            const auto& pos = view.get<PositionC>(e);
            tickData.cameraMap = pos.map;
            tickData.camera.offset = {sWidth / 2, sHeight / 2};
            tickData.cameraEntity = e;
            tickData.camera.target = {pos.x, pos.y};
#if MAGIQUE_DEBUG == 1
            count++;
#endif
        }
        // Center the camera
        const auto coll = REGISTRY.try_get<CollisionC>(global::DRAW_TICK_DATA.cameraEntity);
        if (coll) [[likely]]
        {
            tickData.camera.offset.x -= static_cast<float>(coll->width) / 2.0F;
            tickData.camera.offset.y -= static_cast<float>(coll->height) / 2.0F;
        }
#if MAGIQUE_DEBUG == 1
        //M_ASSERT(count < 2, "You have multiple cameras? O.O");
#endif
    }


} // namespace magique

#endif //RENDERUTIL_H