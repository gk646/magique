// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_TILEMAP_H
#define MAGIQUE_TILEMAP_H

#include <vector>
#include <magique/core/Types.h>

//===============================================
// TileMap
//===============================================
// .....................................................................
// A tilemap defines the actual map data by storing numbers that correspond to textures.
// Thus, to draw a tilemap a TileSheet is needed that converts those numbers into visuals.
// The numbering is top to bottom with both tile and objects layers having own counters
// .....................................................................

namespace magique
{
    struct TileMap final
    {
        TileMap() = default;

        //================= TILES =================//

        // Returns a modifiable reference to the tile index at the given position
        int16_t& getTileIndex(int x, int y, int layer);

        // Returns the tile index at the given position
        [[nodiscard]] int16_t getTileIndex(int x, int y, int layer) const;

        // Returns the pointer to the start of the layer
        // layers are counted from top to bottom (in the editor), only counting tile layers
        // Note: the length the of the layer data (for on layer) is: getWidth() * getHeight()
        [[nodiscard]] const int16_t* getLayerData(int layer) const;

        // Returns the width of each layer
        [[nodiscard]] int getWidth() const;

        // Returns the height of each layer
        [[nodiscard]] int getHeight() const;

        // Returns the layer count
        [[nodiscard]] int getTileLayerCount() const;

        //================= OBJECTS =================//

        [[nodiscard]] int getObjectLayerCount() const;

        // Returns a modifiable reference to the objects in the given layer
        // layers are counted from top to bottom (in the editor), only counting object layers!
        std::vector<TileObject>& getObjects(int layer);
        const std::vector<TileObject>& getObjects(int layer) const;

        //================= MISC =================//

        const char* getName();

    private:
        std::vector<std::vector<TileObject>> objectLayers;
        std::vector<std::vector<int16_t>> tileLayers; // Contiguous array for map data
        int width = 0, height = 0;
        const char* name = nullptr;
        friend TileMap ImportTileMap(Asset);
    };

} // namespace magique

#endif //MAGIQUE_TILEMAP_H