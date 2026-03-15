// SPDX-License-Identifier: zlib-acknowledgement
#include <cstring>
#include <algorithm>
#include <magique/assets/types/TileMap.h>
#include <magique/util/Logging.h>

namespace magique
{
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

    const TiledProperty* TileMap::getProperty(const std::string_view& name) const
    {
        const auto it = std::ranges::find_if(properties, [&](const auto& p) { return p.getName() == name; });
        if (it == properties.end())
        {
            return nullptr;
        }
        return &(*it);
    }

    Point TileMap::getDims() const { return {(float)width, (float)height}; }

    Point TileMap::getPixelDims() const { return getDims() * tileSize; }

    int TileMap::getTileSize() const { return tileSize; }

} // namespace magique
