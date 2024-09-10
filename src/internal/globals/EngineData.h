#ifndef MAGIQUE_ENGINE_DATA_H
#define MAGIQUE_ENGINE_DATA_H

#include <raylib/raylib.h>

#include <magique/persistence/container/GameConfig.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"

//-----------------------------------------------
// Engine Data
//-----------------------------------------------
// .....................................................................
// Saves shared state for both dynamic and static collision, camera and more (loaded maps, camera entity, camera map...)
// .....................................................................

namespace magique
{
    static constexpr int WORK_PARTS = MAGIQUE_WORKER_THREADS + 1; // Amount of parts to split work into

    using EntityCache = HashMap<entt::entity, uint16_t>;
    using StateCallback = std::function<void(GameState, GameState)>;

    struct NearbyQueryData final
    {
        HashSet<entt::entity> cache;
        float lastRadius = 0;
        Point lastOrigin{};
    };

    struct EngineData final
    {
        StateCallback stateCallback{};                       // Callback function for gamestate changes
        EntityCache entityUpdateCache;                       // Caches entities not in update range anymore
        std::vector<entt::entity> entityUpdateVec;           // Vector containing the entities to update for this tick
        std::vector<entt::entity> drawVec;                   // Vector containing all entities to be drawn this tick
        vector<entt::entity> collisionVec;                   // Vector containing the entities to check for collision
        GameConfig gameConfig{};                             // Global game config instance
        std::array<MapID, MAGIQUE_MAX_PLAYERS> loadedMaps{}; // Currently loaded zones
        Camera2D camera{};                                   // Current camera
        entt::entity cameraEntity{};                         // Entity id of the camera
        GameState gameState{};                               // Global gamestate
        MapID cameraMap = MapID(UINT8_MAX);                  // Map the camera is in
        NearbyQueryData nearbyQueryData;

        EngineData()
        {
            entityUpdateCache.reserve(1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            collisionVec.reserve(500);
            nearbyQueryData.cache.reserve(100);
        }
    };

    namespace global
    {
        inline EngineData ENGINE_DATA{};
    }

} // namespace magique

#endif //MAGIQUE_ENGINE_DATA_H