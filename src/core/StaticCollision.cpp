#include <raylib/raylib.h>

#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSet.h>
#include <magique/core/StaticCollision.h>
#include <magique/util/Logging.h>

#include "internal/globals/StaticCollisionData.h"

namespace magique
{
    void SetStaticWorldBounds(const Rectangle& rectangle) { global::STATIC_COLL_DATA.worldBounds = rectangle; }

    void LoadMapColliders(const MapID map, const std::vector<TileObject>& collisionObjects, const float scale)
    {
        if (collisionObjects.empty())
            return;
        auto& data = global::STATIC_COLL_DATA;
        if (!data.objectReferences.objectVectors.contains(map))
        {
            data.objectReferences.objectVectors[map] = {}; // Initalize to nullptr
        }

        auto& ptrArr = data.objectReferences.objectVectors[map];
        for (const auto& ptr : ptrArr) // Check if the vector already exists
        {
            if (ptr == &collisionObjects)
            {
                return; // Skip if already exists
            }
        }

        int count = 0;
        for (auto& ptr : ptrArr) // Find insert position
        {
            if (ptr == nullptr)
            {
                ptr = &collisionObjects;
                break;
            }
            ++count;
        }
        if (count >= MAGIQUE_MAX_OBJECT_LAYERS)
        {
            LOG_WARNING("There are already the maximum amount of object vectors registered for this map");
            return;
        }

        // Insert
        auto& referenceVec = data.objectReferences.tileObjectMap[map];
        for (const auto& obj : collisionObjects)
        {
            if (!obj.visible) // Only visible objects
                continue;
            const float scaledX = obj.x * scale;
            const float scaledY = obj.y * scale;
            const float scaledWidth = obj.width * scale;
            const float scaledHeight = obj.height * scale;

            const auto num = data.objectStorage.insert(scaledX, scaledY, scaledWidth, scaledHeight);
            const auto staticID = StaticIDHelper::CreateID(num, obj.getClass());
            data.objectGrid.insert(staticID, scaledX, scaledY, scaledWidth, scaledHeight);
            referenceVec.push_back(num); // So we can uniquely delete later
        }
    }

    void LoadGlobalTileSet(const TileSet& tileSet, const std::vector<int>& markedClasses, const float scale)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (data.tileSet != nullptr)
        {
            return;
        }
        data.tileSet = &tileSet;
        data.tileSetScale = scale;
        HashSet<int> markedClassMap; // Local hashmap to guarantee speed - TileMaps can be big up to 65k tiles
        for (const auto num : markedClasses)
        {
            markedClassMap.insert(num);
        }
        for (const auto& tileInfo : tileSet.getTileInfo())
        {
            if (markedClassMap.contains(tileInfo.getClass()))
            {
                data.markedTilesMap[tileInfo.tileID] = tileInfo;
            }
        }
    }

    void LoadTileMapCollisions(const MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (data.tileSet == nullptr)
        {
            LOG_WARNING("Cannot load tile collision data without tile set. Use LoadGlobalTileSet()");
            return;
        }
        auto& hashMap = data.objectReferences.markedTilesDataMap;
        const auto it = hashMap.find(map);
        if (it == hashMap.end())
        {
            const auto tileSize = data.tileSetScale * static_cast<float>(data.tileSet->getTileSize());
            const auto width = tileMap.getWidth();
            const auto height = tileMap.getHeight();
            const auto& markedMap = data.markedTilesMap;
            auto& tileGrid = data.tileGrid;
            auto& storage = data.objectStorage;
            auto& tileVec = hashMap[map];
            for (const auto layer : layers)
            {
                const auto start = tileMap.getLayerData(layer);
                for (int i = 0; i < height; ++i)
                {
                    const auto yOff = i * width;
                    for (int j = 0; j < width; ++j)
                    {
                        const auto tileNum = start[yOff + j] - 1; // tile data is 1 more to mark empty as 0
                        if (tileNum == UINT16_MAX)                // uint overflows to maximum value (0-1 = MAX)
                            continue;
                        const auto infoIt = markedMap.find(tileNum);
                        if (infoIt != markedMap.end())
                        {
                            auto [x, y, width, height] = infoIt->second.getCollisionRect(data.tileSetScale);
                            x += static_cast<float>(j) * tileSize;
                            y += static_cast<float>(i) * tileSize;
                            if (width == 0) // rect is 0 if not assigned - so adding to x and y is always valid
                            {
                                width = tileSize;
                                height = tileSize;
                            }
                            const auto objectNum = storage.insert(x, y, width, height);
                            tileVec.push_back(objectNum);
                            const auto tileClass = infoIt->second.getClass();
                            tileGrid.insert(StaticIDHelper::CreateID(objectNum, tileClass), x, y, width, height);
                        }
                    }
                }
            }
        }
    }

    void AddStaticColliderRect(const int group, float x, float y, float width, float height)
    {
        auto& data = global::STATIC_COLL_DATA;
        // data.manualColliderMap[group].push_back({x, y, width, height});
    }

    void RemoveColliderGroup(int group) {}

} // namespace magique