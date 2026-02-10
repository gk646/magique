// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>
#include <functional>
#include <entt/entity/fwd.hpp>
#include <magique/core/Types.h>

//===============================================
// Core Module
//===============================================
// ................................................................................
// This module allows access and control over the core behavior of the engine
// Also provides util methods to configure the engine
// ................................................................................

namespace magique
{
    // Returns the current gamestate
    // Default: if unset returns GameState(UINT8_MAX)
    GameState GetGameState();

    // Sets the current gamestate
    void SetGameState(GameState gameState);

    using GameStateCallback = std::function<void(GameState oldState, GameState newState)>;

    // Sets the callback function that is called each time the gamestate is changed
    // Note: Called after the new gamestate has been assigned internally
    void SetGameStateChangeCallback(const GameStateCallback& func);

    //================= CORE BEHAVIOR =================//

    // Sets the size of the update square centered on the actors
    // Entities in range will be collision checked and added to the update vector
    // Note: Entities not in range can still be updated via the EntityScript::onTick function
    // Default: 2500
    void SetEntityUpdateRange(int distance);

    // Returns the current update range
    int GetEntityUpdateRange();

    // When entities leave the update range they are still updated for the cache duration
    // Default: 300 Ticks - Max: 65535
    void SetEntityCacheDuration(int ticks);

    // Adds the entity to the update cache manually regardless of position
    void AddToEntityCache(entt::entity e);

    // Manually clears the entity cache in this tick
    void ClearEntityCache();

    // Returns true if this entity is in the entity cache (update this tick)
    bool IsInEntityCache(entt::entity e);

    // Allows to turn off the built-in lighting system - useful if you want to do it on your own
    // Note: if disabled you will have to iterate the lighting components and render it with your own shader
    // Default: true
    void SetEnableLightingSystem(bool val);

    // Allows to turn off built-in collision handling - useful if you want to do it on your own
    // Note: if disabled onStaticCollision() and onDynamicCollision() event method will NOT be called anymore
    // Default: true
    void SetEnableCollisionSystem(bool value);

    //================= DATA ACCESS =================//

    // Returns a list of all entities within update range of any actor - works across multiple maps!
    const std::vector<entt::entity>& GetUpdateEntities();

    // Returns a list of all entities that should be drawn - culled with the current camera
    const std::vector<entt::entity>& GetDrawEntities();

    // Returns the currently loaded maps - a map is loaded if it contains at least 1 actor
    const std::vector<MapID>& GetActiveMaps();

    //================= QUERY LOADED ENTITIES =================//

    using FilterFunc = std::function<bool(entt::entity)>;

    // Returns a vector containing all (collision) entities within the given shape
    // Note: The returned vector is only valid until this method is called again (single instance)
    // Note: Only searches entity within update range of any ActorC!
    // If filter func returns false entity is removed from the vector
    const std::vector<entt::entity>& QueryLoadedEntitiesCircle(MapID map, Point origin, float size,
                                                               const FilterFunc& filter = {});
    const std::vector<entt::entity>& QueryLoadedEntitiesRect(MapID map, Point origin, Point size,
                                                             const FilterFunc& filter = {});

    //================= QUERY ENTITIES =================//

    // Similar to the loaded variant but searches all entities instead of only those within update range
    // Much slower!
    const std::vector<entt::entity>& QueryEntitiesCircle(MapID map, Point origin, float size,
                                                         const FilterFunc& filter = {});
    const std::vector<entt::entity>& QueryEntitiesRect(MapID map, Point origin, Point size,
                                                       const FilterFunc& filter = {});

    //================= UTILS =================//

    // Allows to set and retrieve a player entity (the controlled entity)
    // This is useful, as the camera could be different from the controlled character
    // With this you have a single point of truth which is the currently controlled character
    // e.g. What abilities to draw (hotbar)? Show the health of what entity? Which entity receives inputs?
    // Failure: Returns entt::null if not set - assignment is fully MANUAL!
    void SetPlayerEntity(entt::entity entity);
    entt::entity GetPlayerEntity();

    // Sets the engine font for performance-overlay and console
    void SetEngineFont(const Font& font);
    const Font& GetEngineFont();

    // Returns the seconds since startup - updated at the start of each tick
    float GetEngineTime();

    // Returns the logic ticks since startup - updated at the start of each logic tick (MAGIQUE_LOGIC_TICKS)
    // Note: Can also be used to track if a tick passed
    uint32_t GetEngineTick();

    namespace internal
    {
        // Initializes the engine - does not need to be called when using the game template (Game class)
        // Needs to be called after InitWindow();
        bool InitMagique();
    } // namespace internal

} // namespace magique


#endif // MAGIQUE_CORE_H
