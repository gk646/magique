#ifndef STATICCOLLISIONSYSTEM_H
#define STATICCOLLISIONSYSTEM_H

namespace magique
{

    // Give each thread a piece and the main thread aswell - so it doesnt wait while doing nothing
    static constexpr int WORK_PARTS = MAGIQUE_WORKER_THREADS + 1;

    void StaticCollisionSystem(const entt::registry& registry)
    {
        auto& data = global::ENGINE_DATA;
        const auto& collisionVec = data.collisionVec;

        for (const auto e : collisionVec)
        {
            const auto& pos = registry.get<const PositionC>(e);
            const auto& col = registry.get<const CollisionC>(e);
            switch (col.shape)
            {
            case Shape::RECT:
                break;
            case Shape::CIRCLE:
                break;
            case Shape::CAPSULE:
                break;
            case Shape::TRIANGLE:
                break;
            }
        }
    }


} // namespace magique

#endif //STATICCOLLISIONSYSTEM_H