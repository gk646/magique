// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_STATIC_COLLISION_H
#define MAGIQUE_STATIC_COLLISION_H

#include <vector>
#include <magique/core/Types.h>

//===============================================
// Static Collision Module
//===============================================
// ................................................................................
// This module is for defining and controlling static collision elements
// There are multiple supported ways for adding static collision:
//      - World Bounds: Makes everything outside the given rectangle solid (has to be big enough for all loaded maps)
//      - Tile Objects: Manually place colliders in the tile editor - load them as colliders with the TileMap getters
//      - TileSet     : Allows to define certain tile-indices as collidable  (you then also need to load the maps)
//      - Groups      : User managed groups that can be added and removed manually
// Note: The methods can be used in any combination, they all work together and do NOT have runtime overhead when ignored
// Note: Collideable means that a collision is detected and the event method called. Simulating something as solid is done
//       through calling AccumulateCollision() in the EntityScript
// ................................................................................

namespace magique
{
    //================= WORLD BOUNDS =================//

    // Sets static collision bounds - this is only useful for simpler (static) scenes
    // Everything outside the rectangle is considered solid - pass a width or height of 0 to disable
    // Default: Disabled
    void SetStaticWorldBounds(const Rectangle& rectangle);

    //================= TILE OBJECTS =================//

    // Loads the given tile-objects as static colliders - probably from TileMap::getObjects()
    // This can be called at any time and has immediate effect - recommended at startup as many objects get expensive
    // Passed vector is saved and future calls with it are skipped (unless removed)
    // Note: If you applied scaling to the TileSheet needs to be applied here as well to match
    void AddTileMapObjects(MapID map, const std::vector<TileObject>& collisionObjects, float scale = 1);

    // Removes the TileObjects added by the given vector
    // This method usually does not need to be called but there might be niche uses cases
    void RemoveTileMapObjects(MapID map, const std::vector<TileObject>& collisionObjects);

    //================= TILESET =================//

    // Sets the global tileset and allows to specify which class numbers mark a collidable tile
    // Note: In Tiled click on the tileset file -> select any tiles that should be solid and set the class property (e.g. 1)
    // Note: Can also be used for non-solid tiles to define special areas (water, slime, poison, ...)
    // Note: Supports the Tile Collision Editor (only Rectangles!) - allows to define a custom collision area per tile
    void LoadGlobalTileSet(const TileSet& tileSet, const std::vector<int>& markedClasses, float scale = 1);

    // Parses the selected layers of the tile data of the given map  and inserts correct static colliders for marked tiles
    // Note: Using this is only possible if set a global tileset with LoadGlobalTileSet()
    // Once set all calls with the same map are skipped (because there's only 1 tilemap per map)
    //       - layers: specifies which layers to parse (e.g. what layers contain collidable tiles: background, ...)
    void AddTileCollisions(MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers);

    // Removes the tile collision data associated with this map
    void RemoveTileCollisions(MapID map);

    //================= MANUAL GROUPS =================//

    struct ManualColliderGroup final
    {
        ManualColliderGroup();

        // Adds a rect collider with the given dimensions (topleft and dimensions)
        void addRect(float x, float y, float width, float height);

        // Adds a rect collider centered on the given position
        void addRectCentered(float x, float y, float width, float height);

        // Returns the id of the collider group
        [[nodiscard]] int getID() const;

        // Returns the current colliders part of the group
        [[nodiscard]] const std::vector<StaticCollider>& getColliders() const;


    private:
        std::vector<StaticCollider> colliders;
        int id = -1;
    };

    // Adds all colliders of the given group to the specified map
    void AddColliderGroup(MapID map, const ManualColliderGroup& group);

    // Removes all colliders of the given group from the specified map (if exists)
    void RemoveColliderGroup(MapID map, const ManualColliderGroup& group);

} // namespace magique


#endif //MAGIQUE_STATIC_COLLISION_H