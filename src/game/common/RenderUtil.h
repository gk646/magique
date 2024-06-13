#ifndef RENDERUTIL_H
#define RENDERUTIL_H


namespace magique
{
    // Logic loop as authority over the state
    // Render thread just draws the current state
    inline void AssignDrawTickCamera()
    {
        global::LOGIC_TICK_DATA.lock();
        {
            auto& drawTick = global::DRAW_TICK_DATA;
            auto& logicTick = global::LOGIC_TICK_DATA;

            drawTick.camera = logicTick.camera;
            drawTick.cameraMap = logicTick.cameraMap;
            drawTick.cameraEntity = logicTick.cameraEntity;
        }
        global::LOGIC_TICK_DATA.unlock();
    }

} // namespace magique

#endif //RENDERUTIL_H