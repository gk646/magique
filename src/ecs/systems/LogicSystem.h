#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

#include <ranges>
#include <cxstructs/SmallVector.h>

using namespace cxstructs;

namespace magique
{
    inline Vector3 GetUpdateCircle(const float x, const float y) { return {x, y, CONFIGURATION.entityUpdateDistance}; }

    inline Rectangle GetCameraRect()
    {
        const auto pad = CONFIGURATION.cameraViewPadding;
        auto& camera = LOGIC_TICK_DATA.camera;
        auto& target = camera.target;
        auto& offset = camera.offset;
        return {target.x - offset.x - pad, target.y - offset.y - pad, offset.x * 2 + pad * 2, offset.y * 2 + pad * 2};
    }

    inline void AssignCameraData(entt::registry& registry)
    {
        const auto view = registry.view<const CameraC, const PositionC>();
        auto& tickData = LOGIC_TICK_DATA;
#if MAGIQUE_DEBUG == 1
        int count = 0;
#endif
        const float sWidth = GetScreenWidth();
        const float sHeight = GetScreenHeight();
        for (const auto e : view)
        {
            const auto& pos = view.get<PositionC>(e);
            tickData.cameraMap = pos.map;
            tickData.camera.offset = {sWidth / 2, sHeight / 2};
            tickData.id = e;
            tickData.camera.target = {pos.x, pos.y};
#if MAGIQUE_DEBUG == 1
            count++;
#endif
        }
        // Center the camera
        const auto coll = REGISTRY.try_get<CollisionC>(LOGIC_TICK_DATA.id);
        if (coll) [[likely]]
        {
            tickData.camera.offset.x -= static_cast<float>(coll->width) / 2.0F;
            tickData.camera.offset.y -= static_cast<float>(coll->height) / 2.0F;
        }
#if MAGIQUE_DEBUG == 1
        //M_ASSERT(count < 2, "You have multiple cameras? O.O");
#endif
    }

    // Insert numbers into flattened array
    inline void InsertToActorDist(SmallVector<int8_t, MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS>& actorDist,
                                  int map, int num)
    {

        for (int i = 0; i < 4; i++)
        {
            if (actorDist[MAGIQUE_MAX_PLAYERS * map + i] == -1)
            {
                actorDist[MAGIQUE_MAX_PLAYERS * map + i] = static_cast<int8_t>(num);
                return;
            }
        }
    }

    // This is a bit complicated as we dont know how many total maps there are
    // So we stay flexible with stack arrays and epxand if needed
    inline void BuildCache(entt::registry& registry, std::array<MapID, MAGIQUE_MAX_PLAYERS>& loadedMaps,
                           Vector3 (&actorCircles)[4], SmallVector<bool, MAGIQUE_MAX_EXPECTED_MAPS>& actorMaps,
                           SmallVector<int8_t, MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS>& actorDist)
    {
        std::memset(loadedMaps.data(), UINT8_MAX, MAGIQUE_MAX_PLAYERS);
        actorDist.resize(MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS, -1);
        actorMaps.resize(MAGIQUE_MAX_EXPECTED_MAPS, false);

        int count = 0;
        const auto view = registry.view<const ActorC, const PositionC>();
        for (const auto actor : view)
        {
            M_ASSERT(count < MAGIQUE_MAX_PLAYERS, "More actors than configured!");
            const auto& pos = view.get<const PositionC>(actor);

            actorDist.resize((int)pos.map, -1);    // only resizes if needed - initializes new values to -1
            actorMaps.resize((int)pos.map, false); // only resizes if needed - initializes new values to false

            actorMaps[(int)pos.map] = true;
            loadedMaps[count] = pos.map;
            actorCircles[count] = GetUpdateCircle(pos.x, pos.y);
            InsertToActorDist(actorDist, (int)pos.map, count);
            count++;
        }
    }

    void HandleCollisionEntity(entt::entity e, const CollisionC collision, const PositionC pos, auto& hashGrid)
    {
        if (pos.rotation == 0) [[likely]] // More likely
        {
            hashGrid.insert(e, pos.x, pos.y, collision.width, collision.height);
        }
        else
        {
            // Calculate rotation bounding rect
            // hashGrids[zone].insert(pos.x, pos.y, pos.width, pos.height, 0, 0, 0, e);
        }
    }

    inline void UpdateLogic(entt::registry& registry)
    {
        auto& tickData = LOGIC_TICK_DATA;
        auto& hashGrid = tickData.hashGrid;
        auto& drawVec = tickData.drawVec;
        auto& cache = tickData.entityUpdateCache;
        auto& updateVec = tickData.entityUpdateVec;
        auto& loadedMaps = tickData.loadedMaps;

        tickData.lock(); // Lock early due to camera assign

        AssignCameraData(registry);

        // Cache
        const auto cameraMap = tickData.cameraMap;
        const uint16_t cacheDuration = CONFIGURATION.entityCacheDuration;
        const auto cameraBounds = GetCameraRect();

        // Lookup tables
        SmallVector<int8_t, MAGIQUE_MAX_EXPECTED_MAPS * MAGIQUE_MAX_PLAYERS> actorDistribution{};
        SmallVector<bool, MAGIQUE_MAX_EXPECTED_MAPS> actorMaps{};
        Vector3 actorCircles[MAGIQUE_MAX_PLAYERS];

        BuildCache(registry, loadedMaps, actorCircles, actorMaps, actorDistribution);

        // Critical section
        {
            drawVec.clear();
            hashGrid.clear();
            updateVec.clear();

            const auto view = registry.view<const PositionC>();
            for (const auto e : view)
            {
                const auto pos = view.get<const PositionC>(e);
                const auto map = pos.map;
                if (actorMaps[static_cast<int>(map)]) [[likely]] // entity is in any map where at least 1 actor is
                {
                    // Check if inside the camera bounds already
                    if (map == cameraMap && CheckCollisionPointRec({pos.x, pos.y}, cameraBounds))
                    {
                        drawVec.push_back(e); // Should be drawn
                        cache[e] = cacheDuration;
                        const auto collision = registry.try_get<CollisionC>(e);
                        if (collision != nullptr) [[likely]]
                            HandleCollisionEntity(e, *collision, pos, hashGrid);
                    }
                    else
                    {
                        for (int i = 0; i < MAGIQUE_MAX_PLAYERS; ++i)
                        {
                            const int8_t actorNum = actorDistribution[static_cast<int>(map) * MAGIQUE_MAX_PLAYERS];
                            if (actorNum == 1)
                                break;
                            const auto vec3 = actorCircles[actorNum];
                            // Check if insdie any update circle
                            if (CheckCollisionPointCircle({pos.x, pos.y}, {vec3.x, vec3.y}, vec3.z))
                            {
                                cache[e] = cacheDuration;
                                const auto collision = registry.try_get<CollisionC>(e);
                                if (collision != nullptr) [[likely]]
                                    HandleCollisionEntity(e, *collision, pos, hashGrid);
                            }
                        }
                    }
                }
            }
        }

        // Fill the update vec after to avoid adding entities that drop out
        for (auto it = cache.begin(); it != cache.end();)
        {
            if (it->second > 0)
            {
                it->second--;
                updateVec.push_back(it->first);
                ++it;
            }
            else
            {
                it = cache.erase(it);
            }
        }

        tickData.unlock();
    }

} // namespace magique


#endif //LOGICSYSTEM_H