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

    struct TiledTileLayer final
    {
        std::string name;
        Point dims;
        std::vector<TileID> tiles;

        // Access by given coordinate
        const TileID& operator()(size_t x, size_t y) const;
        TileID& operator()(size_t x, size_t y);

        auto begin() const { return tiles.begin(); }
        auto end() const { return tiles.end(); }
    };

    struct TileMap final
    {
        TileMap() = default;

        //================= TILES =================//

        const TiledTileLayer& operator[](size_t layer) const;
        TiledTileLayer& operator[](size_t layer);

        const TiledTileLayer& getLayer(std::string_view name) const;
        TiledTileLayer& getLayer(std::string_view name);

        const std::vector<TiledTileLayer>& getTileLayers() const;
        std::vector<TiledTileLayer>& getTileLayers();

        // Allows to iterate tile layers
        auto begin() const { return tileLayers.begin(); }
        auto end() const { return tileLayers.end(); }
        auto begin() { return tileLayers.begin(); }
        auto end() { return tileLayers.end(); }

        //================= OBJECTS =================//


        // Returns true if a object layer with the given name is present
        bool hasObjectLayer(std::string_view layer) const;

        // Returns the object layer with the given name
        // Failure: Returns the first layer and error
        TiledObjectLayer& getObjectLayer(std::string_view name);
        const TiledObjectLayer& getObjectLayer(std::string_view name) const;

        // Returns a vector with all object layers
        const std::vector<TiledObjectLayer>& getObjectLayers() const;
        std::vector<TiledObjectLayer>& getObjectLayers();

        //================= MISC =================//

        // Returns: the property with the given name or nullptr if not exists
        const TiledProperty* getProperty(const std::string_view& name) const;

        // Returns the width hand height of the tile layers (in tiles)
        Point getDims() const;

        // Returns the dimensions in pixels
        Point getPixelDims() const;

        int getTileSize() const;

    private:
        M_MAKE_PUB()
        std::vector<TiledObjectLayer> objectLayers;
        std::vector<TiledTileLayer> tileLayers; // Contiguous array for map data
        std::vector<TiledProperty> properties;
        int width = 0, height = 0;
        int tileSize = 0;
    };

} // namespace magique

#endif // MAGIQUE_TILEMAP_H
