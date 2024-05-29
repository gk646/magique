#ifndef BASECOMPONENTS_H
#define BASECOMPONENTS_H

// Shape classes
enum Shape : uint8_t
{
    CIRCLE, // Circle
    AABB,   // Non-rotated retangle
    POLY,   // Polygone
};

enum MapID : uint8_t; // Models different levels

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
    MapID zone;
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


#endif // BASECOMPONENTS_H