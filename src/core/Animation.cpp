// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animation.h>


namespace magique
{
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

    void EntityAnimation::setOffset(Point newOffset) { offset = newOffset; }

    bool EntityAnimation::hasAnimation(AnimationState state) const
    {
        if ((size_t)state >= animations.width())
        {
            return false;
        }
        const auto& anim = animations[static_cast<int>(state)];
        return anim.isValid();
    }

    const SparseRangeVector<SpriteAnimation>& EntityAnimation::getAnimations() const { return animations; }


} // namespace magique
