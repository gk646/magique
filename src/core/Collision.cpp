#include <raylib/raylib.h>

#include <magique/core/Collision.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/StaticCollisionData.h"

namespace magique
{
    void SetStaticWorldBounds(const Rectangle& rectangle) { global::ENGINE_CONFIG.worldBounds = rectangle; }

    void LoadCollisionObjects(const MapID map, const std::vector<TileObject>& collisionObjects)
    {
        auto& data = global::STATIC_COLL_DATA;
        const auto it = data.mapObjects.find(map);
        if (it == data.mapObjects.end()) [[unlikely]]
        {
            for (const auto& obj : collisionObjects)
            {
                auto& grid = data.hashGrids[map];
                grid.insert()
            }
            it->second = &collisionObjects;
        }
    }

    void SetGlobalTileSet(TileSet& tileSet, int collisionClass) {}

    void LoadTileMap(MapID map, const TileMap& tileMap) {}

    void AddStaticColliderRect(int group, float x, float y, float width, float height) {}

    void AddStaticColliderCircle(int group, float x, float y, float radius) {}

    void RemoveColliderGroup(int group) {}


} // namespace magique