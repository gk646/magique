#include "Scripts.h"

#include <magique/core/Core.h>
#include <magique/gamedev/PathFinding.h>
#include <magique/util/Math.h>

#include "WizardQuest.h"
#include "ecs/Components.h"

void PlayerScript::onTick(entt::entity self, bool updated)
{
    auto& anim = ComponentGet<AnimationC>(self);
    if (anim.getCurrentState() == AnimationState::JUMP && anim.getHasAnimationPlayed())
        anim.setAnimationState(AnimationState::IDLE);
}

void PlayerScript::onKeyEvent(entt::entity self)
{
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

    auto& anim = ComponentGet<AnimationC>(self);

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

void TrollScript::onTick(entt::entity self, bool updated)
{
    auto& pos = ComponentGet<PositionC>(self);
    auto& col = ComponentGet<CollisionC>(self);
    for (const auto e : GetNearbyEntities(pos.map, pos.pos, 1000))
    {
        if (EntityIsActor(e))
        {
            const auto& tarPos = ComponentGet<PositionC>(e);
            const auto& tarCol = ComponentGet<CollisionC>(e);
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