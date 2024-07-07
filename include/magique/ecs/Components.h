#pragma once
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
    uint8_t layerMask = 1;                // Collision layer -> Use the methods to set it
    magique::Shape shape = magique::RECT; // Shape
    uint16_t width = 0;
    uint16_t height = 0;
    int16_t anchorX = 0; // Rotation anchor point for the hitbox
    int16_t anchorY = 0; // Rotation anchor point for the hitbox

    void addCollisionLayer(magique::CollisionLayer layer);
    void removeCollisionLayer(magique::CollisionLayer layer);
    void removeAllLayers();
};

// If added entity will emit light
struct EmitterC final
{
    uint8_t r, g, b, a; // 0 - 255, a = transparency
    uint16_t intensity; // Style dependant - the size of the light
    magique::LightStyle style;
};

// If added entity will throw shadows
// Can be different dimensions and shape than collision
struct OccluderC final
{
    int16_t width;
    int16_t height;
    magique::Shape shape;
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