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
    void CollisionSetWorldBounds(const Rectangle& rectangle) { global::STATIC_COLL_DATA.worldBounds = rectangle; }

    //----------------- TILE OBJECTS -----------------//

    void CollisionAddTileObjects(const MapID map, const std::vector<TileObject>& collisionObjects, const float scale)
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
            grid.insert(staticID, scaled.x, scaled.y, scaled.width, scaled.height);
            info.objectIds.push_back(num); // So we can uniquely delete later
        }

        // Insert the info
        mapTileObjectVec.push_back(std::move(info));
        global::PATH_DATA.updateStaticPathGrid(map);
    }

    void CollisionRemoveTileObjects(const MapID map, const std::vector<TileObject>& collisionObjects)
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

    void CollisionSetTileset(const TileSet& tileSet, const float scale)
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
                data.markedTilesMap[tileInfo.tileID + 1] = tileInfo;
            }
        }
    }

    void CollisionAddTiles(const MapID map, const TileMap& tileMap, const std::initializer_list<int>& layers)
    {
        auto& data = global::STATIC_COLL_DATA;
        if (data.tileSet == nullptr)
        {
            LOG_WARNING("Cannot load tile collision data without tile set. Use CollisionSetTileset()");
            return;
        }

        if (data.colliderReferences.tilesCollisionMap.contains(map))
        {
            LOG_WARNING("Tile data for this map was already loaded. Remove it before you can load it again");
            return;
        }

        auto& grid = data.mapTileGrids[map];
        const auto tileSize = static_cast<float>(data.tileSet->getTileSize());
        const int mapWidth = tileMap.getDims().x;
        const int mapHeight = tileMap.getDims().y;
        auto& tileVec = data.colliderReferences.tilesCollisionMap[map];

        auto insertTile = [&](Point tilePos, TileID tile, Rect hitbox, TileClass tileClass)
        {
            if (tile.flippedDiagonal)
            {
                hitbox = hitbox.mirrorDiagonal(tileSize / 2);
            }

            if (tile.flippedHorizontal)
            {
                hitbox = hitbox.mirrorHorizontal(tileSize / 2);
            }

            if (tile.flippedVertical)
            {
                hitbox = hitbox.mirrorVertical(tileSize / 2);
            }

            hitbox += tilePos;
            hitbox = hitbox.scale(data.tileSetScale);

            if (hitbox.size() == 0)
            {
                return;
            }

            const auto objectNum = data.colliderStorage.insert(hitbox);
            tileVec.push_back(objectNum);
            const auto objectId = StaticIDHelper::CreateID(objectNum, (int)tileClass);
            grid.insert(objectId, hitbox.x, hitbox.y, hitbox.width, hitbox.height);
        };

        for (const auto layer : layers)
        {
            if (layer > (int)tileMap.getTileLayers().size())
            {
                LOG_WARNING("Tilemap does not contain a layer with index: %d. Check TileMap.h for more info", layer);
                continue;
            }

            const auto* start = tileMap[layer].tiles.data();
            for (int i = 0; i < mapHeight; ++i)
            {
                const auto yOff = i * mapWidth;
                for (int j = 0; j < mapWidth; ++j)
                {
                    auto tile = start[yOff + j];
                    if (tile.id == 0)
                        continue;

                    const auto infoIt = data.markedTilesMap.find(tile.id);
                    if (infoIt == data.markedTilesMap.end())
                    {
                        continue;
                    }
                    const auto& info = infoIt->second;
                    const auto tilePos = Point{(float)j, (float)i} * tileSize;

                    insertTile(tilePos, tile, info.bounds, info.tileClass);
                    insertTile(tilePos, tile, info.secBounds, info.tileClass);
                }
            }
        }
        global::PATH_DATA.updateStaticPathGrid(map);
    }

    void CollisionRemoveTiles(const MapID map)
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

    ColliderGroup::ColliderGroup() : id(MANUAL_GROUP_ID++) {}

    void ColliderGroup::addRect(const float x, const float y, const float width, const float height)
    {
        colliders.push_back({{x, y, width, height}});
    }

    void ColliderGroup::addRectCentered(const float x, const float y, const float width, const float height)
    {
        colliders.push_back({{x - width / 2, y - height / 2, width, height}});
    }

    int ColliderGroup::getID() const { return id; }

    const std::vector<StaticCollider>& ColliderGroup::getColliders() const { return colliders; }

    void CollisionAddGroup(const MapID map, const ColliderGroup& group)
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

    void CollisionRemoveGroup(const MapID map, const ColliderGroup& group)
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
