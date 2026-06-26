// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STATIC_COLLISION_H
#define MAGIQUE_STATIC_COLLISION_H

#include <vector>
#include <magique/core/Types.h>

//===============================================
// StaticCollision
//===============================================
// ................................................................................
// This module is for defining and controlling static collision elements
// There are multiple supported ways for adding static collision:
//      - World Bounds: Makes everything outside the given rectangle solid (has to be big enough for all loaded maps)
//      - Tile Objects: Manually place colliders in the tile editor - load them as colliders with the TileMap getters
//      - TileSet     : Allows to define certain tile-indices as collidable  (you then also need to load the maps)
//      - Groups      : User managed groups that can be added and removed manually
// Note: The methods can be used in any combination, they all work together and do NOT have runtime overhead when ignored
// ................................................................................

namespace magique
{
    //================= WORLD BOUNDS =================//

    // Sets static collision bounds - this is only useful for simpler (static) scenes
    // Everything outside the rectangle is considered solid - pass a width or height of 0 to disable
    // Default: Disabled
    void CollisionSetWorldBounds(const Rectangle& rectangle);

    //================= TILESET =================//

    // Sets the global tileset to use for parsing tile collision and other features
    // Note: Can also be used for non-solid tiles to define special areas (water, slime, poison, ...)
    // Note: Supports the Tile Collision Editor (only Rectangles!) - allows to define a custom collision area per tile
    void CollisionSetTileset(const TileSet& tileSet, float scale = 1);

    // Parses the selected layers of the tile data of the given map and inserts correct static colliders for marked tiles
    // Note: Using this is only possible if set a global tileset with CollisionSetTileset()
    // Once set all calls with the same map are skipped (because there's only 1 tilemap per map)
    //       - layers: specifies which layers to parse (e.g. what layers contain collidable tiles: background, ...)
    void CollisionAddTiles(MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers);

    // Removes the tile collision data associated with this map
    void CollisionRemoveTiles(MapID map);

} // namespace magique


#endif // MAGIQUE_STATIC_COLLISION_H
