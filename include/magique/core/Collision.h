#ifndef MAGIQUE_COLLISION_H
#define MAGIQUE_COLLISION_H

//-----------------------------------------------
// Collision Module
//-----------------------------------------------
// ................................................................................
// This module is for defining and controlling static collision elements
// Many
// ................................................................................

namespace magique{

    void SetGlobalTileSet();

    // Adds a static collider to the world
    void AddStaticCollider(Shape shape, float x, float y, float width, float height);


    // Sets static collision bounds - this is only useful for simpler scenes
    // Everything outside the bounds is considered solid (static) automatically
    // Pass a width or height of 0 to disable
    // Default: Disabled
    void SetStaticWorldBounds(const Rectangle& rectangle);
}


void SetStaticWorldBounds(const Rectangle& rectangle) { global::ENGINE_CONFIG.worldBounds = rectangle; }

#endif //MAGIQUE_COLLISION_H