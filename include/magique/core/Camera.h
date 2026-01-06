// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CAMERA_H
#define MAGIQUE_CAMERA_H

#include <magique/core/Types.h>

//===============================================
// Camera Module
//===============================================
// ................................................................................
// This module contains methods to access and manipulate the internal engine camera.
// Note: You can use a custom camera, but then you have to update the internal camera each tick (used for culling, and more)
// ................................................................................

namespace magique
{
    // Adds additional padding to the sides of the normal camera rectangle to be used to cull entities inside visible space
    // Entities are checked with their topleft to be inside the enlarged rectangle (so necessary when you have large entities)
    // Default: 250
    void CameraSetCullPadding(int distance);

    // Manually sets the camera view offset from the top left of the screen - automatically set to be half the screen dimensions
    // Note: If any offset other than (0,0) is set there are no automatic adjustments - useful if for different render target
    // Default: (0,0)
    void CameraSetViewOffset(float x, float y);

    // Sets the cameras position offset - automatically centered within the camera entities collision shape (if it exists)
    // Note: If any offset other than (0,0) is set there are no automatic adjustments
    // Default: (0,0)
    void CameraSetTargetOffset(float x, float y);

    // Sets a smoothing value from 0.0 - 1.0 with 1.0 being the fastest
    // Controls how fast the camera position catches up to the position of the camera holder
    // Default: 0.4
    void CameraSetSmoothing(float smoothing);

    // Removes the CameraC component from the current holder and adds it to the specified entity (making it the new holder)
    void CameraSetEntity(entt::entity target);

    // Returns the current camera holder
    // Failure: returns UINT32_MAX if there is no camera currently
    entt::entity CameraGetEntity();

    // Returns the engine internal camera - used for all getter methods, culling visible entities and drawing debug visuals
    Camera2D& CameraGet();

    // Returns the map of the camera - you should probably draw this map
    // Failure: returns UINT8_MAX if there is no camera
    MapID CameraGetMap();

    // Returns the current position (target) of the camera
    Vector2 CameraGetPosition();

    // Returns the bounds of the camera rect including the view padding and zoom scaling
    Rectangle CameraGetBounds();

    // Returns the bounds of the camera without padding
    Rectangle CameraGetNativeBounds();

    //================= UTIL =================//

    // Returns true if the given point is inside any actors view bounds (using the camera bounds)
    bool IsInsideAnyActorViewBounds(Point pos);

    //================= SHAKE =================//

    // Adds a camera shake impulse which cause the camera to oscillate around the origin
    //      - direction: direction vector in which to apply the impulse (see gamedev/Particles.h for more info)
    //      - maxDistance: maximum distance in pixels from the origin the camera will go in either direction
    //      - velocity: how far the shake changes each tick in pixels/s
    //      - decay: how much the maxDistance is reduced in pixels/s
    // Note: the shake stops when maxDistance is 0 - the direction of multiple impulses adds up, the rest overwrites
    void CameraAddShakeImpulse(Point direction, float maxDistance = 25, float velocity = 5, float decay = 5);

} // namespace magique


#endif //MAGIQUE_CAMERA_H
