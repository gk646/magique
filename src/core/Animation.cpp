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
        if (sheet.frames == 0)
        {
            LOG_WARNING("Cannot add sheet with 0 frames to animation");
            return;
        }

        auto& animation = animations[state];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = durations[i];
            animation.durationMillis += durations[i];
        }
        animation.sheet = sheet;
        offset = off * logicScale;
        if (anch == 0)
            anch = sheet.getRegion(0).getSize() / 2;
        anchor = anch * logicScale;
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

    void EntityAnimation::setAnchor(Point newAnchor) { anchor = newAnchor; }

    bool EntityAnimation::hasAnimation(AnimationState state) const { return animations.contains(state); }

    const HashMap<AnimationState, SpriteAnimation>& EntityAnimation::getAnimations() const { return animations; }

} // namespace magique
