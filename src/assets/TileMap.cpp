// SPDX-License-Identifier: zlib-acknowledgement
#include <cstring>
#include <algorithm>
#include <magique/assets/types/TileMap.h>

#include "magique/util/Math.h"

#include <magique/assets/types/TileSet.h>
#include <magique/util/Datastructures.h>
#include <magique/util/Logging.h>

namespace magique
{
    TiledObject* TiledObjectLayer::operator[](int objectId)
    {
        for (auto& object : objects)
        {
            if (object.getID() == objectId)
                return &object;
        }
        return nullptr;
    }

    TiledObject* TiledObjectLayer::operator[](std::string_view name)
    {
        for (auto& object : objects)
        {
            if (object.getName() == name)
                return &object;
        }
        return nullptr;
    }

    const TileID& TiledTileLayer::operator()(size_t x, size_t y) const { return tiles[x + y * (size_t)dims.x]; }

    TileID& TiledTileLayer::operator()(size_t x, size_t y) { return tiles[x + y * (size_t)dims.x]; }

    const TiledTileLayer& TileMap::operator[](size_t layer) const { return tileLayers[layer]; }

    TiledTileLayer& TileMap::operator[](size_t layer) { return tileLayers[layer]; }

    const TiledTileLayer& TileMap::getLayer(std::string_view name) const
    {
        const auto it = std::ranges::find_if(tileLayers, [&](const auto& l) { return l.name == name; });
        if (it == tileLayers.end())
        {
            LOG_ERROR("No such tile layer: %s", name.data());
            return tileLayers.front();
        }
        return *it;
    }

    TiledTileLayer& TileMap::getLayer(std::string_view name)
    {
        const auto it = std::ranges::find_if(tileLayers, [&](const auto& l) { return l.name == name; });
        if (it == tileLayers.end())
        {
            LOG_ERROR("No such tile layer: %s", name.data());
            return tileLayers.front();
        }
        return *it;
    }

    const std::vector<TiledTileLayer>& TileMap::getTileLayers() const { return tileLayers; }

    std::vector<TiledTileLayer>& TileMap::getTileLayers() { return tileLayers; }

    bool TileMap::hasObjectLayer(std::string_view layer) const
    {
        const auto it = std::ranges::find_if(objectLayers, [&](const auto& l) { return l.name == layer; });
        return it != objectLayers.end();
    }

    TiledObjectLayer& TileMap::getObjectLayer(std::string_view name)
    {
        const auto it = std::ranges::find_if(objectLayers, [&](const auto& l) { return l.name == name; });
        if (it == objectLayers.end())
        {
            LOG_ERROR("No such objects layer: %s", name.data());
            return objectLayers.front();
        }
        return *it;
    }

    const TiledObjectLayer& TileMap::getObjectLayer(std::string_view name) const
    {
        const auto it = std::ranges::find_if(objectLayers, [&](const auto& l) { return l.name == name; });
        if (it == objectLayers.end())
        {
            LOG_ERROR("No such objects layer: %s", name.data());
            return objectLayers.front();
        }
        return *it;
    }

    const std::vector<TiledObjectLayer>& TileMap::getObjectLayers() const { return objectLayers; }

    std::vector<TiledObjectLayer>& TileMap::getObjectLayers() { return objectLayers; }

    void TileMap::updateAnimations(const TileSet& tileset, bool rebuild)
    {
        const auto& animations = tileset.getAnimations();
        if (!builtAnimationCache || rebuild) [[unlikely]]
        {
            animatedTiles.reserve(256);
            HashMap<int16_t, int> idToAnimationMap; // Improves lookup speed - avoids iteration each time
            for (int a = 0; a < (int)animations.size(); a++)
            {
                idToAnimationMap[animations[a].baseTile] = a;
            }

            for (int l = 0; l < (int)getTileLayers().size(); l++)
            {
                const auto& layer = getTileLayers()[l];
                for (int y = 0; y < layer.dims.y; y++)
                {
                    for (int x = 0; x < layer.dims.x; x++)
                    {
                        const auto& tile = layer(x, y);
                        const auto it = idToAnimationMap.find(tile.id);
                        if (it != idToAnimationMap.end())
                        {
                            AnimatedTile& newTile = animatedTiles.emplace_back();
                            newTile.x = x;
                            newTile.y = y;
                            newTile.layer = l;
                            newTile.animation = it->second;
                            newTile.millis = 0;
                        }
                    }
                }
            }
            builtAnimationCache = true;
        }

        for (auto& info : animatedTiles)
        {
            const auto& anim = animations[info.animation];
            TileID& tile = getTileLayers()[info.layer](info.x, info.y);

            int16_t next = anim.baseTile;
            int accum = 0;

            for (const auto& [id, duration] : anim.entries)
            {
                accum += duration;
                if (info.millis < accum)
                {
                    next = id;
                    break;
                }
            }

            tile.id = next;

            info.millis += int(MAGIQUE_TICK_TIME * 1000.0F);
            if (info.millis > anim.duration)
                info.millis = 0;
        }
    }

    void TileMap::randomizeAnimations(const TileSet& tileset)
    {
        for (auto& info : animatedTiles)
        {
            const auto& anim = tileset.getAnimations()[info.animation];
            info.millis = MathRandom(0, anim.duration);
        }
    }

    void TileMap::syncAnimations()
    {
        for (auto& info : animatedTiles)
        {
            info.millis = 0;
        }
    }

    const TiledProperty* TileMap::getProperty(const std::string_view& name) const
    {
        const auto it = std::ranges::find_if(properties, [&](const auto& p) { return p.getName() == name; });
        if (it == properties.end())
        {
            return nullptr;
        }
        return &(*it);
    }

    bool TileMap::hasProperty(std::string_view name) const { return getProperty(name) != nullptr; }

    Point TileMap::getDims() const { return {(float)width, (float)height}; }

    Point TileMap::getPixelDims() const { return getDims() * tileSize; }

    int TileMap::getTileSize() const { return tileSize; }

} // namespace magique
