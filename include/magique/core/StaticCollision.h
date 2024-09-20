#ifndef MAGIQUE_STATIC_COLLISION_H
#define MAGIQUE_STATIC_COLLISION_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Collision Module
//-----------------------------------------------
// ................................................................................
// This module is for defining and controlling static collision elements
// There are multiple supported ways for adding static collision:
//      - World Bounds: Makes everything outside the given rectangle solid (has to be big enough for all loaded maps)
//      - TileMaps: Manually place colliders in the tile editor - they are imported automatically and can be loaded in
//      - TileSet: Allows to define certain tile-indices as solid making them (you then also need to load the maps)
//      - Manual: Manually add and manage collider groups
// Note: The methods can be used in any combination, they all work together and do NOT have runtime overhead when ignored
// Note: Collidable means that a collision is detected and the event method called. Simulating something as solid is done
//       through calling AccumulateCollision() in the EntityScript
// ................................................................................

namespace magique
{
    // Sets static collision bounds - this is only useful for simpler (static) scenes
    // Everything outside the rectangle is considered solid - pass a width or height of 0 to disable
    // Default: Disabled
    void SetStaticWorldBounds(const Rectangle& rectangle);

    //----------------- TILEMAP -----------------//

    // Loads the given tile-objects as static colliders - probably from TileMap::getObjects()
    // Needs to be called whenever any actor enters a map - if the vector is already loaded its skipped!
    // All objects Will be unloaded automatically when no actors are left in the map
    // Note: You can load up to MAGIQUE_MAX_OBJECT_LAYERS many vectors for each map - only visible objects are loaded!
    // Note: If you applied scaling to the texture needs to be applied here as well
    void LoadMapColliders(MapID map, const std::vector<TileObject>& collisionObjects, float scale = 1);

    //----------------- TILESET -----------------//

    // Sets the global tileset and allows to specify which class numbers mark a collidable tile
    // Note: In Tiled click on the tileset file -> select any tiles that should be solid and set the class property (e.g. 1)
    // Note: Can also be use for non-solid tiles to define special areas (water, slime, poison, ...)
    // Note: Supports the Tile Collision Editor (only Rectangles!) - allows to define a custom collision area per tile
    void LoadGlobalTileSet(const TileSet& tileSet, const std::vector<int>& markedClasses, float scale = 1);

    // Parses the tile data of the given map and inserts correct static colliders
    // Note: You need to call LoadGlobalTileSet() first if you wanna use that as the tileset is used to lookup data
    // Needs to be called whenever any actor enters a map - if the map is already loaded its skipped!
    //       - layers: specifies which layers to parse (e.g. what layers contain collidable tiles: background, ...)
    void LoadTileMapCollisions(MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers);

    //----------------- MANUAL -----------------//

    // Manually adds a static collider to the given group
    // Note: manual colliders can only be managed on a per-group basis - not individual elements
    void AddColliderGroupRect(int group, float x, float y, float width, float height);

    // Removes all colliders that are part of the given group
    void RemoveColliderGroup(int group);

} // namespace magique


#endif //MAGIQUE_STATIC_COLLISION_H