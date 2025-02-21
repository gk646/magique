// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animations.h>
#include <magique/core/Draw.h>
#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>

namespace magique
{
    float PositionC::getRotation() const { return rotation; }

    Point PositionC::getPosition() const { return {x, y}; }

    Point PositionC::getMiddle(const CollisionC& collisionC) const
    {
        Point targetPosition{x, y};
        switch (collisionC.shape)
        {
        case Shape::RECT:
            targetPosition.x += collisionC.p1 / 2.0F;
            targetPosition.y += collisionC.p2 / 2.0F;
            break;
        case Shape::CIRCLE:
            targetPosition.x += collisionC.p1;
            targetPosition.y += collisionC.p1;
            break;
        case Shape::CAPSULE:
            targetPosition.x += collisionC.p1;
            targetPosition.y += collisionC.p2 / 2.0F;
            break;
        case Shape::TRIANGLE:
            break;
        }
        return targetPosition;
    }

    //----------------- ANIMATION -----------------//

    AnimationC::AnimationC(const EntityAnimation& animation, const AnimationState startState) :
        entityAnimation(&animation), currentAnimation({}), spriteCount(0), animationStart(0),
        lastState(AnimationState{UINT8_MAX}), currentState(AnimationState{UINT8_MAX})
    {
        setAnimationState(startState);
    }

    void AnimationC::drawCurrentFrame(const float x, const float y, const float rotation, const bool flipX,
                                      const bool flipY) const
    {
        const auto currentFrame = currentAnimation.getCurrentFrame(spriteCount);
        const Rectangle dest = {x + static_cast<float>(currentAnimation.offX),
                                y + static_cast<float>(currentAnimation.offY),
                                static_cast<float>(flipX ? -currentFrame.width : currentFrame.width),
                                static_cast<float>(flipY ? -currentFrame.height : currentFrame.height)};
        DrawRegionPro(currentFrame, dest, rotation, currentAnimation.getAnchor());
    }

    void AnimationC::update() { ++spriteCount; }

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

    SpriteAnimation AnimationC::getCurrentAnimation() const { return currentAnimation; }

    bool AnimationC::getHasAnimationPlayed() const
    {

        return spriteCount > (animationStart + currentAnimation.duration * currentAnimation.sheet.frames);
    }

    AnimationState AnimationC::getCurrentState() const { return currentState; }

    uint16_t AnimationC::getSpriteCount() const { return spriteCount; }

    //----------------- COLLISION -----------------//

    bool IsValidLayer(const CollisionLayer layer)
    {
        const auto layerNum = static_cast<uint8_t>(layer);
        // Binary: 1000  (-1)-> 0111 (& operator)-> 1000
        //                                          0111   = 0 - Will never have any overlap if power of 2!
        return layerNum != 0 && (layerNum & (layerNum - 1)) == 0;
    }

    void CollisionC::unsetAll() { layerMask = 0; }

    void CollisionC::clearCollisionData()
    {
        lastNormal = {0, 0};
        resolutionVec = {0, 0};
    }

    void CollisionC::setLayer(const CollisionLayer layer, const bool enabled)
    {
        if (IsValidLayer(layer)) [[likely]]
        {
            if (enabled)
            {
                layerMask |= static_cast<uint8_t>(layer);
            }
            else
            {
                layerMask &= ~static_cast<uint8_t>(layer);
            }
        }
        else
        {
            LOG_WARNING("Trying to assign invalid collision layer! Skipping");
        }
    }

    bool CollisionC::getIsLayerSet(const CollisionLayer layer) const
    {
        if (IsValidLayer(layer))
        {
            return (layerMask & static_cast<uint8_t>(layer)) != 0;
        }
        LOG_WARNING("Trying to assign invalid collision layer! Skipping");
        return false;
    }
} // namespace magique