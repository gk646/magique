#ifndef MAGIQUE_PATHFINDING_H
#define MAGIQUE_PATHFINDING_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Pathfinding Module
//-----------------------------------------------
// .....................................................................
// This module allows to find paths using the collision data provided by dynamic entities
// and the StaticCollision module. It uses A-Star and works with a search grid.
// Note: The grid size is configured at compile time in magique/config.h
// .....................................................................

namespace magique
{
    //----------------- PATH -----------------//

    // Assigns the (middle points) cells along the shortest path to the given vector - excluding the start tile
    //      - searchLen: stops searching for a better path after that many iterations
    //      - dynamic  : if true avoids tiles occupied by other dynamic entities (with CollisionC component)
    // Note: The point list is in REVERSE order!
    // Failure: if no path can be found returns an empty vector
    void FindPath(std::vector<Point>& path, Point start, Point end, MapID map, int searchLen = 64, bool dynamic = false);

    // Finds the next position you should move to, in order to reach the end point the fastest
    // Same as FindPath() but returns the next point
    Point GetNextOnPath(Point start, Point end, MapID map, int searchLen = 64, bool dynamic = false);

    // Same as FindPath() but allows to specify the dimensions of the searching entity
    // The pathfinding tries to find a path that fits the entity
    void FindPathEx(std::vector<Point>& path, Point start, Point end, Point dimensions, int searchLen = 64);

    //----------------- RAYCAST -----------------//

    // Returns true if the ray cast from start to the end hits something a solid tile
    bool GetRayCast(Point start, Point end);

    //----------------- UTIL -----------------//

    // If enabled, updates the dynamic grid
    // Default: On
    void SetEnableCollidableDynamicEntities(bool value);

    // Returns a normalized direction vector that points from the current to the target position
    // This is useful for moving the entity towards the next tile (pos.x += direction.x * movementSpeed)
    Point GetDirectionVector(Point current, Point target);

    //----------------- DEBUG -----------------//

    // Draws the current state of the pathfinding grid
    // Transparent (not solid), grey (solid from static source), black (solid from dynamic source)
    void DrawPathFindingGrid(MapID map);

    // Visually draws the tiles
    void DrawPath(const std::vector<Point>& path);

} // namespace magique

#endif //MAGIQUE_PATHFINDING_H