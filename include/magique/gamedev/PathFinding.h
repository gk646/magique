#ifndef MAGIQUE_PATHFINDING_H
#define MAGIQUE_PATHFINDING_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Pathfinding Module
//-----------------------------------------------
// .....................................................................
// This module allows to find paths using the collision data static collision (see core/StaticCollision.h)
// It uses A-Star and works by keeping a search grid of traversable tiles
// Note: The grid size is configured at compile time in magique/config.h
// Note: Watch out if you have a low MAGIQUE_PATHFINDING_CELL_SIZE and low 'searchLen' - there might be a path but its too long!
// .....................................................................

namespace magique
{
    //----------------- PATH -----------------//

    // Assigns the (middle points) cells along the shortest path to the given vector - excluding the start tile
    //      - searchLen: stops searching for a better path after that many iterations
    // Note: The point list is in REVERSE order!
    // Failure: if no path can be found returns an empty vector
    void FindPath(std::vector<Point>& path, Point start, Point end, MapID map, int searchLen = 64);

    // Finds the next position you should move to, in order to reach the end point the fastest
    // Same as FindPath() but returns the next point
    // Failure: returns {0,0} if no path can be found or the search took too many iterations or target is not traversable
    Point GetNextOnPath(Point start, Point end, MapID map, int searchLen = 64);

    //----------------- QUERY -----------------//

    // Returns true if the ray cast through the pathfinding grid only hits traversable tiles
    bool GetPathRayCast(Point start, Point end, MapID map);

    //----------------- UTIL -----------------//

    // If set, all entities of the given type are considered solid for pathfinding and make cells non-traversable
    // The dynamic pathfinding grid will be updated each tick with their position and collision shapes
    // IMPORTANT: Marked entities need to have the CollisionC so collision can be calculated
    void SetTypePathSolid(EntityType type, bool value);
    bool GetIsTypePathSolid(EntityType type);

    // Marks the given entity as solid for pathfinding - automatically removed
    // IMPORTANT: Marked entity needs to have the CollisionC so collision can be calculated
    void SetEntityPathSolid(entt::entity entity, bool value);
    bool GetIsEntityPathSolid(entt::entity entity);

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