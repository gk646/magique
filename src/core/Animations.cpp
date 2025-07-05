// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/core/Animations.h>
#include <magique/internal/Macros.h>

#include "internal/datastructures/VectorType.h"

// Needs namespace cause of friend struct
namespace magique
{
    struct AnimationData final
    {
        vector<EntityAnimation> animations{};

        void add(const EntityType type, const EntityAnimation& animation)
        {
            const int typeNum = type;
            if (typeNum >= animations.size())
                animations.resize(typeNum + 1, EntityAnimation{1});
            animations[typeNum] = animation;
            animations[typeNum].isSet = true;
        }

        [[nodiscard]] EntityAnimation& get(const EntityType type)
        {
            MAGIQUE_ASSERT(animations.size() > type, "No animation registered for that type!");
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
    AnimationData ANIMATION_DATA{};

    //----------------- ENTITY ANIMATION -----------------//

    EntityAnimation::EntityAnimation(const float scale) : scale(scale), isSet(false) {}

    void EntityAnimation::addAnimation(AnimationState state, const SpriteSheet sheet, const int frameDuration)
    {
        const int stateNum = static_cast<int>(state);
        if (stateNum >= static_cast<int>(animations.size()))
            animations.resize(stateNum + 1);
        auto& animation = animations[stateNum];
        animation.duration = static_cast<uint16_t>(frameDuration);
        animation.sheet = sheet;
    }

    void EntityAnimation::addAnimation(AnimationState state, const SpriteSheet sheet, const int frameDuration,
                                       const Point offset, const Point anchor)
    {
        const int stateNum = static_cast<int>(state);
        if (stateNum >= static_cast<int>(animations.size()))
            animations.resize(stateNum + 1);
        auto& animation = animations[stateNum];
        animation.duration = static_cast<uint16_t>(frameDuration);
        animation.sheet = sheet;
        animation.offX = static_cast<int16_t>(offset.x * scale);
        animation.offY = static_cast<int16_t>(offset.y * scale);
        animation.rotX = static_cast<int16_t>(anchor.x * scale);
        animation.rotY = static_cast<int16_t>(anchor.y * scale);
    }

    void EntityAnimation::removeAnimation(AnimationState state)
    {
        const int stateNum = static_cast<int>(state);
        if (stateNum > static_cast<int>(animations.size()))
            animations.resize(stateNum + 1);
        auto& animation = animations[stateNum];
        animation.duration = UINT16_MAX; // Mark as invalid
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