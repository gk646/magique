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
            if (tileInfo.hasCollision())
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
                hitbox = hitbox.mirrorDiagonal();
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

} // namespace magique
