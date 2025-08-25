// SPDX-License-Identifier: zlib-acknowledgement
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
        TileSet() = default;

        // Returns the vector containing all marked tiles
        [[nodiscard]] const std::vector<TileInfo>& getTileInfo() const;

        // Returns the tile size parsed from the tileset data
        [[nodiscard]] int getTileSize() const;

    private:
        int tileSize = -1; // Default
        std::vector<TileInfo> infoVec;
        friend TileSet ImportTileSet(Asset);
    };

} // namespace magique

#endif //MAGIQUE_TILESET_H