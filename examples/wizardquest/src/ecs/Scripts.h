#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <magique/ecs/ECS.h>

#include "ScriptUtil.h"

struct PlayerScript final : EntityScript
{
    void onTick(entt::entity self) override
    {
        auto& anim = GetComponent<AnimationC>(self);
        if (anim.getCurrentState() == AnimationState::JUMP && anim.getHasAnimationPlayed())
            anim.setAnimationState(AnimationState::IDLE);
    }

    void onKeyEvent(entt::entity self) override
    {
        auto& mov = GetComponent<MovementC>(self);
        auto& stats = GetComponent<EntityStatsC>(self);
        if (IsKeyDown(KEY_W))
            mov.baseVelocY -= stats.moveSpeed;
        if (IsKeyDown(KEY_S))
            mov.baseVelocY += stats.moveSpeed;
        if (IsKeyDown(KEY_A))
            mov.baseVelocX -= stats.moveSpeed;
        if (IsKeyDown(KEY_D))
            mov.baseVelocX += stats.moveSpeed;

        if (mov.baseVelocX < 0)
            mov.movedLeft = true;
        else if (mov.baseVelocX > 0)
            mov.movedLeft = false;

        auto& anim = GetComponent<AnimationC>(self);

        if (IsKeyPressed(KEY_SPACE))
        {
            anim.setAnimationState(AnimationState::JUMP);
            return;
        }

        if (anim.getCurrentState() == AnimationState::JUMP)
            return;

        if (mov.baseVelocX != 0.0f || mov.baseVelocY != 0.0f)
        {
            anim.setAnimationState(AnimationState::RUN);
        }
        else
        {
            anim.setAnimationState(AnimationState::IDLE);
        }
    }

    void onStaticCollision(entt::entity self, ColliderInfo collider, CollisionInfo& info) override
    {
        const auto& pos = GetComponent<PositionC>(self);
        AccumulateCollision(info);
    }
};

struct TrollScript final : EntityScript
{
    void onTick(entt::entity self) override
    {
        return;
        auto& pos = GetComponent<PositionC>(self);
        auto& col = GetComponent<CollisionC>(self);
        for (const auto e : GetNearbyEntities(pos.map, pos.getPosition(), 1000))
        {
            if (EntityIsActor(e))
            {
                const auto& tarPos = GetComponent<PositionC>(e);
                const auto& tarCol = GetComponent<CollisionC>(e);
                if (tarPos.map != pos.map)
                    break;
                std::vector<Point> path;
                FindPath(path, pos.getMiddle(col), tarPos.getMiddle(tarCol), pos.map);
                if (path.empty())
                    return;
                const auto moveVec = GetDirectionVector(pos.getMiddle(col), path[0]) * 0.5F;
                pos.x += moveVec.x;
                pos.y += moveVec.y;
            }
        }
    }
};

#endif //SCRIPTS_H