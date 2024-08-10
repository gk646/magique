#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>
#include <entt/entity/fwd.hpp>
#include <magique/fwd.hpp>
#include <magique/util/Defines.h>

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

    //----------------- CORE FEATURES -----------------//

    // Returns the current gamestate
    // Default: if unset returns GameState(INT32_MAX)
    GameState GetGameState();

    // Sets the current gamestate
    void SetGameState(GameState gameState);

    // Sets the callback function that is called each time the gamestate is changed
    // Note: Called after the new gamestate has been assigned internally
    void SetGameStateChangeCallback(const std::function<void(GameState oldState, GameState newState)>& func);

    // Sets the update radius distance around actors
    // Entities in range will be collision checked and added to the update vector
    // Default: 1000
    void SetUpdateDistance(int distance);

    // Adds aditional padding to the sides of the normal camera rectangle - automatically scales with zoom
    // Useful for when you have large effects or entities
    // Default: 250
    void SetCameraViewPadding(int distance);

    // Manually sets the camera offset from the top left of the screen
    // Automatically set to half the screen dimensions and centered on the collision shape of the camera entity (if any)
    // If any offset other than (0,0) is set there are no automatic adjustments
    void SetManualCameraOffset(float x, float y);

    // Sets static collision bounds - this is only useful for simpler scenes
    // Everything outside the bounds is considered solid (static) automatically
    // Pass a width or height of 0 to disable
    // Default: Disabled
    void SetStaticWorldBounds(const Rectangle& rectangle);

    // Sets the current lighting mode - Entities need the Occluder and Emitter components!
    // HardShadows (default,fast, looks nice) , RayTracking (slow!,looks really nice) , None (very fast!, looks bland)
    void SetLightingMode(LightingMode model);

    // Adds a static collider to the world
    void AddStaticCollider(Shape shape, float x, float y, float width, float height);

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

    //----------------- Logic Tick Data -----------------// // Updated at the beginning of each update tick
    // IMPORTANT: If you access this data on the draw thread (main thread) AND outside of drawGame() needs to be synced

    // Returns a list of all entities within update range of any actor - works across multiple maps!
    const std::vector<entt::entity>& GetUpdateEntities();

    // Returns a list of all entities that should be drawn - culled with the current camera
    const std::vector<entt::entity>& GetDrawEntities();

    //----------------- ACCESS -----------------//

    // Returns the currently loaded zones - fills up unused slots with UINT8_MAX
    std::array<MapID, MAGIQUE_MAX_PLAYERS> GetLoadedZones();

    // Returns the global camera
    Camera2D& GetCamera();

    // Returns the map of the camera - you should probably draw this map
    MapID GetCameraMap();

    // Returns the current position (target) of the camera
    // Specifically the entities position offset by its collision bounds (if any)
    Vector2 GetCameraPosition();

    // Returns the bounds of the camera rect including the view padding and zoom scaling
    Rectangle GetCameraBounds();

    // Returns the bounds of camera without padding
    Rectangle GetCameraNativeBounds();

    // Returns the current camera holder
    entt::entity GetCameraEntity();

    // Returns the game configuration
    // IMPORTANT: Only valid after the Game() constructor returned!
    GameConfig& GetGameConfig();

    //----------------- SETTINGS -----------------//

    // If enabled display performance metrics on the top left
    // Default: false
    void SetShowPerformanceOverlay(bool val);

    // Sets the engine font for performance-overlay and console
    void SetEngineFont(const Font& font);

    //----------------- UTILS -----------------//

    // If true shows red hitboxes for collidable entities
    void SetShowHitboxes(bool val);

    // Sets the amount of logic ticks until the game closes automatically
    // This ensures same length benchmarks
    // Default: 0 - off
    void SetBenchmarkTicks(int ticks);

    // Clears the collected benchmark times
    void ResetBenchmarkTimes();

} // namespace magique
#endif //MAGIQUE_CORE_H