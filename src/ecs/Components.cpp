// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animations.h>
#include <magique/ecs/ECS.h>
#include <magique/core/Draw.h>
#include <magique/ecs/Components.h>

#include "internal/utils/CollisionPrimitives.h"

namespace magique
{
    Point PositionC::getMiddle(const CollisionC& col) const { return pos + col.getMidOffset(); }

    Rect PositionC::getBounds(const CollisionC& col) const
    {
        switch (col.shape)
        {
        [[likely]] case Shape::RECT:
            {
                if (rotation == 0) [[likely]]
                {
                    return {pos + col.offset, {col.p1, col.p2}};
                }
                else
                {
                    RECT_ROTATE_POINTS(pa, (*this), col)
                    return GetBBQuadrilateral(paX, paY);
                }
            }
        case Shape::CIRCLE:
            // Top left and diameter as w and h
            return {pos + col.offset, {col.p1 * 2.0F, col.p1 * 2.0F}};
        case Shape::TRIANGLE:
            {
                if (rotation == 0)
                {
                    return GetBBTriangle(pos.x, pos.y, pos.x + col.p1, pos.y + col.p2, pos.x + col.p3, pos.y + col.p4);
                }
                TRI_ROTATE_POINTS(pa, (*this), col);
                return GetBBTriangle(paX[0], paY[0], paX[1], paY[1], paX[2], paY[2]);
            }
        }
        return {};
    }


    //----------------- ANIMATION -----------------//

    AnimationC::AnimationC(const EntityAnimation& animation, const AnimationState startState) :
        entityAnimation(&animation), currentAnimation({}), spriteCount(0), animationStart(0),
        lastState(AnimationState{UINT8_MAX}), currentState(AnimationState{UINT8_MAX}), flipX(false), flipY(false)
    {
        setAnimationState(startState);
    }

    void AnimationC::drawCurrentFrame(const float x, const float y, const float rotation) const
    {
        const auto currentFrame = currentAnimation.getCurrentFrame((int)spriteCount);
        auto offset = entityAnimation->getOffset();
        const Rectangle dest = {x + offset.x, y + offset.y,
                                static_cast<float>(flipX ? -currentFrame.width : currentFrame.width),
                                static_cast<float>(flipY ? -currentFrame.height : currentFrame.height)};
        DrawRegionPro(currentFrame, dest, rotation, entityAnimation->getAnchor());
    }

    void AnimationC::update() { spriteCount += MAGIQUE_TICK_TIME * 1000.0F; }

    void AnimationC::setAnimationState(const AnimationState state)
    {
        if (state != currentState)
        {
            lastState = currentState;
            currentState = state;
            spriteCount = 0;
            currentAnimation = entityAnimation->getCurrentAnimation(state);
            animationStart = spriteCount;
        }
    }

    void AnimationC::setFlipX(bool flip) { flipX = flip; }

    void AnimationC::setFlipY(bool flip) { flipY = flip; }

    SpriteAnimation AnimationC::getCurrentAnimation() const { return currentAnimation; }

    bool AnimationC::getHasAnimationPlayed() const
    {
        return spriteCount > animationStart + currentAnimation.maxDuration;
    }

    AnimationState AnimationC::getCurrentState() const { return currentState; }

    float AnimationC::getSpriteCount() const { return spriteCount; }

    //----------------- COLLISION -----------------//

    Point CollisionC::GetMiddle(const entt::entity e)
    {
        const auto& pos = magique::ComponentGet<PositionC>(e);
        const auto& col = magique::ComponentGet<CollisionC>(e);
        return pos.getMiddle(col);
    }

    bool CollisionC::detects(const CollisionC& other) const
    {
        // We check if we search for a layer that the other has
        // 1100  - our mask
        // 0101  - other layers
        //      &
        // 0100  - at least one 1 => not 0 so true
        return mask.isSet(other.layer.get());
    }

    Point CollisionC::getMidOffset() const
    {
        switch (shape)
        {
        case Shape::RECT:
            return (Point{p1, p2} / 2.0F) + offset;
        case Shape::CIRCLE:
            return Point{p1, p2} + offset;
        case Shape::TRIANGLE:
            break;
        }
        return {0, 0};
    }

} // namespace magique
