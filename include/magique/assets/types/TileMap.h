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
    struct TiledObjectLayer final
    {
        std::string name;
        std::vector<TileObject> objects;

        auto begin() const { return objects.begin(); }
        auto end() const { return objects.end(); }
    };

    struct TileMap final
    {
        TileMap() = default;

        //================= TILES =================//

        // Returns a modifiable reference to the tile index at the given position
        int16_t& getTileIndex(int x, int y, int layer);

        // Returns the tile index at the given position
        [[nodiscard]] int16_t getTileIndex(int x, int y, int layer) const;

        // Returns the pointer to the start of the layer
        // Layers are sorted bottom up as shown in the editor - only counting tile layers
        // Note: the length the of the layer data (for on layer) is: getWidth() * getHeight()
        [[nodiscard]] const int16_t* getLayerData(int layer) const;

        // Returns the layer count
        [[nodiscard]] int getTileLayerCount() const;

        //================= OBJECTS =================//

        [[nodiscard]] int getObjectLayerCount() const;

        // Returns true if a object layer with the given name is present
        bool hasObjectLayer(std::string_view layer) const;

        // Returns the object layer with the given name
        // Failure: Returns the first layer and error
        TiledObjectLayer& getObjectLayer(std::string_view name);
        const TiledObjectLayer& getObjectLayer(std::string_view name) const;

        const std::vector<TiledObjectLayer>& getObjectLayers() const;

        //================= MISC =================//

        // Returns: the property with the given name or nullptr if not exists
        const TiledProperty* getProperty(const char* name) const;

        // Returns the width hand height of the tile layers (in tiles)
        Point getDims() const;

        // Returns the dimensions in pixels
        Point getPixelDims() const;

        int getTileSize() const;

    private:
        M_MAKE_PUB()
        std::vector<TiledObjectLayer> objectLayers;
        std::vector<std::vector<int16_t>> tileLayers; // Contiguous array for map data
        std::vector<TiledProperty> properties;
        int width = 0, height = 0;
        int tileSize = 0;
    };

} // namespace magique

#endif // MAGIQUE_TILEMAP_H
