#ifndef MAGIQUE_PATHFINDING_H
#define MAGIQUE_PATHFINDING_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Pathfinding Module
//-----------------------------------------------
// .....................................................................
//
// .....................................................................

namespace magique
{
    // Finds the next tile you should move to to reach the end point the fastest
    // Equal to the next point when searching the shortest path
    Point GetNextTile(Point start, Point end);

    // Finds and assigns the tiles along the shortest path to the given vector - excluding the start tile
    // Failure: if no path can be found return an empty vector
    void GetShortestPath(std::vector<Point>& path, Point start, Point end);

} // namespace magique

#endif //MAGIQUE_PATHFINDING_H