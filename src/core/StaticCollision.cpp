// SPDX-License-Identifier: zlib-acknowledgement
#include <raylib/raylib.h>

#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSet.h>
#include <magique/core/StaticCollision.h>
#include <magique/util/Logging.h>

#include "internal/globals/StaticCollisionData.h"
#include "internal/globals/PathFindingData.h"


namespace magique
{
    void SetStaticWorldBounds(const Rectangle& rectangle) { global::STATIC_COLL_DATA.worldBounds = rectangle; }

    //----------------- TILE OBJECTS -----------------//

    void AddTileMapObjects(const MapID map, const std::vector<TileObject>& collisionObjects, const float scale)
    {
        if (collisionObjects.empty())
        {
            LOG_WARNING("Passed tile object vector is empty");
            return;
        }

        auto& data = global::STATIC_COLL_DATA;
        auto& mapTileObjectVec = data.colliderReferences.tileObjectMap[map];
        for (const auto& info : mapTileObjectVec) // Check if the vector already exists
        {
            if (info.vectorPointer == &collisionObjects)
            {
                LOG_WARNING("Passed tile object vector was already loaded for this map");
                return; // Skip if already exists
            }
        }

        // Insert new info
        ObjectReferenceHolder::TileObjectInfo info;
        info.vectorPointer = &collisionObjects;

        // Insert
        auto& grid = data.mapObjectGrids[map];
        for (const auto& obj : collisionObjects)
        {
            if (!obj.visible) // Only visible objects
                continue;

            const Rect scaled = obj.bounds.scale(scale);
            const auto num = data.colliderStorage.insert(scaled);
            const auto staticID = StaticIDHelper::CreateID(num, obj.getTileClass());
            grid.insert(staticID, scaled.x, scaled.y, scaled.w, scaled.h);
            info.objectIds.push_back(num); // So we can uniquely delete later
        }

        // Insert the info
        mapTileObjectVec.push_back(std::move(info));
        global::PATH_DATA.updateStaticPathGrid(map);
    }

    void RemoveTileMapObjects(const MapID map, const std::vector<TileObject>& collisionObjects)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (!data.colliderReferences.tileObjectMap.contains(map))
        {
            LOG_WARNING("The given map has no TileObject vectors loaded!");
            return;
        }

