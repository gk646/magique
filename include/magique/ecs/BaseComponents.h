#ifndef BASECOMPONENTS_H
#define BASECOMPONENTS_H

// Shape classes
enum Shape : uint8_t
{
    CIRCLE, // Circle
    AABB, // Non-rotated retangle
    POLY, // Polygone
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
    int16_t width;
    int16_t height;
};


struct CollisionC
{
    Shape shape = AABB; // Shape
    int16_t anchorX = 0; // Rotation anchor point for the hitbox
    int16_t anchorY = 0; // Rotation anchor point for the hitbox
};


#endif // BASECOMPONENTS_H