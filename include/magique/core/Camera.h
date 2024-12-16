#ifndef MAGIQUE_CAMERA_H
#define MAGIQUE_CAMERA_H

#include <magique/fwd.hpp>

//===============================================
// Camera Module
//===============================================
// ................................................................................
// This module contains methods to access and manipulate the internal engine camera.
// Note: You can use a custom camera, but then you have to update the internal camera each tick (used for culling, and more)
// ................................................................................

namespace magique
{
    //================= CAMERA =================//

    // Adds additional padding to the sides of the normal camera rectangle to be used to cull entities inside visible space
    // Entities are checked with their topleft to be inside the enlarged rectangle (so necessary when you have large entities)
    // Default: 250
    void SetCameraCullPadding(int distance);

    // Manually sets the camera view offset from the top left of the screen - automatically set to be half the screen dimensions
    // Note: If any offset other than (0,0) is set there are no automatic adjustments
    // Default: (0,0)
    void SetCameraViewOffset(float x, float y);

    // Sets the cameras position offset - automatically centered within the camera entities collision shape (if it exists)
    // Note: If any offset other than (0,0) is set there are no automatic adjustments
    // Default: (0,0)
    void SetCameraPositionOffset(float x, float y);

    // Sets a smoothing value from 0.0 - 1.0 with 1.0 being the slowest
    // Controls how fast the camera position catches up to the position of the camera holder
    // Default: 0.9
    void SetCameraSmoothing(float smoothing);

    // Removes the CameraC component from the current holder and adds it to the specified entity (making it the new holder)
    void SetCameraEntity(entt::entity entity);

    // Returns the engine internal camera - used for all getter methods, culling visible entities and drawing debug visuals
    Camera2D& GetCamera();

    // Returns the map of the camera - you should probably draw this map
    // Failure: returns UINT8_MAX if there is no camera
    MapID GetCameraMap();

    // Returns the current position (target) of the camera
    Vector2 GetCameraPosition();

    // Returns the bounds of the camera rect including the view padding and zoom scaling
    Rectangle GetCameraBounds();

    // Returns the bounds of the camera without padding
    Rectangle GetCameraNativeBounds();

    // Returns the current camera holder
    // Failure: returns UINT32_MAX if there is no camera currently
    entt::entity GetCameraEntity();

} // namespace magique


#endif //MAGIQUE_CAMERA_H