#ifndef MAGIQUE_TILEMAP_H
#define MAGIQUE_TILEMAP_H

#include <vector>
#include <magique/fwd.hpp>
#include <magique/util/Defines.h>

//-----------------------------------------------
// TileMap
//-----------------------------------------------
// .....................................................................
// A tilemap defines the actual map data by storing numbers that correspond to textures.
// Thus to draw a tilemap a tilesheet is needed that converts those numbers into visuals.
// .....................................................................

namespace magique
{
    struct TileMap final
    {
        // Returns a modifiable reference to the tilenum at the given position
        uint16_t& getTileNum(int x, int y, int layer);

        // Returns the pointer to the start of the layer
        [[nodiscard]] const uint16_t* getLayerStart(int layer) const;

        // Returns the start index of the given layer
        [[nodiscard]] int getLayerStartIndex(int layer) const;

        // Returns the width of each layer
        [[nodiscard]] int getWidth() const { return width; }

        // Returns the height of each layer
        [[nodiscard]] int getHeight() const { return height; }

        // Returns the layer count
        [[nodiscard]] int getLayerCount() const { return layerCount; }

    private:
        explicit TileMap(const Asset& asset);

        std::vector<uint16_t> layerData;                     // Flattened array for the tile numbers
        uint32_t layerIndices[MAGIQUE_MAX_TILEMAP_LAYERS]{}; // Layer start indices
        uint8_t layerCount = 0;                              // Layer count
        int16_t width = 0;
        int16_t height = 0;
        friend handle RegisterTileMap(const Asset&);
    };

} // namespace magique

#endif //MAGIQUE_TILEMAP_H