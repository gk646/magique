#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>
#include <entt/entity/fwd.hpp>
#include <magique/fwd.hpp>


//-----------------------------------------------
// Core Module
//-----------------------------------------------
// ................................................................................
// This module allows access and control over the core behavior of the engine
// Also provides util methods regarding hitboxes, benchmarking and performance stats
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

    // Adds aditional padding to the sides of the normal camera rectangle (enlarged rectangle) - scales with zoom
    // Useful for when you have large effects or entities
    // Default: 250
    void SetCameraViewPadding(int distance);

    // Manually sets the camera offset from the top left of the screen
    // Automatically set to half the screen dimensions and centered on the collision shape of the camera entity (if any)
    // If any offset other than (0,0) is set there are no automatic adjustments
    void SetManualCameraOffset(float x, float y);

    // Sets the current lighting mode - Entities need the Occluder and Emitter components!
    // HardShadows (default,fast, looks nice) , RayTracking (slow!,looks really nice) , None (very fast!, looks bland)
    void SetLightingMode(LightingMode model);

    //----------------- ENTITIES -----------------//

    // Sets the new camera holder - removes the component from the current and adds it to the new holder
    void SetCameraEntity(entt::entity entity);

    // When entities leave the update range they are still updated for the cache duration
    // Default: 300 Ticks
    void SetEntityCacheDuration(int ticks);

    // Adds the entity to the update cache manually regardless of position
    void AddToEntityCache(entt::entity e);

    // Manually clears the entity cache in this tick
    void ClearEntityCache();

    //----------------- DATA ACCESS -----------------//

    // Returns a list of all entities within update range of any actor - works across multiple maps!
    const std::vector<entt::entity>& GetUpdateEntities();

    // Returns a list of all entities that should be drawn - culled with the current camera
    const std::vector<entt::entity>& GetDrawEntities();

    // Returns the currently loaded maps - fills up unused slots with UINT8_MAX
    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones();

    // Returns the global camera
    Camera2D& GetCamera();

    // Returns the map of the camera - you should probably draw this map
    // Failure: returns UINT8_MAX if there no camera
    MapID GetCameraMap();

    // Returns the current position (target) of the camera
    Vector2 GetCameraPosition();

    // Returns the bounds of the camera rect including the view padding and zoom scaling
    Rectangle GetCameraBounds();

    // Returns the bounds of the camera without padding
    Rectangle GetCameraNativeBounds();

    // Returns the current camera holder
    entt::entity GetCameraEntity();

    // Returns the game configuration
    GameConfig& GetGameConfig();

    //----------------- UTILS -----------------//

    // If enabled display performance metrics on the top left
    // Default: false
    void SetShowPerformanceOverlay(bool val);

    // Sets the engine font for performance-overlay and console
    void SetEngineFont(const Font& font);

    // Initializes the engine - does not need to be called when using the game template
    // Needs to be called after InitWindow();
    bool InitMagique();

    // If true shows red hitboxes for collidable entities
    // This is the single point of truth - If two hitboxes visually overlap then a collision happened!
    void SetShowHitboxes(bool val);

    // Sets the amount of logic ticks until the game closes automatically
    // This ensures same length benchmarks
    // Default: 0 - off
    void SetBenchmarkTicks(int ticks);

    // Clears the collected benchmark times
    void ResetBenchmarkTimes();

    void DrawHashGridDebug();

} // namespace magique
#endif //MAGIQUE_CORE_H