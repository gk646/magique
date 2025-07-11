// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ANIMATIONS_H
#define MAGIQUE_ANIMATIONS_H

#include <vector>
#include <magique/core/Types.h>

//===============================
// Animation Module
//===============================
// ................................................................................
// This module allows to define animations that are shared across all entities of that type
// Every skeleton has the same underlying spritesheets, but each instance can be in a different state or progress
// Thus you only need to store the sprite progress and animation state for each entity
// The intended workflow is:
//                  1. Create a EntityAnimation and add animations for the states it has
//                  2. Register the EntityAnimation to a EntityType
//                  3. Now you can freely use it when creating entities via GiveAnimation(entity,type)
// ................................................................................

enum class AnimationState : uint8_t; // User implemented to denote different animation states - shared for all animations

namespace magique
{
    struct EntityAnimation final
    {
        // Sets the scaling for the offset and anchor points
        explicit EntityAnimation(float scale = 1);

        // Sets the animation for this action state
        //      - frameDuration: the duration in ticks of each frame of the SpriteSheet
        void addAnimation(AnimationState state, SpriteSheet sheet, int frameDuration);

        // Same as addAnimation but with extended parameters
        // Allows to set a custom offset per state (e.g. to align the animation with the hitbox correctly)
        //      - frameDuration: the duration in ticks of each frame of the SpriteSheet
        void addAnimation(AnimationState state, SpriteSheet sheet, int frameDuration, Point offset, Point anchor = {});

        // Removes the animation for this state
        void removeAnimation(AnimationState state);

        // Returns the frame of the given state
        [[nodiscard]] SpriteAnimation getCurrentAnimation(AnimationState state) const;

    private:
        std::vector<SpriteAnimation> animations;
        float scale;
        bool isSet;
        friend struct AnimationData;
    };

    // Registers the given animation
    // Note: Passed animation shouldn't be used anymore - use GetEntityAnimation()
    void RegisterEntityAnimation(EntityType type, const EntityAnimation& animation);

    // Returns the animation registered under the given type
    // Failure: returns an empty animation with warning
    EntityAnimation& GetEntityAnimation(EntityType type);

} // namespace magique


#endif //MAGIQUE_ANIMATIONS_H