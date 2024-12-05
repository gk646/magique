// SPDX-License-Identifier: zlib-acknowledgement
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
    static constexpr int COL_WORK_PARTS = MAGIQUE_WORKER_THREADS + 1; // Amount of parts to split collision work into

    using EntityCache = HashMap<entt::entity, uint16_t>;
    using StateCallback = std::function<void(GameState, GameState)>;

    struct NearbyQueryData final
    {
        HashSet<entt::entity> cache;
        Point lastOrigin{};
        float lastRadius = 0;
        MapID lastMap{};

        [[nodiscard]] bool getIsSimilarParameters(const MapID map, const Point& p, const float radius) const
        {
            return lastOrigin == p && lastRadius == radius && lastMap == map;
        }
    };

    struct EngineData final
    {
        StateCallback stateCallback{};             // Callback function for gamestate changes
        EntityCache entityUpdateCache;             // Caches entities not in update range anymore
        HashSet<entt::entity> entityNScriptedSet;  // Contains all entities NOT scripted
        std::vector<entt::entity> entityUpdateVec; // Vector containing the entities to update for this tick
        std::vector<entt::entity> drawVec;         // Vector containing all entities to be drawn this tick
        std::vector<MapID> loadedMaps{};           // Currently loaded zones
        vector<entt::entity> collisionVec;         // Vector containing the entities to check for collision
        GameConfig gameConfig{};                   // Global game config instance
        Camera2D camera{};                         // Current camera
        Game* gameInstance;                        // The game instance created by the user
        entt::entity cameraEntity{UINT32_MAX};     // Entity id of the camera
        GameState gameState{};                     // Global gamestate
        MapID cameraMap = MapID(UINT8_MAX);        // Map the camera is in
        NearbyQueryData nearbyQueryData;           // Caches the parameters of the last query to skip similar calls

        void init()
        {
            entityUpdateCache.reserve(1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            collisionVec.reserve(500);
            nearbyQueryData.cache.reserve(100);
        }

        [[nodiscard]] bool isEntityScripted(const entt::entity e) const { return !entityNScriptedSet.contains(e); }
    };

    namespace global
    {
        inline EngineData ENGINE_DATA{};
    }

} // namespace magique

#endif //MAGIQUE_ENGINE_DATA_H