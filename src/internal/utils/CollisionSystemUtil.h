#ifndef MAGIQUE_COLLISION_SYSTEM_UTIL_H
#define MAGIQUE_COLLISION_SYSTEM_UTIL_H

namespace magique
{

    inline void AccumulateInfo(CollisionC& col, Shape other, const CollisionInfo& info)
    {
        const auto finalVec = info.normalVector * info.penDepth;

        // x
        if (col.resolutionVec.x > 0 && finalVec.x > 0)
        {
            col.resolutionVec.x = std::max(finalVec.x, col.resolutionVec.x);
        }
        else if (col.resolutionVec.x < 0 && finalVec.x < 0)
        {
            col.resolutionVec.x = std::min(finalVec.x, col.resolutionVec.x);
        }
        else
        {
            col.resolutionVec.x += finalVec.x;
        }

        // Y
        if (col.resolutionVec.y > 0 && finalVec.y > 0)
        {
            col.resolutionVec.y = std::max(finalVec.y, col.resolutionVec.y);
        }
        else if (col.resolutionVec.y < 0 && finalVec.y < 0)
        {
            col.resolutionVec.y = std::min(finalVec.y, col.resolutionVec.y);
        }
        else
        {
            col.resolutionVec.y += finalVec.y;
        }
    }

    inline void ResolveCollisions()
    {
        const auto& colVec = global::ENGINE_DATA.collisionVec;
        auto& group = internal::POSITION_GROUP;
        for (const auto e : colVec)
        {
            auto [pos, col] = group.get<PositionC, CollisionC>(e);
            pos.pos += col.resolutionVec;
            col.resolutionVec = {0, 0};
            for (auto& p : col.dirs)
            {
                p = 0.0F;
            }
        }
    }

} // namespace magique

#endif // MAGIQUE_COLLISION_SYSTEM_UTIL_H
