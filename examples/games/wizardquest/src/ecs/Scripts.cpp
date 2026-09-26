#include "WizardQuest.h"
#include "Scripts.h"
#include "ecs/Components.h"

void PlayerScript::onUpdate(entt::entity self, bool updated)
{
    auto& anim = ComponentGet<AnimationC>(self);
    if (anim.getState() == AnimationState::JUMP && anim.getHasAnimationPlayed())
        anim.setState(AnimationState::IDLE);

    auto& mov = ComponentGet<MovementC>(self);
    auto& stats = ComponentGet<EntityStatsC>(self);
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

    if (IsKeyPressed(KEY_SPACE))
    {
        anim.setState(AnimationState::JUMP);
        return;
    }

    if (anim.getState() == AnimationState::JUMP)
        return;

    if (mov.baseVelocX != 0.0f || mov.baseVelocY != 0.0f)
    {
        anim.setState(AnimationState::RUN);
    }
    else
    {
        anim.setState(AnimationState::IDLE);
    }
}

void TrollScript::onUpdate(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self);
    const auto mid = CollisionC::GetMiddle(self);
    auto nearby = EngineQuery(pos, Circle{mid, 100});
    for (const auto e : nearby)
    {
        if (EntityIsActor(e))
        {
            const auto& tarPos = ComponentGet<PositionC>(e);
            if (tarPos.map != pos.map)
                break;
            const auto tarMid = CollisionC::GetMiddle(e);
            std::vector<Point> path;
            PathFind(path, mid, tarMid, pos.map);
            if (path.empty())
                return;
            const auto moveVec = PathGetNextOnPath(mid, tarMid, path);
            pos.pos += moveVec * 1.5;
        }
    }
}
