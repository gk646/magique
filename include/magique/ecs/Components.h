// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_COMPONENTS_H
#define MAGIQUE_COMPONENTS_H

#include <magique/core/Types.h>

//===============================================
// Components Module
//===============================================
// ................................................................................
// These are the built-in ECS components
// Note that EntityID and MapID are both user defined types. They are used to identify different maps (levels, zones,...)
// and entity types (classes, groups, ...).
// Note: the 'C' stands for Component
// Note: Unless you know what your doing these should NOT be changed.
// ................................................................................

namespace magique
{
    // Implicitly added to each entity
    // MUST NOT be removed
    struct PositionC final
    {
        Point pos;         // Position of the top left corner (care when using circles)!
        MapID map;         // The current map id of the entity
        EntityType type;   // Type of the entity
        uint16_t rotation; // Rotation in degrees clockwise starting at 12 o'clock - applied to collision if present

        [[nodiscard]] float getRotation() const;

        // Returns the middle point by factoring in the collision shape
        [[nodiscard]] Point getMiddle(const CollisionC& col) const;

        // Returns the bounding rectangle of the entity (a rect that fully contains its shape)
        [[nodiscard]] Rect getBounds(const CollisionC& collisionC) const;

        bool operator==(const PositionC&) const;
    };

    // Denotes an actor
    struct ActorC final
    {
    };

    // Denotes the camera holder - can only be 1!
    struct CameraC final
    {
    };

    struct CollisionC final
    {
        float p1 = 0.0F;           // RECT: width  / CIRCLE: radius  / CAPSULE: radius  / TRIANGLE: offsetX
        float p2 = 0.0F;           // RECT: height / CIRCLE: radius  / CAPSULE: height  / TRIANGLE: offsetY
        float p3 = 0.0F;           //                                                   / TRIANGLE: offsetX2
        float p4 = 0.0F;           //                                                   / TRIANGLE: offsetY2
        Point offset;              // Offset from top left
        int16_t anchorX = 0;       // Rotation anchor point for the hitbox
        int16_t anchorY = 0;       // Rotation anchor point for the hitbox
        Shape shape = Shape::RECT; // Shape

        // https://www.youtube.com/watch?v=9k8cMzv0ZNo - same as Godot's layers
        CollisionLayer layer = CollisionLayer{1}; // Which layers it occupies
        CollisionLayer mask = CollisionLayer{1};  // Against which layers it collides

        // Returns the middle point of an entity with the CollisionC (PositionC is implicit)
        static Point GetMiddle(entt::entity e);

        // Returns true if the mask of this object detect the other objects layers - so if the two can collide
        [[nodiscard]] bool detects(const CollisionC& other) const;

        // Returns true if the entity has this layer is set
        [[nodiscard]] bool isLayerSet(CollisionLayer layer) const;
        void setLayer(CollisionLayer layer, bool enabled = true);

        // Returns true if the entity looks for collision on the given layer
        [[nodiscard]] bool isMaskSet(CollisionLayer layer) const;
        void setMask(CollisionLayer layer, bool enabled = true);

        // Returns the offset from the position (top left) to the middle
        [[nodiscard]] Point getMidOffset() const;

        // Should NOT be modified
        Point resolutionVec{}; // Accumulated normals * depth
        float dirs[4]{};       // X coordinates of rectangle collision - to fix sticky edges bug
    };

    // Animation component references an animation and saves its current state
    struct AnimationC final
    {
        explicit AnimationC(const EntityAnimation& animation, AnimationState start);

        // Draws the current frame applying the offset and rotation around the defined anchor
        // Note: More complex and custom drawing can be done with the SpriteAnimation
        void drawCurrentFrame(float x, float y, float rotation = 0) const;

        // Progresses the animations - has to be called from the update method to be frame rate independent
        void update();

        // Sets a new action state - automatically reset the sprite count to 0 when a state change happens
        void setAnimationState(AnimationState state);

        void setFlipX(bool flip);
        void setFlipY(bool flip);

        //================= GETTERS =================//

        // Returns the current animation
        [[nodiscard]] SpriteAnimation getCurrentAnimation() const;

        // Returns true if the current animation played at least once
        // Useful for when you want to stop certain animations after they played once
        [[nodiscard]] bool getHasAnimationPlayed() const;

        // Returns the current animation state
        [[nodiscard]] AnimationState getCurrentState() const;

        // Returns the current sprite count (in millis)
        [[nodiscard]] float getSpriteCount() const;

    private:
        const EntityAnimation* entityAnimation; // Always valid
        SpriteAnimation currentAnimation;
        float spriteCount;
        uint16_t animationStart;
        AnimationState lastState;
        AnimationState currentState;
        bool flipX, flipY;
    };


    // If added entity will emit light
    struct EmitterC final
    {
        // float radius = 50.0F; // Radius of the light
        uint16_t intensity; // Style dependant - the size of the light
        uint8_t r, g, b, a; // 0 - 255, a = transparency
        LightStyle style;
        bool active = true;
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
