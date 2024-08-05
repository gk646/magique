#ifndef DRAWTICKDATA_H
#define DRAWTICKDATA_H

namespace magique
{
    // The information used through a single draw tick
    // The information is guaranteed to not change during a tick!
    struct DrawTickData final
    {
        Camera2D camera{};
        MapID cameraMap;
        entt::entity cameraEntity;
    };

    namespace global
    {
        inline DrawTickData DRAW_TICK_DATA;
    }
} // namespace magique
#endif //DRAWTICKDATA_H