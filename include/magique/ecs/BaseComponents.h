#ifndef BASECOMPONENTS_H
#define BASECOMPONENTS_H

#include <magique/fwd.hpp>

enum LightStyle : uint8_t
{
    POINT_LIGHT_SOFT,
};

// Shape classes
enum Shape : uint8_t
{
    CIRCLE, // Circle
    AABB,   // Non-rotated retangle
    POLY,   // Polygon
};


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
   magique::EntityScript& script;
};

// Position
struct PositionC final
{
    float x;
    float y;
    EntityID type;
    MapID map;
    uint16_t rotation;
};


struct CollisionC final
{
    Shape shape = AABB; // Shape
    uint16_t width = 0;
    uint16_t height = 0;
    int16_t anchorX = 0; // Rotation anchor point for the hitbox
    int16_t anchorY = 0; // Rotation anchor point for the hitbox
};


// If added entity will emit light
struct EmitterC final
{
    uint8_t r, g, b;
    uint8_t intensity;
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

//----------------- DEBUG -----------------//

// Only works in debug mode
struct DebugVisualsC
{
    // Draws entity as RED rectangle
};

// Only works in debug mode
struct DebugControllerC
{
    // Basic W, A, S ,D movement
};

#endif // BASECOMPONENTS_H