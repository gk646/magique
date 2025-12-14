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
        [[nodiscard]] const std::vector<TileInfo>& getTilesInfo() const;

        // Returns the tile size parsed from the tileset data
        [[nodiscard]] int getTileSize() const;

        TileInfo& getInfo(uint8_t tileId);
        const TileInfo* getInfo(uint8_t tileId) const;
        const TileInfo* getInfo(const char* image) const;

        // Returns the amount of total tiles in the set (maximum amount)
        int getTileCount() const;

    private:
        int tileSize = -1; // Default
        int tileCount = 0;
        std::vector<TileInfo> infoVec;
        friend TileSet ImportTileSet(const Asset&, TileClassMapFunc);
    };

} // namespace magique

#endif //MAGIQUE_TILESET_H