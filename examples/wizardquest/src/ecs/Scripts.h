#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "ScriptUtil.h"

struct PlayerScript final : EntityScript
{
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
    }

    void onStaticCollision(entt::entity self, ColliderInfo collider, const CollisionInfo& info) override
    {
        if (collider.type == ColliderType::TILEMAP_OBJECT)
            printf("Class: %d\n", collider.data);
        AccumulateCollision(self, info);
    }
};
#endif //SCRIPTS_H