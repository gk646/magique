#include <magique/core/Animations.h>
#include <magique/core/Animations.h>
#include <magique/ecs/Components.h>
#include <magique/util/Logging.h>

namespace magique
{
    //----------------- ANIMATION -----------------//

    void AnimationC::setAnimationState(const AnimationState state)
    {
        if (state != currentState)
        {
            lastState = currentState;
            currentState = state;
            spriteCount = 0;
            currentAnimation = entityAnimation->getCurrentAnimation(state);
            animationStart = spriteCount;
        }
    }

    AnimationC::AnimationC(const EntityAnimation& animation) :
        currentAnimation({}), entityAnimation(&animation), spriteCount(0), animationStart(0),
        lastState(AnimationState{}), currentState(AnimationState{})
    {
    }

    TextureRegion AnimationC::getCurrentFrame() const { return currentAnimation.getCurrentFrame(spriteCount); }

    bool AnimationC::getHasAnimationPlayed() const
    {

        return spriteCount > (animationStart + currentAnimation.duration * currentAnimation.sheet.frames);
    }

    AnimationState AnimationC::getCurrentState() const { return currentState; }

    void AnimationC::update() { ++spriteCount; }

    //----------------- COLLISION -----------------//

    bool isValidLayer(const CollisionLayer layer)
    {
        const auto layerNum = static_cast<uint8_t>(layer);
        // Binary: 1000  (-1)-> 0111 (& operator)-> 1000
        //                                          0111   = 0 - Will never have any overlap if power of 2!
        return layerNum != 0 && (layerNum & (layerNum - 1)) == 0;
    }

    void CollisionC::unsetAll() { layerMask = 0; }

    void CollisionC::setLayer(const CollisionLayer layer, const bool enabled)
    {
        if (isValidLayer(layer)) [[likely]]
        {
            if (enabled)
            {
                layerMask |= static_cast<uint8_t>(layer);
            }
            else
            {
                layerMask &= ~static_cast<uint8_t>(layer);
            }
        }
        else
        {
            LOG_WARNING("Trying to assign invalid collision layer! Skipping");
        }
    }

    bool CollisionC::getIsLayerSet(const CollisionLayer layer) const
    {
        if (isValidLayer(layer))
        {
            return (layerMask & static_cast<uint8_t>(layer)) != 0;
        }
        LOG_WARNING("Trying to assign invalid collision layer! Skipping");
        return false;
    }
} // namespace magique