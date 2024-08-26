#include <raylib/raylib.h>

#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSet.h>
#include <magique/internal/Macros.h>
#include <magique/core/StaticCollision.h>

#include "internal/globals/EngineConfig.h"
#include "internal/globals/StaticCollisionData.h"

namespace magique
{
    void SetStaticWorldBounds(const Rectangle& rectangle) { global::ENGINE_CONFIG.worldBounds = rectangle; }

    void LoadMapColliders(const MapID map, const std::vector<TileObject>& collisionObjects, const float scale)
    {
        auto& data = global::STATIC_COLL_DATA;
        const auto it = data.mapObjects.find(map);
        if (it == data.mapObjects.end()) [[unlikely]]
        {
            for (const auto& obj : collisionObjects)
            {
                const float scaledX = obj.x * scale;
                const float scaledY = obj.y * scale;
                const float scaledWidth = obj.width * scale;
                const float scaledHeight = obj.height * scale;
                const auto num = data.objectHolder.insert(scaledX, scaledY, scaledWidth, scaledHeight);
                const auto staticID = StaticIDHelper::CreateID(num, 0, ColliderType::TILEMAP_OBJECT);
                data.objectGrid.insert(staticID, scaledX, scaledY, scaledWidth, scaledHeight);
            }
            data.mapObjects.insert({map, &collisionObjects});
        }
    }

    void SetGlobalTileSet(const TileSet& tileSet, const int collisionClass, const float tileSize)
    {
        auto& data = global::STATIC_COLL_DATA;
        data.tileSize = tileSize;
        if (&tileSet != data.tileSet)
        {
            data.tileSet = &tileSet;
            std::memset(data.tileCollisionVec.data(), 0, data.tileCollisionVec.size());
            for (const auto tile : tileSet.getTileInfo())
            {
                if (tile.clazz == collisionClass)
                {
                    data.tileCollisionVec.resize(tile.tileID + 1, false);
                    data.tileCollisionVec[tile.tileID] = true;
                }
            }
        }
    }

    void LoadTileMap(const MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers)
    {
        auto& data = global::STATIC_COLL_DATA;
        const auto mapIdx = static_cast<int>(map);
        if (data.mapDataVec.size() < mapIdx) [[unlikely]]
        {
            data.mapDataVec.resize(mapIdx + 1);
        }
        auto& mapData = data.mapDataVec[mapIdx];
        mapData.width = static_cast<uint16_t>(tileMap.getWidth());
        mapData.height = static_cast<uint16_t>(tileMap.getHeight());

        int i = 0;
        for (const auto layer : layers)
        {
            mapData.layerData[i] = tileMap.getLayerData(layer);
            ++i;
        }
    }

    bool IsSolidTile(const MapID map, const int tileX, const int tileY)
    {
        const auto& data = global::STATIC_COLL_DATA;
        const auto mapIdx = static_cast<int>(map);
        if (mapIdx > data.mapDataVec.size()) [[unlikely]]
        {
            MAGIQUE_ASSERT(false, "No map with the given MapID has been loaded! Call LoadTileMap() or wrong MapID used");
            return false;
        }
        const auto& mapData = data.mapDataVec[mapIdx];
        MAGIQUE_ASSERT(mapData.width + mapData.height != 0,
                       "No map with the given MapID has been loaded! Call LoadTileMap()");
        MAGIQUE_ASSERT(tileX < mapData.width && tileY < mapData.height, "Out of bounds! Given tile position is invalid");
        MAGIQUE_ASSERT(tileX >= 0 && tileY >= 0, "Tile position cant be negative!");
        const auto flatIdx = tileX + tileY * mapData.width;
        for (const auto* ptr : mapData.layerData)
        {
            if (ptr == nullptr)
                return false;

            if (data.isSolid(ptr[flatIdx])) [[unlikely]]
            {
                return true;
            }
        }
        return false;
    }

    void AddStaticColliderRect(const int group, float x, float y, float width, float height)
    {
        auto& data = global::STATIC_COLL_DATA;
        data.manualColliderMap[group].push_back({x, y, width, height});
    }

    void RemoveColliderGroup(int group) {}


} // namespace magique