// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/assets/types/TileMap.h>
#include <magique/internal/Macros.h>

namespace magique
{
    uint16_t& TileMap::getTileIndex(const int x, const int y, const int layer)
    {
        auto& data = tileLayers[layer];
        return data[x + (y * width)];
    }

    uint16_t TileMap::getTileIndex(const int x, const int y, const int layer) const
    {
        return getLayerData(layer)[x + y * width];
    }

    const uint16_t* TileMap::getLayerData(const int layer) const
    {
        MAGIQUE_ASSERT(layer < static_cast<int>(tileLayers.size()), "TileMap does not have that many tile layers");
        return tileLayers[layer].data();
    }

    int TileMap::getWidth() const { return width; }

    int TileMap::getHeight() const { return height; }

    int TileMap::getTileLayerCount() const { return static_cast<int>(tileLayers.size()); }

    int TileMap::getObjectLayerCount() const { return static_cast<int>(objectLayers.size()); }

    std::vector<TileObject>& TileMap::getObjects(const int layer)
    {
        MAGIQUE_ASSERT(layer < static_cast<int>(objectLayers.size()), "TileMap does not have that many object layers");
        return objectLayers[layer];
    }

} // namespace magique