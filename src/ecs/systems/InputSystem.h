#ifndef MAGIQUE_INPUTSYSTEM_H
#define MAGIQUE_INPUTSYSTEM_H

#include <magique/ecs/InternalScripting.h>
#include <raylib/raylib.h>

namespace magique ::ecs
{

    inline void PollInputs(entt::registry& registry)
    {
        const auto view = registry.view<PositionC>();
        for (const auto e : view)
        {
            InvokeEvent<onKeyEvent>(e);
        }
    }
} // namespace magique::ecs


#endif //MAGIQUE_INPUTSYSTEM_H