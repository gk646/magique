#pragma once
#ifndef MAGIQUE_COMPONENTS_H
#define MAGIQUE_COMPONENTS_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Components Module
//-----------------------------------------------
// .....................................................................
// These are the built in components
// They are quite basic and shouldnt limit
// Note that EntityID and MapID are user defined types. These represent the notion of different type
// of maps (levels, zones...) and entity types (classes...).
// .....................................................................

enum LightStyle : uint8_t
{
    POINT_LIGHT_SOFT,
};

// Shape classes
enum Shape : uint8_t
{
    CIRCLE, // Circle
    RECT,   // Rectangle, can be rotated
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
};

// Position
struct PositionC final
{
    float x;
    float y;
    EntityID type;
    MapID map;
    uint16_t rotation; // Rotation in degress clockwise starting at 12 o'clock - applies to collision if present
};

struct CollisionC final
{
    Shape shape = RECT; // Shape
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

#endif // MAGIQUE_COMPONENTS_H