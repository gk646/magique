#ifndef LOGICTICKDATA_H
#define LOGICTICKDATA_H

#include <entt/entity/entity.hpp>

#include <magique/util/Defines.h>
#include <magique/persistence/container/GameConfig.h>

#include "internal/datastructures/VectorType.h"
#include "internal/datastructures/HashTypes.h"
#include "internal/datastructures/MultiResolutionGrid.h"

namespace magique
{
    struct PairInfo final // Saves entity id and type
    {
        entt::entity e1;
        EntityID id1;
        entt::entity e2;
        EntityID id2;
    };

    template <typename T>
    struct AlignedVec final
    {
        // To prevent false sharing
        alignas(64) vector<T> vec;
    };

    using CollPairCollector = AlignedVec<PairInfo>[MAGIQUE_WORKER_THREADS + 1];
    using EntityCollector = AlignedVec<entt::entity>[MAGIQUE_WORKER_THREADS + 1];
    using EntityCache = HashMap<entt::entity, uint16_t>;
    using HashGrid = SingleResolutionHashGrid<entt::entity, 32>;

    struct EngineData final
    {
        CollPairCollector collisionPairs{};                  // Collision pair collectors
        EntityCollector collectors{};                        // Collects entities - 2 for the 2 worker threads
        HashGrid hashGrid{200};                              // Global hashGrid for all entities
        EntityCache entityUpdateCache{1000};                 // Caches entites not in update range anymore
        HashSet<uint64_t> pairSet;                           // Filters unique collision pairs
        std::array<MapID, MAGIQUE_MAX_PLAYERS> loadedMaps{}; // Currently loaded zones
        std::vector<entt::entity> entityUpdateVec;           // vector containing the entites to update for this tick
        std::vector<entt::entity> drawVec;                   // vector containing all entites to be drawn this tick
        std::function<void(GameState, GameState)> stateCallback{}; // Callback funtion for gamstate changes
        GameConfig gameConfig{};                                   // Global game config instance
        vector<entt::entity> collisionVec;      // vector containing the entites to check for collision
        Camera2D camera{};                      // current camera
        entt::entity cameraEntity = entt::null; // entity id of the camera
        GameState gameState;                    // global gamestate
        MapID cameraMap;                        // Map the camera is in

        EngineData()
        {
            hashGrid.reserve(150, 1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            collisionVec.reserve(500);
            pairSet.reserve(1000);
        }

        void clear()
        {
            // changedSet.clear();
            entityUpdateCache.clear();
            entityUpdateVec.clear();
            drawVec.clear();
            hashGrid.clear();
        }
    };

    namespace global
    {
        inline EngineData ENGINE_DATA;
    }

} // namespace magique

#endif //LOGICTICKDATA_H