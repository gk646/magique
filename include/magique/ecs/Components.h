// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_COMPONENTS_H
#define MAGIQUE_COMPONENTS_H

#include <magique/core/Types.h>
#include <magique/core/Animation.h>
#include <magique/util/Datastructures.h>

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
        Rotation rotation; // Rotation in degrees clockwise starting at 12 o'clock - applied to collision if present

        // Returns the middle point by factoring in the collision shape
        [[nodiscard]] Point getMiddle(const CollisionC& col) const;

        // Returns the bounding rectangle of the entity (a rect that fully contains its shape)
        [[nodiscard]] Rect getBounds(const CollisionC& collisionC) const;

        bool operator==(const PositionC&) const = default;
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
        float p1 = 0.0F;           // RECT: width  / CIRCLE: radius  / TRIANGLE: offsetX
        float p2 = 0.0F;           // RECT: height / CIRCLE: radius  / TRIANGLE: offsetY
        float p3 = 0.0F;           //                                / TRIANGLE: offsetX2
        float p4 = 0.0F;           //                                / TRIANGLE: offsetY2
        Point offset;              // Offset from top left
        Point anchor;              // Rotation anchor point for the hitbox
        Shape shape = Shape::RECT; // Shape

        // https://www.youtube.com/watch?v=9k8cMzv0ZNo - same as Godot's layers
        EnumSet<CollisionLayer> layer{CollisionLayer{1}}; // Which layers it occupies
        EnumSet<CollisionLayer> mask{CollisionLayer{1}};  // Against which layers it collides

        // Sets the values to be a rectangle - anchor is relative to the offset
        // x and y = offset / size = size / anchor = size/2
        void setRectShape(const Rect& rect, Point anchor = {-1});
        void setCircleShape(float radius);

        // Returns the middle point of the entity (or just the position if CollisionC is missing)
        static Point GetMiddle(entt::entity e);

        // Returns true if the mask of this object detect the other objects layers - so if the two can collide
        bool detects(const CollisionC& other) const;

        // Returns the offset from the position (top left) to the middle
        Point getMidOffset() const;

        bool operator==(const CollisionC& other) const;
        // Should NOT be modified
        Point resolutionVec{}; // Accumulated normals * depth
        float dirs[4]{};       // X coordinates of rectangle collision - to fix sticky edges bug
    };

    // Animation component references an animation and saves its current state
    struct AnimationC final
    {
        explicit AnimationC(const EntityAnimation& animation, AnimationState start = {});

        // Draws the current frame applying the offset and rotation around the defined anchor
        // Note: More complex and custom drawing can be done with the SpriteAnimation
        void drawCurrentFrame(const Point& pos, float rotation = 0) const;

        // Progresses the animations - has to be called from the update method to be frame rate independent
        void update();

        // Sets a new action state - automatically reset the sprite count to 0 when a state change happens
        void setAnimationState(AnimationState state);

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

        bool flipX = false;
        bool flipY = false;

    private:
        const EntityAnimation* animation = nullptr;
        SpriteAnimation currentAnimation{};
        float millisCount = 0;
        uint16_t animationStart = 0;
        AnimationState lastState{UINT8_MAX};
        AnimationState currentState{UINT8_MAX};
    };

    // A layered animation is more complex and allows to stack animation ontop of each other
    // This is useful for dynamically drawing characters where e.g. the face, clothes can change dynamically
    // Each layer is a reference to an existing EntityAnimation and layers are set for each component individually
    // All layers are then controlled via the same AnimationState, so they display the correct visuals
    struct LayeredAnimationC final
    {
        // Draws all layers in ascending order (enum value) - uses each individual anchor or this one if set
        void draw(const Point& pos, float rotation = 0) const;

        // Progresses the animations - has to be called from the update method to be frame rate independent
        void update();

        // Sets the animation for the given layer - adds the offset ontop of any existing offset
        void setLayer(AnimationLayer layer, const EntityAnimation& animation, Point offset);
        void setLayer(AnimationLayer layer, const LayeredEntityAnimation& animation);

        // Returns or creates a new animation for that layer
        LayeredEntityAnimation getLayer(AnimationLayer layer);

        bool hasLayer(AnimationLayer layer) const;

        Point globalAnchor = -1;

    private:
        HashMap<AnimationLayer, LayeredEntityAnimation> animations;
        float millisCount = 0;
        AnimationState lastState{};
        AnimationState currentState{};
    };

    // Can be used for a texture system to draw the visual representation of an entity
    struct TextureC final
    {
        TextureC() = default;
        // Sets anchor to the texture mid if invalid
        TextureC(TextureRegion texture, Point offset = {}, Point anchor = {-1});

        // Draws the texture at base size at the given position with offset and rotation around the anchor
        void draw(Point pos, float rotation) const;

        TextureRegion texture{};
        Point offset{};
        Point anchor{};
    };

    // Allows to layer textures to create more complex look
    // Layers can be set dynamically at any point
    struct LayeredTextureC final
    {
        // Draws all valid layers in ascending order (enum value) - if valid uses the global anchor over the individual ones
        void draw(const Point& pos, float rotation = 0) const;

        void setTexture(AnimationLayer layer, TextureC texture);

        // Returns the texture on the layer or a dummy
        TextureC getTexture(AnimationLayer layer);

        Point globalAnchor = -1;

    private:
        HashMap<AnimationLayer, TextureC> textures;
    };

} // namespace magique

#endif // MAGIQUE_COMPONENTS_H
