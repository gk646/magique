// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ANIMATIONS_H
#define MAGIQUE_ANIMATIONS_H

#include <magique/core/Types.h>
#include <magique/util/Datastructures.h>

//===============================
// Animation Module
//===============================
// ................................................................................
// This module allows to define animations, which means mapping animation states to a sprite sheet
// This animation data is meant to be shared across entities as it contains no state just data
// The intended workflow is:
//                  1. Create a EntityAnimation and add animations for the states it has (save it globally)
//                  2. Use ComponentGive<AnimationC>(entity, animation) or else to pass them a reference of the data
// Notes: Frame duration is in millis
// ................................................................................

enum class AnimationState : uint8_t; // User implemented to denote different animation states - shared for all animations
enum class AnimationLayer : uint8_t; // User implemented to denote different animation layers - shared for all animations

namespace magique
{
    struct EntityAnimation final
    {
        // Sets the scaling for the offset and anchor points
        EntityAnimation(float scale = 1);

        // Sets the animation for this action state with the same duration for all frames
        void addAnimation(AnimationState state, SpriteSheet sheet, int frameMillis);

        // Same as addAnimation but with extended parameters
        void addAnimationEx(AnimationState state, SpriteSheet sheet, const DurationArray& durations, Point offset = {},
                            Point anchor = {});

        // Removes the animation for this state
        void removeAnimation(AnimationState state);

        // Returns the frame of the given state
        [[nodiscard]] SpriteAnimation getCurrentAnimation(AnimationState state) const;

        // Sets/gets the draw offset applied to every frame
        [[nodiscard]] Point getOffset() const;
        void setOffset(Point offset);

        // The rotation anchor
        [[nodiscard]] Point getAnchor() const;

        bool hasAnimation(AnimationState state) const;
        const SparseRangeVector<SpriteAnimation>& getAnimations() const;

    private:
        Point offset{};
        Point anchor{};
        SparseRangeVector<SpriteAnimation> animations;
        float scale = 1.0F;
        friend struct AnimationData;
    };


    struct LayeredEntityAnimation final
    {
        LayeredEntityAnimation();

    private:
        SparseRangeVector<EntityAnimation> animations;
    };

} // namespace magique


#endif // MAGIQUE_ANIMATIONS_H
