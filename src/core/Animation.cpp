// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animation.h>

namespace magique
{
    //----------------- ENTITY ANIMATION -----------------//

    EntityAnimation::EntityAnimation(const float scale) : logicScale(scale) {}

    void EntityAnimation::addAnimation(AnimationState state, const SpriteSheet sheet, const int frameMillis)
    {
        auto& animation = animations[state];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = frameMillis;
            animation.durationMillis += frameMillis;
        }
        animation.sheet = sheet;
    }

    void EntityAnimation::addAnimationEx(AnimationState state, SpriteSheet sheet, const DurationArray& durations,
                                         Point off, Point anch)
    {
        auto& animation = animations[state];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = durations[i];
            animation.durationMillis += durations[i];
        }
        animation.sheet = sheet;
        offset = {off.x, off.y};
        offset *= logicScale;
        anchor = {anch.x, anch.y};
        anchor *= logicScale;
    }

    void EntityAnimation::removeAnimation(AnimationState state) { animations.erase(state); }

    SpriteAnimation EntityAnimation::getCurrentAnimation(AnimationState state) const
    {
        auto it = animations.find(state);
        if (it != animations.end())
        {
            return it->second;
        }
        return {};
    }

    Point EntityAnimation::getOffset() const { return offset; }

    void EntityAnimation::setOffset(Point newOffset) { offset = newOffset; }

    Point EntityAnimation::getAnchor() const { return anchor; }

    bool EntityAnimation::hasAnimation(AnimationState state) const { return animations.contains(state); }

    const HashMap<AnimationState, SpriteAnimation>& EntityAnimation::getAnimations() const { return animations; }

} // namespace magique
