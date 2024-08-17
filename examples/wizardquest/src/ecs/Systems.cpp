#include <magique/core/Core.h>
#include <magique/ecs/ECS.h>

#include "ecs/Systems.h"
#include "ecs/Components.h"

using namespace magique;

void MovementSystem::update()
{
    for (const auto e : GetUpdateEntities())
    {
        if (!EntityHasComponents<PositionC, MovementC>(e))
            continue;
        auto& pos = GetComponent<PositionC>(e);
        auto& mov = GetComponent<MovementC>(e);
        const auto [x, y] = mov.getVelocity();
        pos.x += x;
        pos.y += y;
    }
}