#ifndef MAGIQUE_COLLISION_SYSTEM_UTIL_H
#define MAGIQUE_COLLISION_SYSTEM_UTIL_H

namespace magique
{

    inline void GetMovementDeltas()
    {
        const auto& colVec = global::ENGINE_DATA.collisionVec;
        auto& group = internal::POSITION_GROUP;
        for (const auto e : colVec)
        {
            auto [pos, col] = group.get<PositionC, CollisionC>(e);
        }
    }

    inline void AccumulateInfo(CollisionC& col, Shape other, const CollisionInfo& info)
    {
        if (other == Shape::RECT)
        {
            // As we are resolving towards x overlap first (in case of a tie) we only have to handle vertical stickies

            // Incoming Stick edge when moving top left
            if (info.normalVector.y == 1.0F && col.dirs[(int)Direction::RIGHT] == info.collisionPoint.x)
            {
                return;
            }
            // vertical sticky edge already happened
            else if (info.normalVector.x == 1.0F && col.dirs[(int)Direction::DOWN] == info.collisionPoint.x)
            {
                col.resolutionVec.y = 0;
            }

            // Incoming Stick edge when moving top right
            if (info.normalVector.y == 1.0F && col.dirs[(int)Direction::LEFT] == info.collisionPoint.x)
            {
                return;
            }
            // already happened
            else if (info.normalVector.x == -1.0F && col.dirs[(int)Direction::DOWN] == info.collisionPoint.x)
            {
                col.resolutionVec.y = 0;
            }

            // Incoming moving down left
            if (info.normalVector.y == -1.0F && col.dirs[(int)Direction::RIGHT] == info.collisionPoint.x)
            {
                return;
            }
            else if (info.normalVector.x == 1.0F && col.dirs[(int)Direction::UP] == info.collisionPoint.x)
            {
                col.resolutionVec.y = 0;
            }

            // moving down right
            if (info.normalVector.y == -1.0F && col.dirs[(int)Direction::LEFT] == info.collisionPoint.x)
            {
                return;
            }
            else if (info.normalVector.x == -1.0F && col.dirs[(int)Direction::UP] == info.collisionPoint.x)
            {
                col.resolutionVec.y = 0;
            }

            // Normal

            // Wants us to go up
            if (info.normalVector.y == -1.0F)
            {
                col.dirs[(int)Direction::UP] = info.collisionPoint.x;
            }
            else if (info.normalVector.y == 1.0F)
            {
                col.dirs[(int)Direction::DOWN] = info.collisionPoint.x;
            }
            else if (info.normalVector.x == -1.0F)
            {
                col.dirs[(int)Direction::LEFT] = info.collisionPoint.x;
            }
            else
            {
                col.dirs[(int)Direction::RIGHT] = info.collisionPoint.x;
            }
        }

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
            pos.x += col.resolutionVec.x;
            pos.y += col.resolutionVec.y;
            col.resolutionVec = {0, 0};
            for (auto& p : col.dirs)
            {
                p = 0.0F;
            }
        }
    }

} // namespace magique

#endif //MAGIQUE_COLLISION_SYSTEM_UTIL_H