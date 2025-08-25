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
            animations[static_cast<int>(type)].isSet = true;
        }

        [[nodiscard]] EntityAnimation& get(const EntityType type)
        {
            MAGIQUE_ASSERT(animations.width() > type, "No animation registered for that type!");
            auto& animation = animations[type];
            MAGIQUE_ASSERT(
                animation.isSet,
                "No animation was registered for that type. Use RegisterEntityAnimation() to register an animation");
            return animation;
        }
    };

} // namespace magique

namespace magique
{
    static AnimationData ANIMATION_DATA{};

    //----------------- ENTITY ANIMATION -----------------//

    EntityAnimation::EntityAnimation(const float scale) : scale(scale), isSet(false) {}

    void EntityAnimation::addAnimation(AnimationState state, const SpriteSheet sheet, const int frameDuration)
    {
        auto& animation = animations[static_cast<int>(state)];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = frameDuration;
            animation.maxDuration += frameDuration;
        }
        animation.sheet = sheet;
    }

    void EntityAnimation::addAnimationEx(AnimationState state, SpriteSheet sheet, const DurationArray& durations,
                                         Point offset, Point anchor)
    {
        auto& animation = animations[static_cast<int>(state)];
        for (int i = 0; i < sheet.frames; ++i)
        {
            animation.durations[i] = durations[i];
            animation.maxDuration += durations[i];
        }
        animation.sheet = sheet;
        animation.offX = static_cast<int16_t>(offset.x * scale);
        animation.offY = static_cast<int16_t>(offset.y * scale);
        animation.rotX = static_cast<int16_t>(anchor.x * scale);
        animation.rotY = static_cast<int16_t>(anchor.y * scale);
    }

    void EntityAnimation::removeAnimation(AnimationState state)
    {
        auto& animation = animations[static_cast<int>(state)];
        animation.maxDuration = UINT16_MAX; // Mark as invalid
        animation.sheet = {};
        animation.offX = static_cast<int16_t>(0);
        animation.offY = static_cast<int16_t>(0);
    }

    SpriteAnimation EntityAnimation::getCurrentAnimation(AnimationState state) const
    {
        MAGIQUE_ASSERT(isSet, "Dont use unregistered animations");
        return animations[static_cast<int>(state)];
    }

    //----------------- GLOBAL INTERFACE -----------------//

    void RegisterEntityAnimation(const EntityType type, const EntityAnimation& animation)
    {
        ANIMATION_DATA.add(type, animation);
    }

    EntityAnimation& GetEntityAnimation(const EntityType type) { return ANIMATION_DATA.get(type); }

} // namespace magique