#ifndef MAGIQUE_COLLISION_H
#define MAGIQUE_COLLISION_H

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
    void LoadCollisionObjects(MapID map, const std::vector<TileObject>& collisionObjects);

    //----------------- TILESET -----------------//

    // Sets the global tileset and allows to specify which class number means a solid tile
    // Note: In Tiled click on the tileset file -> select any tiles the should be solid and set a class property (e.g. 1)
    void SetGlobalTileSet(TileSet& tileSet, int collisionClass);

    // When using collision over tile indices this needs to be called every time any actor enters a map
    // Once set all calls with the same map are quickly skipped - once a map has no actors it will be unloaded automatically
    // layers   - specifies which layers are collision
    void LoadTileMap(MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers);

    // Returns true ONLY if: a map was loaded with the given id, the tilenumber at [x,y] in any specified layer is flagged as solid
    bool IsSolidTile(MapID, int x, int y);

    //----------------- MANUAL -----------------//

    // Manually adds a static collider to the given group
    // Note: manual colliders can only be managed on a per group basis - not individual elements
    void AddStaticColliderRect(int group, float x, float y, float width, float height);
    void AddStaticColliderCircle(int group, float x, float y, float radius);

    // Removes all colliders that are part of the given group
    void RemoveColliderGroup(int group);


} // namespace magique


#endif //MAGIQUE_COLLISION_H