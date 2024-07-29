#ifndef MAGIQUE_COMPONENTS_H
#define MAGIQUE_COMPONENTS_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Components Module
//-----------------------------------------------
// ................................................................................
// These are the built in components
// They are quite basic and shouldnt limit
// Note that EntityID and MapID are user defined types. These represent the notion of different type
// of maps (levels, zones...) and entity types (classes...).
// ................................................................................

namespace magique
{
    // Denotes an actor
    struct ActorC final
    {
    };

    // Denotes the camera holder - can only be 1!
    struct CameraC final
    {
    };

    // Denotes that the entity is scripted - will recieve script updates
    struct ScriptC final
    {
    };

    // Position
    struct PositionC final
    {
        float x, y;        // Position of the top left corner!
        MapID map;         // The current map id of the entity
        EntityID type;     // Type of the entity
        uint16_t rotation; // Rotation in degress clockwise starting at 12 o'clock - applied to collision if present
    };

    struct CollisionC final
    {
        float p1 = 0.0F;                   // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        float p2 = 0.0F;                   // RECT: height                   / CAPSULE: heigth  / TRIANGLE: offsetY
        float p3 = 0.0F;                   //                                                   / TRIANGLE: offsetX2
        float p4 = 0.0F;                   //                                                   / TRIANGLE: offsetY2
        int16_t anchorX = 0;               // Rotation anchor point for the hitbox
        int16_t anchorY = 0;               // Rotation anchor point for the hitbox
        uint8_t layerMask = DEFAULT_LAYER; // Entities only collide if they share any layer
        Shape shape = Shape::RECT;         // Shape

        // Returns true if the entity has this layer enabled
        [[nodiscard]] bool isLayerEnabled(CollisionLayer layer) const;

        // Sets the collision layer to the given value
        void setLayer(CollisionLayer layer, bool enabled);

        // Removes ALL collisions layers
        void unsetAll();
    };

    template <typename ActionStateEnum = ActionState> // Supply your own action state enum if needed
    struct AnimationC final
    {
        Animation animations[ActionStateEnum::STATES_END]{}; // Custom enums need that end value
        uint16_t currentSpriteCount = 0;
        ActionStateEnum lastActionState;

        // Sets the animation for this action state
        void addAnimation(ActionStateEnum state, SpriteSheet sheet, uint16_t frameDuration);

        // Removes the animation for this state
        void removeAnimation(ActionStateEnum state);

        // Draws the current sprite
        void draw();

        // Progresses the sprite - should be called on the update thread to be frame rate independant
        void update();

        // Sets a new action state
        void setActionState(ActionStateEnum actionState);
    };

    // If added entity will emit light
    struct EmitterC final
    {
        uint8_t r, g, b, a; // 0 - 255, a = transparency
        uint16_t intensity; // Style dependant - the size of the light
        LightStyle style;
    };

    // If added entity will throw shadows
    // Can be different dimensions and shape than collision
    struct OccluderC final
    {
        int16_t width;
        int16_t height;
        Shape shape;
    };

} // namespace magique

//----------------- IMPLEMENTATION -----------------//

#include <magique/internal/Macros.h>

template <typename ActionStateEnum>
void magique::AnimationC<ActionStateEnum>::addAnimation(ActionStateEnum state, magique::SpriteSheet sheet,
                                                        uint16_t frameDuration)
{
    M_ASSERT(state < ActionStateEnum::STATES_END, "Given enum is bigger than the max value!");
    auto& anim = animations[(int)state];
    anim.duration = frameDuration;
    anim.sheet = sheet;
}
template <typename ActionStateEnum>
void magique::AnimationC<ActionStateEnum>::removeAnimation(ActionStateEnum state)
{
    M_ASSERT(state < ActionStateEnum::STATES_END, "Given enum is bigger than the max value!");
    auto& anim = animations[(int)state];
    anim.duration = UINT16_MAX;
    anim.sheet = magique::SpriteSheet{};
}
template <typename ActionStateEnum>
void magique::AnimationC<ActionStateEnum>::draw()
{
    auto frame = animations[3].getCurrentTexture(currentSpriteCount);
}
template <typename ActionStateEnum>
void magique::AnimationC<ActionStateEnum>::update()
{
    currentSpriteCount++;
}
template <typename ActionStateEnum>
void magique::AnimationC<ActionStateEnum>::setActionState(ActionStateEnum actionState)
{
    if (lastActionState != actionState)
    {
        lastActionState = actionState;
        currentSpriteCount = 0;
    }
}

#endif // MAGIQUE_COMPONENTS_H