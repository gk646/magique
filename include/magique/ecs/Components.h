#ifndef MAGIQUE_COMPONENTS_H
#define MAGIQUE_COMPONENTS_H

#include <magique/core/Types.h>

//-----------------------------------------------
// Components Module
//-----------------------------------------------
// ................................................................................
// These are the built-in ECS components
// Note that EntityID and MapID are both user defined types. They are used to identify different maps (levels, zones,...)
// and entity types (classes, groups, ...).
// Note: the 'C' stands for Component
// Note: Unless you know what your doing these should NOT be changed.
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

    // Denotes that the entity is scripted - will receive script updates
    struct ScriptC final
    {
    };

    // Position
    struct PositionC final
    {
        float x, y;        // Position of the top left corner!
        MapID map;         // The current map id of the entity
        EntityType type;   // Type of the entity
        uint16_t rotation; // Rotation in degrees clockwise starting at 12 o'clock - applied to collision if present

        [[nodiscard]] float getRotation() const;
        [[nodiscard]] Point getPosition() const;
    };

    struct CollisionC final
    {
        float p1 = 0.0F;                   // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        float p2 = 0.0F;                   // RECT: height                   / CAPSULE: height  / TRIANGLE: offsetY
        float p3 = 0.0F;                   //                                                   / TRIANGLE: offsetX2
        float p4 = 0.0F;                   //                                                   / TRIANGLE: offsetY2
        int16_t anchorX = 0;               // Rotation anchor point for the hitbox
        int16_t anchorY = 0;               // Rotation anchor point for the hitbox
        uint8_t layerMask = DEFAULT_LAYER; // Entities only collide if they share any layer
        Shape shape = Shape::RECT;         // Shape

        // Returns true if the entity has this layer is set
        [[nodiscard]] bool getIsLayerSet(CollisionLayer layer) const;

        // Sets the collision layer to the given value
        void setLayer(CollisionLayer layer, bool enabled);

        // Removes ALL collisions layers
        void unsetAll();

        void clearCollisionData(); // Clears the accumulated collision data for the next tick

        Point resolutionVec{}; // Accumulated normals * depth
        Point lastNormal{};    // Last normal
    };

    // Animation component references an animation and saves its current state
    struct AnimationC final
    {
        explicit AnimationC(const EntityAnimation& animation, AnimationState start);

        // Draws the current frame applying the offset and rotation around the defined anchor
        // Note: More complex and custom drawing can be done with the SpriteAnimation
        void drawCurrentFrame(float x, float y, float rotation = 0, bool flipX = false, bool flipY = false) const;

        // Progresses the animations - has to be called from the update method to be frame rate independent
        void update();

        // Sets a new action state - automatically reset the spritecount to 0 when a state change happens
        void setAnimationState(AnimationState state);

        //----------------- GETTERS -----------------//

        // Returns the current animation
        [[nodiscard]] SpriteAnimation getCurrentAnimation() const;

        // Returns true if the current animation played at least once
        // Useful for when you want to stop certain animations after they played once
        [[nodiscard]] bool getHasAnimationPlayed() const;

        // Returns the current animation state
        [[nodiscard]] AnimationState getCurrentState() const;

        // Returns the current sprite count
        [[nodiscard]] uint8_t getSpriteCount() const;

    private:
        const EntityAnimation* entityAnimation; // Always valid
        SpriteAnimation currentAnimation;
        uint16_t spriteCount;
        uint16_t animationStart;
        AnimationState lastState;
        AnimationState currentState;
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

#endif // MAGIQUE_COMPONENTS_H