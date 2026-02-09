// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animations.h>
#include <magique/ecs/ECS.h>
#include <magique/core/Draw.h>
#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>

#include "internal/utils/CollisionPrimitives.h"

namespace magique
{
    float PositionC::getRotation() const { return rotation; }

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
                    float pxs[4] = {0, col.p1, col.p1, 0};
                    float pys[4] = {0, 0, col.p2, col.p2};
                    RotatePoints4(pos.x + col.offset.x, pos.y + col.offset.y, pxs, pys, rotation, col.anchorX,
                                  col.anchorY);
                    return GetBBQuadrilateral(pxs, pys);
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
                float txs[4] = {0, col.p1, col.p3, 0};
                float tys[4] = {0, col.p2, col.p4, 0};
                RotatePoints4(pos.x, pos.y, txs, tys, rotation, col.anchorX, col.anchorY);
                return GetBBTriangle(txs[0], tys[0], txs[1], tys[1], txs[2], tys[2]);
            }
        }
        return {};
    }

    bool PositionC::operator==(const PositionC& other) const
    {
        return pos == other.pos && map == other.map && type == other.type && rotation == other.rotation;
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

    bool CollisionC::isLayerSet(const CollisionLayer newLayer) const
    {
        return ((uint8_t)layer & static_cast<uint8_t>(newLayer)) != 0;
    }

    void CollisionC::setLayer(const CollisionLayer newLayer, const bool enabled)
    {
        SetBitflag(layer, (uint8_t)newLayer, enabled);
    }

    bool CollisionC::isMaskSet(CollisionLayer newLayer) const
    {
        return ((uint8_t)mask & static_cast<uint8_t>(newLayer)) != 0;
    }

    void CollisionC::setMask(CollisionLayer newLayer, bool enabled) { SetBitflag(mask, (uint8_t)newLayer, enabled); }

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
