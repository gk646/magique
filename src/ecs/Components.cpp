// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animations.h>
#include <magique/ecs/ECS.h>
#include <magique/core/Draw.h>
#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>

namespace magique
{
    float PositionC::getRotation() const { return rotation; }

    Point PositionC::getPosition() const { return {x, y}; }

    Point PositionC::getMiddle(const CollisionC& collisionC) const { return Point{x, y} + collisionC.getMidOffset(); }

    bool PositionC::operator==(const PositionC& other) const
    {
        return x == other.x && y == other.y && map == other.map && type == other.type && rotation == other.rotation;
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
        const auto& pos = magique::GetComponent<PositionC>(e);
        const auto& col = magique::GetComponent<CollisionC>(e);
        return pos.getMiddle(col);
    }

    bool CollisionC::detects(const CollisionC& other) const
    {
        // We check if we search for a layer that the other has
        // 1100  - our mask
        // 0101  - other layers
        //      &
        // 0100  - at least one 1 => not 0 so true
        return ((uint8_t)mask & (uint8_t)other.layer) != 0;
    }

    static bool IsValidLayer(const uint8_t layer)
    {
        // Binary: 1000  (-1)-> 0111 (& operator)-> 1000
        //                                          0111   = 0 - Will never have any overlap if power of 2!
        return layer != 0 && (layer & (layer - 1)) == 0;
    }

    static void SetBitflag(CollisionLayer& flag, const uint8_t mask, const bool set)
    {
        if (!IsValidLayer(mask)) [[unlikely]]
            LOG_WARNING("Trying to assign invalid collision layer! Skipping");

        if (set)
        {
            flag = CollisionLayer{static_cast<uint8_t>(static_cast<uint32_t>(flag) | mask)};
        }
        else
        {
            // 1010  layer
            // 0111  flipped deletion layer (1000)
            // 0010  only the deletion layer deleted
            flag = CollisionLayer{static_cast<uint8_t>(static_cast<uint32_t>(flag) & ~mask)};
        }
    }

    void CollisionC::setMask(CollisionLayer newLayer, bool enabled) { SetBitflag(mask, (uint8_t)newLayer, enabled); }

    Point CollisionC::getMidOffset() const
    {
        switch (shape)
        {
        case Shape::RECT:
            return {p1 / 2.0F + offX, p2 / 2.0F + offY};
        case Shape::CIRCLE:
            return {p1 + offX, p1 + offY};
        case Shape::CAPSULE:
            return {p1 + offX, p2 / 2.0F + offY};
        case Shape::TRIANGLE:
            break;
        }
        return {0, 0};
    }

    void CollisionC::setLayer(const CollisionLayer newLayer, const bool enabled)
    {
        SetBitflag(layer, (uint8_t)newLayer, enabled);
    }

    bool CollisionC::isMaskSet(CollisionLayer newLayer) const
    {
        return ((uint8_t)mask & static_cast<uint8_t>(newLayer)) != 0;
    }

    bool CollisionC::isLayerSet(const CollisionLayer newLayer) const
    {
        return ((uint8_t)layer & static_cast<uint8_t>(newLayer)) != 0;
    }

} // namespace magique
