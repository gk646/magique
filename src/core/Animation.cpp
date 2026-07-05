// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animation.h>

namespace magique
{
    //----------------- ENTITY ANIMATION -----------------//

    Animation::Animation(const float scale) : logicScale(scale) {}

    void Animation::addAnimation(AnimationState state, const SpriteSheet sheet, const int frameMillis)
    {
        auto& animation = animations[state];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = frameMillis;
            animation.durationMillis += frameMillis;
        }
        animation.sheet = sheet;
    }

    void Animation::addAnimationEx(AnimationState state, SpriteSheet sheet, const DurationArray& durations, Point off,
                                   Point anch)
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

    void Animation::removeAnimation(AnimationState state) { animations.erase(state); }

    SpriteAnimation Animation::getAnimation(AnimationState state) const
    {
        auto it = animations.find(state);
        if (it != animations.end())
        {
            return it->second;
        }
        return {};
    }

    Point Animation::getOffset() const { return offset; }

    void Animation::setOffset(Point newOffset) { offset = newOffset; }

    Point Animation::getAnchor() const { return anchor; }

    void Animation::setAnchor(Point newAnchor) { anchor = newAnchor; }

    bool Animation::hasState(AnimationState state) const { return animations.contains(state); }

    const HashMap<AnimationState, SpriteAnimation>& Animation::getAnimations() const { return animations; }

} // namespace magique
