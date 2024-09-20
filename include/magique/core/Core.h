#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>
#include <entt/entity/fwd.hpp>
#include <magique/core/Types.h>
#include <magique/internal/Macros.h>
INCLUDE_FUNCTIONAL()

//-----------------------------------------------
// Core Module
//-----------------------------------------------
// ................................................................................
// This module allows access and control over the core behavior of the engine
// Also provides util methods to configure the engine
// ................................................................................

namespace magique
{
    // Returns the current gamestate
    // Default: if unset returns GameState(INT32_MAX)
    GameState GetGameState();

    // Sets the current gamestate
    void SetGameState(GameState gameState);

    // Sets the callback function that is called each time the gamestate is changed
    // Note: Called after the new gamestate has been assigned internally
    void SetGameStateChangeCallback(const std::function<void(GameState oldState, GameState newState)>& func);

    //----------------- CORE BEHAVIOR -----------------//

    // Sets the size of the update square centered on the actors in each direction
    // Entities in range will be collision checked and added to the update vector
    // Default: 1000
    void SetUpdateDistance(int distance);

    // Sets the current lighting mode - Entities need the Occluder and Emitter components!
    // HardShadows (fast, looks nice) , RayTracking (slow!,looks really nice) , None (very fast!, looks bland)
    // Default: None (others don't work yet)
    void SetLightingMode(LightingMode model);

    // Sets the new camera holder - removes the component from the current and adds it to the new holder
    void SetCameraEntity(entt::entity entity);

    // When entities leave the update range they are still updated for the cache duration
    // Default: 300 Ticks
    void SetEntityCacheDuration(int ticks);

    // Adds the entity to the update cache manually regardless of position
    void AddToEntityCache(entt::entity e);

    // Manually clears the entity cache in this tick
    void ClearEntityCache();

    // Returns the game configuration
    GameConfig& GetGameConfig();

    //----------------- DATA ACCESS -----------------//

    // Returns a list of all entities within update range of any actor - works across multiple maps!
    const std::vector<entt::entity>& GetUpdateEntities();

    // Returns a list of all entities that should be drawn - culled with the current camera
    const std::vector<entt::entity>& GetDrawEntities();

    // Returns the currently loaded maps - fills up unused slots with UINT8_MAX
    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones();

    // Note: Both nearby entity methods cache call parameters
    // -> No overhead when called with the same origin and radius (only in the same tick)

    // Returns a vector containing all entities within the specified distance of the given entity
    // Note: The returned vector is only valid until this method is called again (single instance)
    const std::vector<entt::entity>& GetNearbyEntities(Point origin, float radius);

    // Returns true if the nearby entities contain the given target entity
    // Note: This is a hash lookup O(1) (after querying the hashgrid)
    bool NearbyEntitiesContain(Point origin, float radius, entt::entity target);

    //----------------- CAMERA -----------------//

    // Adds additional padding to the sides of the normal camera rectangle (enlarged rectangle) - scales with zoom
    // Useful for when you have large effects or entities
    // Default: 250
    void SetCameraViewPadding(int distance);

    // Manually sets the camera offset from the top left of the screen
    // Automatically set to half the screen dimensions and centered on the collision shape of the camera entity (if any)
    // If any offset other than (0,0) is set there are no automatic adjustments
    void SetManualCameraOffset(float x, float y);

    // Sets a smoothing value from 0.0 - 1.0 with 1.0 being the slowest
    // Controls how fast the camera position catches up to the position of the camera holder
    // Default: 0.9
    void SetCameraSmoothing(float smoothing);

    // Returns the global camera
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
    entt::entity GetCameraEntity();

    //----------------- UTILS -----------------//

    // Sets the engine font for performance-overlay and console
    void SetEngineFont(const Font& font);

    // Initializes the engine - does not need to be called when using the game template (Game class)
    // Needs to be called after InitWindow();
    bool InitMagique();

} // namespace magique
#endif //MAGIQUE_CORE_H