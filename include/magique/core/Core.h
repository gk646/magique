#ifndef MAGIQUE_CORE_H
#define MAGIQUE_CORE_H

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
    bool InitMagique();


    //----------------- SETTERS -----------------//

    // Sets the update distance in pixels around actors
    // Entities in range will be collision checked
    // Default: 1000
    void SetEntityUpdateDistance(int pixels);

    // When entities leave the update range they are still updated for the cache duration
    // Default: 300 Ticks-> 5 seconds
    void SetEntityCacheDuration(int ticks);

    // Adds the entity to the update cache manually regardless of position
    void AddToUpdateCache(entt::entity e);


    //----------------- GETTERS -----------------//

    // Returns a list of all entities within update range
    const vector<entt::entity>& GetUpdateEntities();

    // Returns the current draw data for this tick
    // Guaranteed to not change during each tick
    DrawTickData& GetDrawTickData();


} // namespace magique


#endif //MAGIQUE_CORE_H