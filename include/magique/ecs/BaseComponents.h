#ifndef BASECOMPONENTS_H
#define BASECOMPONENTS_H

#include <magique/core/Types.h>

enum class EntityType : uint16_t;

enum LightStyle : uint8_t
{
    POINT_LIGHT_SOFT,
};

// Shape classes
enum Shape : uint8_t
{
    CIRCLE, // Circle
    AABB,   // Non-rotated retangle
    POLY,   // Polygone
};


// Denotes an actor
struct ActorC
{
};

// Denotes the camera holder - can only be 1!
struct CameraC
{
};

// Position
struct PositionC
{
    float x;
    float y;
    EntityType type;
    magique::MapID map;
    uint16_t rotation = 0;
};


struct CollisionC
{
    Shape shape = AABB; // Shape
    uint16_t width = 0;
    uint16_t height = 0;
    int16_t anchorX = 0; // Rotation anchor point for the hitbox
    int16_t anchorY = 0; // Rotation anchor point for the hitbox
};

// If added entity will emit light
struct EmitterC
{
    uint8_t r, g, b;
    uint8_t intensity = 1;
    LightStyle style = POINT_LIGHT_SOFT;
};

// If added entity will throw shadows
struct OccluderC
{
    int width;
    int height;
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