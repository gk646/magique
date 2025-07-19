// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>
#include <entt/entity/fwd.hpp>
#include <magique/core/Types.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// Core Module
//===============================================
// ................................................................................
// This module allows access and control over the core behavior of the engine
// Also provides util methods to configure the engine
// ................................................................................

namespace magique
{

    // Returns the global game instance
    // IMPORTANT: only valid after the game constructor finished (so before game.run(), ... so pretty early)
    Game& GetGame();

    // Returns the current gamestate
    // Default: if unset returns GameState(INT32_MAX)
    GameState GetGameState();

    // Sets the current gamestate
    void SetGameState(GameState gameState);

    // Sets the callback function that is called each time the gamestate is changed
    // Note: Called after the new gamestate has been assigned internally
    void SetGameStateChangeCallback(const std::function<void(GameState oldState, GameState newState)>& func);

    //================= CORE BEHAVIOR =================//

    // Sets the size of the update square centered on the actors
    // Entities in range will be collision checked and added to the update vector
    // Note: Entities not in range can still be updated via the EntityScript::onTick function
    // Default: 1500
    void SetEntityUpdateRange(int distance);

    // Sets the current lighting mode - Entities need the Occluder and Emitter components!
    // HardShadows (fast, looks nice), RayTracking (slow!,looks really nice) , None (very fast!, looks bland)
    // Note: If disabled you can iterate the lighting components and render it with your own shader or co
    // Default: None (others don't work yet)
    void SetLightingMode(LightingMode model);

    // When entities leave the update range they are still updated for the cache duration
    // Default: 300 Ticks - Max: 65535
    void SetEntityCacheDuration(int ticks);

    // Adds the entity to the update cache manually regardless of position
    void AddToEntityCache(entt::entity e);

    // Manually clears the entity cache in this tick
    void ClearEntityCache();

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
    const std::vector<MapID>& GetLoadedMaps();

    //================= QUERY DYNAMIC ENTITIES =================//
    // Note: These methods cache their call parameters
    // -> No overhead when called with the same parameters in the same tick (with no other calls in between)

    // Returns a vector containing all entities within the specified distance of the given entity
    // Note: The returned vector is only valid until this method is called again (single instance)
    const std::vector<entt::entity>& GetNearbyEntities(MapID map, Point origin, float radius);

    // Returns true if the nearby entities contain the given target entity
    // Note: This is a hash lookup O(1) (after querying the entity grid)
    bool NearbyEntitiesContain(MapID map, Point origin, float radius, entt::entity target);

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

    // Returns the default font used by the engine
    const Font& GetEngineFont();

    // Returns the time since startup - updated each tick
    float GetEngineTime();

    namespace internal
    {
        // Initializes the engine - does not need to be called when using the game template (Game class)
        // Needs to be called after InitWindow();
        bool InitMagique();
    } // namespace internal

} // namespace magique


#endif //MAGIQUE_CORE_H