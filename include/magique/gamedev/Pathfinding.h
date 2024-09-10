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
// Note: The gridsize is configured at compile time in magique/config.h
// .....................................................................

namespace magique
{
    //----------------- PATH -----------------//

    // Finds and assigns the (middle points) tiles along the shortest path to the given vector - excluding the start tile
    //      - searchLen: stops searching for a better path after that many iterations
    // Failure: if no path can be found returns an empty vector
    void GetShortestPath(std::vector<Point>& path, Point start, Point end, MapID map, int searchLen = 64);

    // Finds the next position you should move to, in order to reach the end point the fastest
    // Equal to the middle point of the next tile on the shortest path
    Point GetNextPosition(Point start, Point end, MapID map, int searchLen = 64);

    //----------------- UTIL -----------------//

    // Returns true if the ray cast from the start to the end coordinates does hits someting collidable
    //
    bool GetRayCast(Point start, Point end);

} // namespace magique

#endif //MAGIQUE_PATHFINDING_H