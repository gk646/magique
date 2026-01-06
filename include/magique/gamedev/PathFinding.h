// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_PATHFINDING_H
#define MAGIQUE_PATHFINDING_H

#include <vector>
#include <magique/core/Types.h>
#include <raylib/raylib.h>

//===============================================
// Pathfinding Module
//===============================================
// .....................................................................
// This module allows to find paths using the collision data static collision (see core/StaticCollision.h)
// It uses A-Star and works by keeping a search grid of traversable tiles
// Note: The grid size is configured at compile time in magique/config.h
// IMPORTANT: You probably don't need to get a new path each tick! It's probably enough to call it a couple of times per second
//          => 30 times faster if you only do it 2 times per second instead of 60 (each tick) with almost same results
// .....................................................................

namespace magique
{
    //================= PATH =================//

    // Assigns the (middle points) cells along the shortest path to the given vector - excluding the start tile
    //      - pathLen: stops searching if the path length exceeds this
    // Note: The point list is in REVERSE order! (last element is the next point)
    // Failure: if no path can be found returns an empty vector
    // Returns: True if a path could be found, false if the target is a solid tile or cant be reached
    bool PathFind(std::vector<Point>& path, Point start, Point end, MapID map, Point hitbox = {}, int maxLen = 50,
                  GridMode mode = GridMode::STAR);

    // Allows to specify a custom heuristic function
    bool PathFindEx(std::vector<Point>& path, Point start, Point end, MapID map, Point hitbox,
                    PathFindHeuristicFunc heuristic, int maxLen = 50, GridMode mode = GridMode::STAR);

    // Assigns "next" to the next position you should move to, in order to reach the end point the fastest
    // Same as FindPath() but only assigns the next point
    bool PathFindNext(Point& next, Point start, Point end, MapID map, int maxLen = 50, GridMode mode = GridMode::STAR);

    //================= QUERY =================//

    // Returns true if the ray cast through the pathfinding grid does not hit solid cells (in line of sight)
    bool PathRayCast(Point start, Point end, MapID map);

    // Returns true if there exists a valid path from start to end
    bool PathExist(Point start, Point end, MapID map, int maxLen = 50, GridMode mode = GridMode::STAR);

    //================= UTIL =================//

    // Returns a randomly chosen movable position within the given area that can be reached from start
    // Failure: Returns -1
    Point PathFindRandomTarget(Point start, const Rect& area, MapID map, int iterations = 50);

    // Returns the next point on the path you should go to
    // If inside a cell in the path returns the next logical cell you should move to
    // Otherwise returns the closest cell in the path
    Point PathFindNextOnPath(const Point& pos, const Point& target, const std::vector<Point>& path);

    // If set, all entities of the given type are considered solid for pathfinding and make cells non-traversable
    // The dynamic pathfinding grid will be updated each tick with their position and collision shapes
    // IMPORTANT: Marked entities need to have the CollisionC so collision can be calculated
    void PathSetSolidType(EntityType type, bool value);
    bool PathGetIsSolidType(EntityType type);

    // Marks the given entity as solid for pathfinding - automatically removed when the entity is deleted
    // IMPORTANT: Marked entity needs to have the CollisionC so collision can be calculated
    void PathSetSolidEntity(entt::entity entity, bool value);
    bool PathGetSolidEntity(entt::entity entity);

    //================= DEBUG =================//

    // Draws the tiles along the given path in the pathfinding grid
    void PathDraw(const std::vector<Point>& path, Color color = RED);

    // Returns true if the pathfinding tile (that contains the point) is solid (cannot be walked on)
    bool PathIsSolid(const Point& pos, MapID map);

} // namespace magique

#endif //MAGIQUE_PATHFINDING_H
