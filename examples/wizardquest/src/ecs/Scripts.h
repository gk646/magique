#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "ScriptUtil.h"

struct PlayerScript final : EntityScript
{
    void onTick(entt::entity self) override
    {
        auto& anim = GetComponent<AnimationC>(self);
        if (anim.currentState == AnimationState::JUMP && anim.getHasAnimationPlayed())
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

        auto& anim = GetComponent<AnimationC>(self);

        if (IsKeyPressed(KEY_SPACE))
        {
            anim.setAnimationState(AnimationState::JUMP);
            return;
        }

        if (anim.currentState == AnimationState::JUMP)
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

    void onStaticCollision(entt::entity self, ColliderInfo collider, const CollisionInfo& info) override
    {
        if (collider.type == ColliderType::TILEMAP_OBJECT)
        {
            printf("Class: %d\n", collider.getColliderClass());
            if (collider.getColliderClass() == 1)
            {
                return;
            }
        }
        if (collider.type == ColliderType::TILESET_TILE)
            return;
        AccumulateCollision(self, info);
    }
};
#endif //SCRIPTS_H