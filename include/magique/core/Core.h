#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

#include <vector>

#include "entt/entity/fwd.hpp"
#include "magique/fwd.hpp"

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

    // Sets the update distance in pixels around actors
    // Entities in range will be collision checked
    // Default: 1000
    void SetEntityUpdateDistance(int pixels);

    // When entities leave the update range they are still updated for the cache duration
    // Default: 300 Ticks -> 5 seconds
    void SetEntityCacheDuration(int ticks);

    // Adds the entity to the update cache manually regardless of position
    void AddToUpdateCache(entt::entity e);


    //----------------- GETTERS -----------------//

    // Returns a list of all entities within update range of any actor - works across multiple maps!
    const std::vector<entt::entity>& GetUpdateEntities();

    // Returns a list of all entities that should be drawn - culled with the current camera
    const std::vector<entt::entity>& GetDrawEntities();

    // Returns the current draw data for this tick
    // Guaranteed to not change during each tick
    DrawTickData& GetDrawTickData();

    //----------------- THREADING -----------------//

    // You generally dont have to call this - only call this if you know what it does
    // IMPORTANT: Must not be called from Game::drawGame()! - its already locked -> Deadlock
    void SyncThreads();

    // You generally dont have to call this - only call this if you know what it does
    void UnSyncThreads();

} // namespace magique


#endif //MAGIQUE_CORE_H