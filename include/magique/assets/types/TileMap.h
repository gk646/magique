#ifndef MAGIQUE_TILEMAP_H
#define MAGIQUE_TILEMAP_H

#include <vector>
#include <magique/util/Defines.h>

namespace magique
{

    struct TileMap final
    {
        std::vector<uint16_t> tileValues; // Flattened array for the tile numbers
        uint32_t layerIndices[MAGIQUE_MAX_TILEMAP_LAYERS]{};
        uint8_t layerCount = 0;
        TileMap(std::vector<uint16_t>&& tileValues); // Internal constructor
    };

} // namespace magique

#endif //MAGIQUE_TILEMAP_H