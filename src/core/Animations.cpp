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
                animations.resize(typeNum + 1, {});
            animations[typeNum] = animation;
            animations[typeNum].isSet = true;
        }

        [[nodiscard]] const EntityAnimation& get(const EntityType type) const
        {
            const auto& animation = animations[type];
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

    void EntityAnimation::addAnimation(AnimationState state, const SpriteSheet sheet, const int frameDuration)
    {
        const int stateNum = static_cast<int>(state);
        if (stateNum > static_cast<int>(animations.size()))
            animations.resize(stateNum + 1);
        animations[stateNum].duration = static_cast<uint16_t>(frameDuration);
        animations[stateNum].sheet = sheet;
    }

    void EntityAnimation::removeAnimation(AnimationState state)
    {
        const int stateNum = static_cast<int>(state);
        if (stateNum > static_cast<int>(animations.size()))
            animations.resize(stateNum + 1);
        animations[stateNum].duration = UINT16_MAX; // Mark as invalid
        animations[stateNum].sheet = {};
    }

    SpriteAnimation EntityAnimation::getCurrentAnimation(AnimationState state) const
    {
        MAGIQUE_ASSERT(isSet, "Dont use unregistered animations");
        return animations[static_cast<int>(state)];
    }

    //----------------- $EXPR$ -----------------//

    void RegisterEntityAnimation(const EntityType type, const EntityAnimation& animation)
    {
        ANIMATION_DATA.add(type, animation);
    }

    const EntityAnimation& GetEntityAnimation(const EntityType type) { return ANIMATION_DATA.get(type); }

} // namespace magique