// SPDX-License-Identifier: zlib-acknowledgement
#include <cstring>
#include <magique/assets/types/TileMap.h>
#include <magique/util/Logging.h>

namespace magique
{
    int16_t& TileMap::getTileIndex(const int x, const int y, const int layer)
    {
        auto& data = tileLayers[layer];
        return data[x + (y * width)];
    }

    int16_t TileMap::getTileIndex(const int x, const int y, const int layer) const
    {
        return getLayerData(layer)[x + y * width];
    }

    const int16_t* TileMap::getLayerData(const int layer) const
    {
        MAGIQUE_ASSERT(layer < static_cast<int>(tileLayers.size()), "TileMap does not have that many tile layers");
        return tileLayers[layer].data();
    }

    Point TileMap::getDims() const { return {(float)width, (float)height}; }

    Point TileMap::getPixelDims() const { return getDims() * tileSize; }

    int TileMap::getTileSize() const { return tileSize; }

    int TileMap::getTileLayerCount() const { return static_cast<int>(tileLayers.size()); }

    int TileMap::getObjectLayerCount() const { return static_cast<int>(objectLayers.size()); }

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

    const TiledProperty* TileMap::getProperty(const char* name) const
    {
        for (const auto& property : properties)
        {
            if (property.getName() == nullptr)
            {
                continue;
            }
            if (strcmp(property.getName(), name) == 0)
            {
                return &property;
            }
        }
        return nullptr;
    }

} // namespace magique
