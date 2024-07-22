#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>

#include <magique/fwd.hpp>
#include <magique/util/Defines.h>

#include <entt/entity/fwd.hpp>
#include <raylib/raylib.h>

//-----------------------------------------------
// Core Module
//-----------------------------------------------
// ................................................................................
// This is the core API interface with access to many underlying details
// ................................................................................

namespace magique
{
    // Initializes the engine - does not need to be called when using the game template
    // IF called manually needs to be done after InitWindow();
    bool InitMagique();

    //----------------- SETTERS -----------------//

    // If enabled display performance metrics on the top left
    void SetShowPerformanceOverlay(bool val);

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

    // Sets the engine font for overlays etc.
    void SetFont(const Font& font);

    // Sets the current lighting mode - Entities need the Occluder and Emitter components!
    // HardShadows (default,fast, looks nice) , RayTracking (slow!,looks really nice) , None (very fast!, looks bland)
    void SetLightingModel(LightingModel model);

    // Sets static collision bounds - this is only useful for simpler scenes
    // Everything outside the bounds is considered solid (static) automatically
    // Pass a width or height of 0 to disable
    // Default: Disabled
    void SetWorldBounds(const Rectangle& rectangle);

    //----------------- Logic Tick Data -----------------// // Updated at the beginning of each update tick
    // IMPORTANT: If you access this data on the draw thread (main thread) AND outside of drawGame() needs to be synced

    // Returns a list of all entities within update range of any actor - works across multiple maps!
    const std::vector<entt::entity>& GetUpdateEntities();

    // Returns a list of all entities that should be drawn - culled with the current camera
    const std::vector<entt::entity>& GetDrawEntities();

    //----------------- GETTERS -----------------//

    // Returns the currently loaded zones - fills up unused slots with UINT8_MAX
    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones();

    // Returns the map of the camera - you should probably draw this map
    MapID GetCameraMap();

    // Returns the current position (target) of the camera
    // Specifically the entities position offset by its collision bounds (if any)
    Vector2 GetCameraPosition();

    // Returns the bounds of the camera rect including the view padding and zoom
    Rectangle GetCameraBounds();

    // Returns the bounds of camera without padding
    Rectangle GetCameraNativeBounds();

    // Returns the current camera holder
    entt::entity GetCameraEntity();

    // Returns the game configuration
    // IMPORTANT: Only valid after the Game() constructor returned!
    GameConfig& GetGameConfig();

    //----------------- JOB SYSTEM -----------------//
    // Note:: This is for advanced users! Look at util/Jobs.h

    // Returns a reference to the scheduler
    // Allows to submit concurrent jobs to distribute compatible work across threads
    Scheduler& GetScheduler();

    //----------------- THREADING -----------------//

    // You generally dont have to call this - only call this if you know what it does
    // IMPORTANT: Must not be called from Game::drawGame()! - its already synced -> Deadlock
    void SyncThreads();

    // You generally dont have to call this - only call this if you know what it does
    void UnSyncThreads();

    //----------------- DEBUGGING -----------------//

    // If true shows red hitboxes for collidable entities
    void SetShowHitboxes(bool val);

} // namespace magique
#endif //MAGIQUE_CORE_H