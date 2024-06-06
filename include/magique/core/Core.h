#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>

#include <magique/fwd.hpp>
#include <magique/util/Defines.h>

#include <entt/entity/fwd.hpp>
#include <raylib/raylib.h>

//-----------------------------------------------
// CORE
//-----------------------------------------------
// .....................................................................
// This is the core API interface with access to many underlying details
// .....................................................................

namespace magique
{
    // Initializes the engine - does not need to be called when using the game template
    // Call after InitWindow();
    bool InitMagique();

    //----------------- SETTERS -----------------//

    // Sets the new camera holder - removes the component from the current and adds it to the new holder
    void SetCameraEntity(entt::entity entity);

    // Sets the update radius distance around actors
    // Entities in range will be collision checked and added to the udpate vector
    // Default: 1000
    void SetUpdateDistance(int distance);

    // Adds aditional padding to the sides of the normal camera rectangle - automatically scales with zoome
    // Useful for when you have large effects or large entities
    // Default: 250
    void SetCameraViewPadding(int distance);

    // When entities leave the update range they are still updated for the cache duration
    // Default: 300 Ticks
    void SetEntityCacheDuration(int ticks);

    // Adds the entity to the update cache manually regardless of position
    void AddToUpdateCache(entt::entity e);


    //----------------- Logic Tick Data -----------------// // Updated at the beginning of each update tick
    // IMPORTANT: If you access this data on the draw thread (main thread) AND outside of drawGame() needs to be synced

    // Returns a list of all entities within update range of any actor - works across multiple maps!
    const std::vector<entt::entity>& GetUpdateEntities();

    // Returns the currently loaded zones - fills up unused slots with UINT8_MAX
    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones();

    // Returns a list of all entities that should be drawn - culled with the current camera
    const std::vector<entt::entity>& GetDrawEntities();

    // Returns the map of the camera - you should probably draw this map
    MapID GetCameraMap();

    // Returns the current position (target) of the camera
    // Specifically the entities position offset by its collision bounds (if any)
    Vector2 GetCameraPosition();

    // Returns the bounds of the camera rect including the view padding
    Rectangle GetCameraBounds();

    //----------------- Draw Tick Data -----------------// // Updated at the beginning of each draw tick

    // Returns the current draw data for this tick
    // Guaranteed to not change during each tick
    DrawTickData& GetDrawTickData();

    //----------------- THREADING -----------------//

    // You generally dont have to call this - only call this if you know what it does
    // IMPORTANT: Must not be called from Game::drawGame()! - its already synced -> Deadlock
    void SyncThreads();

    // You generally dont have to call this - only call this if you know what it does
    void UnSyncThreads();

} // namespace magique


#endif //MAGIQUE_CORE_H