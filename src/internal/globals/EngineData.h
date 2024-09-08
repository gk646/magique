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

    struct PhysicsInfo final
    {
        PositionC* pos = nullptr;
        Point normalVector{0, 0}; // The direction  vector in which the object needs to be mover to resolve the collision
    };

    using EntityCache = HashMap<entt::entity, uint16_t>;
    using StateCallback = std::function<void(GameState, GameState)>;
    using CollisionInfoMap = HashMap<entt::entity, PhysicsInfo>;

    struct EngineData final
    {
        StateCallback stateCallback{};                       // Callback funtion for gamstate changes
        EntityCache entityUpdateCache;                       // Caches entites not in update range anymore
        std::vector<entt::entity> entityUpdateVec;           // Vector containing the entites to update for this tick
        std::vector<entt::entity> drawVec;                   // Vector containing all entites to be drawn this tick
        vector<entt::entity> collisionVec;                   // Vector containing the entites to check for collision
        GameConfig gameConfig{};                             // Global game config instance
        std::array<MapID, MAGIQUE_MAX_PLAYERS> loadedMaps{}; // Currently loaded zones
        Camera2D camera{};                                   // Current camera
        entt::entity cameraEntity{};                         // Entity id of the camera
        GameState gameState{};                               // Global gamestate
        MapID cameraMap = MapID(UINT8_MAX);                  // Map the camera is in

        EngineData()
        {
            entityUpdateCache.reserve(1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            collisionVec.reserve(500);
        }
    };

    namespace global
    {
        inline EngineData ENGINE_DATA{};
    }

} // namespace magique

#endif //MAGIQUE_ENGINE_DATA_H