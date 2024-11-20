#ifndef MAGIQUE_TILESET_H
#define MAGIQUE_TILESET_H

#include <vector>
#include <magique/core/Types.h>

//===============================================
// TileSet
//===============================================
// .....................................................................
// The tileset stores metadata about tiles and allows to retrieve it
// .....................................................................

namespace magique
{
    struct TileSet final
    {
        // Returns the vector containing all marked tiles
        [[nodiscard]] const std::vector<TileInfo>& getTileInfo() const;

        // Returns the tile size parsed from the tileset data
        [[nodiscard]] int getTileSize() const;

    private:
        explicit TileSet(const Asset& asset);
        int tileSize = -1; // Default
        std::vector<TileInfo> infoVec;
        friend handle RegisterTileSet(Asset);
    };

} // namespace magique

#endif //MAGIQUE_TILESET_H