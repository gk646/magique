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
// Note: The methods can be used in any combination, they all work together
// ................................................................................

namespace magique
{
    // Sets static collision bounds - this is only useful for simpler (static) scenes
    // Everything outside the rectangle is considered solid - pass a width or height of 0 to disable
    // Default: Disabled
    void SetStaticWorldBounds(const Rectangle& rectangle);

    //----------------- TILEMAP -----------------//

    // Loads the given tile-objects as static colliders - probably from TileMap::getObjects()
    // This needs to be called whenever any actor enters a map - once set further calls for the same map are skipped
    // Will be unloaded automatically when no actors are left in the map
    void LoadMapColliders(MapID map, const std::vector<TileObject>& collisionObjects);

    //----------------- TILESET -----------------//

    // Sets the global tileset and allows to specify which class number means a solid tile - also the global tilesize
    // Note: In Tiled click on the tileset file -> select any tiles the should be solid and set the class property (e.g. 1)
    void SetGlobalTileSet(const TileSet& tileSet, int collisionClass, float tileSize);

    // Loads map data so positions can be looked up
    // Load all maps at the start or load the new map when a actor enters it - duplicate calls dont matter
    // layers   - specifies which layers are collision
    void LoadTileMap(MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers);

    // Returns true if the given map was loaded and the tile at (x,y) in any specified layer is marked as solid in the global tileset
    // Note: For this to work both SetGlobalTilset() AND LoadTileMap() have to be setup correctly
    bool IsSolidTile(MapID map, int tileX, int tileY);

    //----------------- MANUAL -----------------//

    // Manually adds a static collider to the given group
    // Note: manual colliders can only be managed on a per group basis - not individual elements
    void AddColliderGroupRect(int group, float x, float y, float width, float height);

    // Removes all colliders that are part of the given group
    void RemoveColliderGroup(int group);

} // namespace magique


#endif //MAGIQUE_STATIC_COLLISION_H