// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animations.h>
#include <magique/internal/Macros.h>

#include "internal/datastructures/VectorType.h"

// Needs namespace cause of friend struct
namespace magique
{
    struct AnimationData final
    {
        SparseRangeVector<EntityAnimation> animations;

        void add(const EntityType type, const EntityAnimation& animation)
        {
            animations[static_cast<int>(type)] = animation;
        }

        [[nodiscard]] EntityAnimation& get(const EntityType type)
        {
            MAGIQUE_ASSERT(animations.width() > type, "No animation registered for that type!");
            auto& animation = animations[type];
            return animation;
        }
    };

} // namespace magique

namespace magique
{
    static AnimationData ANIMATION_DATA{};

    //----------------- ENTITY ANIMATION -----------------//

    EntityAnimation::EntityAnimation(const float scale) : scale(scale) {}

    void EntityAnimation::addAnimation(AnimationState state, const SpriteSheet sheet, const int frameMillis)
    {
        auto& animation = animations[static_cast<int>(state)];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = frameMillis;
            animation.maxDuration += frameMillis;
        }
        animation.sheet = sheet;
    }

    void EntityAnimation::addAnimationEx(AnimationState state, SpriteSheet sheet, const DurationArray& durations,
                                         Point off, Point anch)
    {
        auto& animation = animations[static_cast<int>(state)];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = durations[i];
            animation.maxDuration += durations[i];
        }
        animation.sheet = sheet;
        offset = {off.x, off.y};
        offset *= scale;
        anchor = {anch.x, anch.y};
        anchor *= scale;
    }

    void EntityAnimation::removeAnimation(AnimationState state)
    {
        auto& animation = animations[static_cast<int>(state)];
        animation.maxDuration = UINT16_MAX; // Mark as invalid
        animation.sheet = {};
    }

    SpriteAnimation EntityAnimation::getCurrentAnimation(AnimationState state) const
    {
        return animations[static_cast<int>(state)];
    }

    Point EntityAnimation::getOffset() const { return offset; }

    Point EntityAnimation::getAnchor() const { return anchor; }

    void EntityAnimation::setOffset(Point newOffset)
    {
        offset = newOffset;
    }

    //----------------- GLOBAL INTERFACE -----------------//

    void RegisterEntityAnimation(const EntityType type, const EntityAnimation& animation)
    {
        ANIMATION_DATA.add(type, animation);
    }

    EntityAnimation& GetEntityAnimation(const EntityType type) { return ANIMATION_DATA.get(type); }

} // namespace magique