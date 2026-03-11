// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animation.h>
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
                    return Rect{pos + col.offset, {col.p1, col.p2}};
                }
                else
                {
                    RECT_ROTATE_POINTS(pa, (*this), col)
                    return GetBBQuadrilateral(paX, paY);
                }
            }
        case Shape::CIRCLE:
            // Top left and diameter as w and h
            return Rect{pos + col.offset, {col.p1 * 2.0F, col.p1 * 2.0F}};
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

    AnimationC::AnimationC(const EntityAnimation& animation, const AnimationState startState) : animation(&animation)
    {
        setAnimationState(startState);
    }

    void AnimationC::drawCurrentFrame(const Point& pos, const float rotation) const
    {
        const auto currentFrame = currentAnimation.getCurrentFrame(millisCount);
        const auto offset = animation->getOffset();
        const Rectangle dest = {pos.x + offset.x, pos.y + offset.y,
                                static_cast<float>(flipX ? -currentFrame.width : currentFrame.width),
                                static_cast<float>(flipY ? -currentFrame.height : currentFrame.height)};
        DrawRegionPro(currentFrame, dest, rotation, animation->getAnchor());
    }

    void AnimationC::update() { millisCount += MAGIQUE_TICK_TIME * 1000.0F; }

    void AnimationC::setAnimationState(const AnimationState state)
    {
        if (state != currentState)
        {
            lastState = currentState;
            currentState = state;
            millisCount = 0;
            currentAnimation = animation->getCurrentAnimation(state);
            animationStart = millisCount;
        }
    }

    SpriteAnimation AnimationC::getCurrentAnimation() const { return currentAnimation; }

    bool AnimationC::getHasAnimationPlayed() const
    {
        return millisCount > animationStart + currentAnimation.durationMillis;
    }

    AnimationState AnimationC::getCurrentState() const { return currentState; }

    float AnimationC::getSpriteCount() const { return millisCount; }

    //----------------- LAYERED ANIMATION -----------------//

    void LayeredAnimationC::draw(const Point& pos, float rotation) const
    {
        for (const auto& [layer, layered] : animations)
        {
            auto& anim = *layered.animation;
            auto layerOffset = layered.offset;
            auto current = anim.getCurrentAnimation(currentState);
            auto frame = current.getCurrentFrame(millisCount);
            Point finalAnchor;
            if (globalAnchor != -1)
            {
                finalAnchor = globalAnchor;
            }
            else
            {
                finalAnchor = layerOffset + anim.getAnchor();
            }
            DrawRegionPro(frame, Rect{pos + layerOffset + anim.getOffset(), frame.getSize()}.floor(), rotation,
                          finalAnchor);
        }
    }

    void LayeredAnimationC::update() { millisCount += MAGIQUE_TICK_TIME * 1000.0F; }

    void LayeredAnimationC::setLayer(AnimationLayer layer, const EntityAnimation& animation, Point offset)
    {
        animations[layer] = {&animation, offset};
    }

    void LayeredAnimationC::setLayer(AnimationLayer layer, const LayeredEntityAnimation& animation)
    {
        animations[layer] = animation;
    }

    LayeredEntityAnimation LayeredAnimationC::getLayer(AnimationLayer layer) { return animations[layer]; }

    bool LayeredAnimationC::hasLayer(AnimationLayer layer) const { return animations.contains(layer); }

    //----------------- TextureC -----------------//

    TextureC::TextureC(TextureRegion texture, Point offset, Point anchor, int priority) :
        texture(texture), offset(offset.floor()),
        anchor(anchor == -1 ? Point{texture.getSize() / 2}.floor() : anchor.floor()), priority(priority)
    {
    }

    void TextureC::draw(Point pos, float rotation) const
    {
        DrawRegionPro(texture, Rect{pos + offset, texture.getSize()}, std::floor(rotation), anchor);
    }

    void LayeredTextureC::draw(const Point& pos, float rotation) const
    {
        for (const auto& [layer, tex] : textures)
        {
            if (!tex.texture.isValid())
                continue;
            Point finalAnchor;
            if (globalAnchor != -1)
            {
                finalAnchor = globalAnchor - tex.offset;
            }
            else
            {
                finalAnchor = tex.offset + tex.anchor;
            }
            const auto dest = Rect{pos + tex.offset, tex.texture.getSize()};
            DrawRegionPro(tex.texture, dest.floor(), std::floor(rotation), finalAnchor);
        }
    }

    void LayeredTextureC::setTexture(AnimationLayer layer, TextureC texture) { textures[layer] = texture; }

    TextureC LayeredTextureC::getTexture(AnimationLayer layer)
    {
        const auto it = textures.find(layer);
        if (it == textures.end())
        {
            return {};
        }
        return it->second;
    }

    //----------------- COLLISION -----------------//

    void CollisionC::setRectShape(const Rect& rect, Point newAnchor)
    {
        shape = Shape::RECT;
        if (newAnchor == -1)
        {
            newAnchor = rect.size() / 2;
        }
        anchor = newAnchor;
        offset = rect.pos();
        anchor.floor();
        offset.floor();
        p1 = rect.width;
        p2 = rect.height;
    }

    void CollisionC::setCircleShape(float radius)
    {
        p1 = radius;
        p2 = radius;
        shape = Shape::CIRCLE;
        anchor = radius;
    }

    void CollisionC::CenterOn(entt::entity e, Point point)
    {
        auto& pos = ComponentGet<PositionC>(e);
        auto& col = ComponentGet<CollisionC>(e);
        pos.pos = point - col.getMidOffset();
    }

    Point CollisionC::GetMiddle(const entt::entity e)
    {
        const auto& pos = magique::ComponentGet<PositionC>(e);
        auto* col = ComponentTryGet<CollisionC>(e);
        if (col == nullptr)
        {
            return pos.pos;
        }
        return pos.getMiddle(*col);
    }

    bool CollisionC::detects(const CollisionC& other) const
    {
        // We check if we search for a layer that the other has
        // 1100  - our mask
        // 0101  - other layers
        //      &
        // 0100  - at least one 1 => not 0 so true
        return mask.any_of(other.layer);
    }

    Point CollisionC::getMidOffset() const
    {
        switch (shape)
        {
        case Shape::RECT:
            return Point{p1, p2} / 2.0F + offset;
        case Shape::CIRCLE:
            return Point{p1, p1} + offset;
        case Shape::TRIANGLE:
            break;
        }
        return {0, 0};
    }

    bool CollisionC::operator==(const CollisionC& other) const
    {
        return offset == other.offset && anchor == other.anchor && shape == other.shape && mask == other.mask &&
            layer == other.layer && p1 == other.p1 && p2 == other.p2 && p3 == other.p3 && p4 == other.p4;
    }

} // namespace magique
