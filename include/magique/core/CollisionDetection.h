// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_COLLISION_DETECTION_H
#define MAGIQUE_COLLISION_DETECTION_H

#include <magique/core/Types.h>

//===============================================
// Collision Detection Module
//===============================================
// ................................................................................
// Note: This is an advanced module
// This module offers optimized collision detection methods that work for the new shapes magique adds
// The new shapes are: Rotated Rectangles, Rotated Triangles, and Capsule
// It uses the CollisionInfo struct to save information about the collision (normal, depth, collision point)
// Intended workflow:
//    1. Create a CollisionInfo struct (probably on the stack)
//    2. Call a collision method with the CollisionInfo object
//    3. Use the resulting info
//
// Check the CollisionC component and its documentation to learn how to access its dimensions based on shape
//
// Note: Do NOT reuse the collision info struct for multiple methods
// Note: Use the getters of the CollisionInfo struct to find out if a collision occurred!
// Note: Quadrilateral also includes Triangles! Just pass the first point as last point again.
// ................................................................................

namespace magique
{
    //================= ENTITY =================//

    // Performs a collision check between two entities based on their position and collision components
    // This automatically takes into account their current position, shape and dimensions!
    void CheckCollisionEntities(const PositionC& posA, const CollisionC& colA, const PositionC& posB,
                                const CollisionC& colB, CollisionInfo& info);

    //================= RECTANGLES =================//

    // Performs a collision check between a rectangle and a capsule given by its top left position, radius and height
    void CheckCollisionRecCapsule(const Rectangle& rect, Point pos, float r, float h, CollisionInfo& info);

    //================= CIRCLE =================//

    // Performs a collision check between a circle given by its center and radius
    // and a quadrilateral given by its 4 points
    void CheckCollisionCircleToQuadrilateral(Point c, float r, Point q1, Point q2, Point q3, Point q4, CollisionInfo& i);

    // Performs a collision check between a circle given by its center and radius
    // and a capsule given by its top left position, radius and height
    void CheckCollisionCircleToCapsule(Point c, float r, Point pos, float cR, float cH, CollisionInfo& info);

    //================= CAPSULE =================//

    // Performs a collision check between two capsules given by their top left position and radius and height
    void CheckCollisionCapsuleCapsule(Point p1, float r1, float h1, Point p2, float r2, float h2, CollisionInfo& info);

    // Performs a collision check between a capsules given it's their top left position and radius and height
    // and a quadrilateral given by its 4 points
    void CheckCollisionCapsuleQuadrilateral(Point p1, float r1, float h1, Point q1, Point q2, Point q3, Point q4,
                                            CollisionInfo& info);

    //================= QUADRILATERAL & TRIANGLES =================//

    // Performs a collision check between two quadrilaterals (p and q) given by their 4 points respectively
    void CheckCollisionQuadrilaterals(Point p1, Point p2, Point p3, Point p4, Point q1, Point q2, Point q3, Point q4,
                                      CollisionInfo& info);

    //================= UTIL =================//

    // Rotates the given points by the angle (starting with 0 degrees at 12'o clock clockwise) around the given anchor
    void RotatePoints(float angle, Point anchor, Point& p1, Point& p2, Point& p3, Point& p4);

} // namespace magique


#endif //MAGIQUE_COLLISION_DETECTION_H