#ifndef BASECOMPONENTS_H
#define BASECOMPONENTS_H

#include <magique/core/Types.h>

enum LightStyle : uint8_t
{
    POINT_LIGHT,
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
    magique::MapID map;
    int16_t width;
    int16_t height;
    uint16_t rotation;
};


struct CollisionC
{
    Shape shape = AABB;  // Shape
    int16_t anchorX = 0; // Rotation anchor point for the hitbox
    int16_t anchorY = 0; // Rotation anchor point for the hitbox
};


struct LightC
{
    uint8_t r, g, b;
    uint8_t intensity;
    LightStyle style;
};

#endif // BASECOMPONENTS_H