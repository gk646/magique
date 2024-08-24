#ifndef SCRIPTS_H
#define SCRIPTS_H

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
    void onStaticCollision(entt::entity self, const CollisionInfo &info, ColliderInfo collider) override
    {
        ResolveCollision(GetComponent<PositionC>(self),info);
    }
};
#endif //SCRIPTS_H