        auto& mapVec = data.colliderReferences.tileObjectMap.at(map);
        for (const auto& info : mapVec) // Check if the vector already exists
        {
            if (info.vectorPointer == &collisionObjects)
            {
                // Remove from storage and hashgrid
                auto& hashGrid = data.mapTileGrids[map];
                for (const auto id : info.objectIds)
                {
                    data.colliderStorage.remove(id);
                    // We store the object num + the data in the hash grid
                    // But we don't want to store 8 byte in the objectReferences - and the object num is still unique
                    // So we compare the internal static id and see if the object num part matches
                    hashGrid.removeIfWithHoles(id, [](const uint32_t num, const StaticID id)
                                               { return num == StaticIDHelper::GetObjectNum(id); });
                }
                hashGrid.patchHoles(); // Patch as we cant rebuild it
                // Remove the entry in the map specific info vec
                std::erase_if(mapVec, [&](auto& ele) { return ele.vectorPointer == info.vectorPointer; });
                global::PATH_DATA.updateStaticPathGrid(map);
                return; // Skip if already exists
            }
        }
        LOG_WARNING("Given vector was not loaded in the given map!");
    }

    //----------------- TILESET -----------------//

    void LoadGlobalTileSet(const TileSet& tileSet, const float scale)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (data.tileSet != nullptr)
        {
            LOG_WARNING("Overwriting the previously set global TileSet!");
        }
        data.tileSet = &tileSet;
        data.tileSetScale = scale;
        for (const auto& tileInfo : tileSet.getTilesInfo())
        {
            if (tileInfo.hasCollision)
            {
                data.markedTilesMap[tileInfo.tileID] = tileInfo;
            }
        }
    }

    void AddTileCollisions(const MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (data.tileSet == nullptr)
        {
            LOG_WARNING("Cannot load tile collision data without tile set. Use LoadGlobalTileSet()");
            return;
        }

        if (data.colliderReferences.tilesCollisionMap.contains(map))
        {
            LOG_WARNING("Tile data for this map was already loaded. Remove it before you can load it again");
            return;
        }

        auto& grid = data.mapTileGrids[map];
        const auto tileSize = data.tileSetScale * static_cast<float>(data.tileSet->getTileSize());
        const auto mapWidth = tileMap.getWidth();
        const auto mapHeight = tileMap.getHeight();
        auto& tileVec = data.colliderReferences.tilesCollisionMap[map];
        for (const auto layer : layers)
        {
            if (layer > tileMap.getTileLayerCount())
            {
                LOG_WARNING("Given tilemap does not contain a layer with index: %d. Check TileMap.h for more info",
                            layer);
                continue;
            }

            const auto* start = tileMap.getLayerData(layer);
            for (int i = 0; i < mapHeight; ++i)
            {
                const auto yOff = i * mapWidth;
                for (int j = 0; j < mapWidth; ++j)
                {
                    // tile data is 1 more so that empty is 0
                    const auto tileNum = static_cast<uint16_t>(start[yOff + j] - 1);
                    if (tileNum == UINT16_MAX) // uint overflows to maximum value (0-1 = MAX)
                        continue;
                    const auto infoIt = data.markedTilesMap.find(tileNum);
                    if (infoIt != data.markedTilesMap.end())
                    {
                        const auto& info = infoIt->second;
                        auto scaled = info.bounds.scale(data.tileSetScale);
                        scaled += Point{(float)j, (float)i} * tileSize;
                        if (scaled.w == 0) // rect is 0 if not assigned - so adding to x and y is always valid
                        {
                            scaled.w = tileSize;
                            scaled.h = tileSize;
                        }
                        const auto objectNum = data.colliderStorage.insert(scaled);
                        tileVec.push_back(objectNum);
                        const auto tileClass = infoIt->second.tileClass;
                        grid.insert(StaticIDHelper::CreateID(objectNum, (int)tileClass), scaled.x, scaled.y, scaled.w,
                                    scaled.h);
                    }
                }
            }
        }
        global::PATH_DATA.updateStaticPathGrid(map);
    }

    void RemoveTileCollisions(const MapID map)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (!data.colliderReferences.tilesCollisionMap.contains(map))
        {
            LOG_WARNING("No tilemap collisions have been loaded for this map!");
            return;
        }

        const auto& colliderIdVec = data.colliderReferences.tilesCollisionMap.at(map);
        auto& hashGrid = data.mapTileGrids[map];
        for (const auto id : colliderIdVec)
        {
            data.colliderStorage.remove(id);
        }
        hashGrid.clear(); // We can clear as tile collisions can only occur once per map
        data.colliderReferences.tilesCollisionMap.erase(map);
        global::PATH_DATA.updateStaticPathGrid(map);
    }

    //----------------- MANUAL GROUPS -----------------//

    int MANUAL_GROUP_ID = 0;

    ManualColliderGroup::ManualColliderGroup() : id(MANUAL_GROUP_ID++) {}

    void ManualColliderGroup::addRect(const float x, const float y, const float width, const float height)
    {
        colliders.push_back({{x, y, width, height}});
    }

    void ManualColliderGroup::addRectCentered(const float x, const float y, const float width, const float height)
    {
        colliders.push_back({{x - width / 2, y - height / 2, width, height}});
    }

    int ManualColliderGroup::getID() const { return id; }

    const std::vector<StaticCollider>& ManualColliderGroup::getColliders() const { return colliders; }

    void AddColliderGroup(const MapID map, const ManualColliderGroup& group)
    {
        auto& data = global::STATIC_COLL_DATA;
        auto& mapGroupInfoVec = data.colliderReferences.groupMap[map];
        for (const auto& info : mapGroupInfoVec)
        {
            if (info.groupId == group.getID())
            {
                LOG_WARNING("Passed group was already loaded for the given map!: %d", info.groupId);
                return;
            }
        }

        if (!data.mapGroupGrids.contains(map))
            data.mapGroupGrids.add(map);

        ObjectReferenceHolder::ManualGroupInfo groupInfo;
        groupInfo.groupId = group.getID();

        auto& hashGrids = data.mapGroupGrids[map];
        for (const auto& collider : group.getColliders())
        {
            const auto& [x, y, w, h] = collider.bounds;
            const auto num = data.colliderStorage.insert(collider.bounds);
            const auto staticID = StaticIDHelper::CreateID(num, groupInfo.groupId);
            hashGrids.insert(staticID, x, y, w, h);
            groupInfo.objectIds.push_back(num); // So we can uniquely delete later
        }
        mapGroupInfoVec.push_back(std::move(groupInfo));
        global::PATH_DATA.updateStaticPathGrid(map);
    }

    void RemoveColliderGroup(const MapID map, const ManualColliderGroup& group)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (!data.colliderReferences.groupMap.contains(map))
        {
            LOG_WARNING("Tile data for this map was already loaded. Remove it before you can load it again");
            return;
        }

        auto& mapGroupInfoVec = data.colliderReferences.groupMap.at(map);
        auto& hashGrid = data.mapGroupGrids[map];
        for (const auto& info : mapGroupInfoVec)
        {
            if (info.groupId == group.getID())
            {
                for (const auto id : info.objectIds)
                {
                    data.colliderStorage.remove(id);
                    hashGrid.removeIfWithHoles(id, [](const uint32_t num, const StaticID id)
                                               { return num == StaticIDHelper::GetObjectNum(id); });
                }
                hashGrid.patchHoles();
                // Remove the entry in the map specific info vec
                const auto pred = [&](const ObjectReferenceHolder::ManualGroupInfo& ele)
                {
                    return ele.groupId == info.groupId;
                };
                std::erase_if(mapGroupInfoVec, pred);
                global::PATH_DATA.updateStaticPathGrid(map);
                return;
            }
        }
        LOG_WARNING("Passed group was not loaded for the given map! GroupID: %d", group.getID());
    }

} // namespace magique
