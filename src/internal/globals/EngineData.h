// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ENGINE_DATA_H
#define MAGIQUE_ENGINE_DATA_H

#include <raylib/raylib.h>

#include <magique/core/GameConfig.h>
#include <entt/entity/entity.hpp>

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

    struct CameraShakeData final
    {
        Point direction;
        Point veloc;
        float maxDist;
        float decay;
        bool up;
    };

    struct EngineData final
    {
        StateCallback stateCallback{};               // Callback function for gamestate changes
        EntityCache entityUpdateCache;               // Caches all entities for a set amount of ticks
        HashSet<entt::entity> entityNScriptedSet;    // Contains all entities NOT scripted
        std::vector<entt::entity> entityUpdateVec;   // Vector containing the entities to update for this tick
        std::vector<entt::entity> drawVec;           // Vector containing all entities to be drawn this tick
        std::vector<MapID> loadedMaps{};             // Currently loaded zones
        vector<entt::entity> collisionVec;           // Vector containing the entities to check for collision
        CameraShakeData cameraShake{};               // Data about the current camera shake
        GameConfig gameConfig{};                     // Global game config instance
        Camera2D camera{};                           // Current camera
        Game* gameInstance;                          // The game instance created by the user
        entt::entity cameraEntity{UINT32_MAX};       // Entity id of the camera
        GameState gameState{INT32_MAX};              // Global gamestate
        MapID cameraMap = MapID(UINT8_MAX);          // Map the camera is in
        NearbyQueryData nearbyQueryData;             // Caches the parameters of the last query to skip similar calls
        entt::entity playerEntity = entt::null;      // Manually set player entity
        float engineTime = 0.0F;                     // Time since engine start
        uint32_t engineTicks;                        // Ticks since engine start
        DestroyEntityCallback destroyEntityCallback; // Function to be called when any entity is destroyed

        void init()
        {
            camera.zoom = 1.0F;
            destroyEntityCallback = nullptr;
            entityUpdateCache.reserve(1000);
            drawVec.reserve(1000);
            entityUpdateVec.reserve(1000);
            collisionVec.reserve(500);
            nearbyQueryData.cache.reserve(100);
        }

        [[nodiscard]] bool isEntityScripted(const entt::entity e) const { return !entityNScriptedSet.contains(e); }

        void updateCameraShake()
        {
            return;
            auto& shake = cameraShake;
            auto& target = camera.target;

            if (target.x >= shake.direction.x * shake.maxDist || target.y >= shake.direction.y * shake.maxDist)
            {
                shake.up = false;
            }
            else if (target.x <= shake.direction.x * -shake.maxDist || target.y <= shake.direction.y * -shake.maxDist)
            {
                shake.up = true;
            }

            if (shake.up)
            {
                target.x = std::min(shake.maxDist, target.x + shake.veloc.x);
                target.y = std::min(shake.maxDist, target.y + shake.veloc.y);
            }
            else
            {
                target.x = std::max(-shake.maxDist, target.x - shake.veloc.x);
                target.y = std::max(-shake.maxDist, target.y - shake.veloc.y);
            }
            shake.maxDist = std::max(shake.maxDist - (shake.decay / MAGIQUE_LOGIC_TICKS), 0.0F);
            if (shake.maxDist <= 0.1F && std::abs(camera.target.x) < 0.5F && std::abs(camera.target.y) < 0.5F)
            {
                shake.veloc = {0, 0};
                //camera.target.x = 0;
                //camera.target.y = 0;
            }
        }
    };

    namespace global
    {
        inline EngineData ENGINE_DATA{};
    }

} // namespace magique

#endif //MAGIQUE_ENGINE_DATA_H