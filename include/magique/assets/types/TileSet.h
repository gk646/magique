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
    struct TileAnimation final
    {
        int duration;
        int16_t baseTile;

        struct Entry
        {
            int16_t tile;
            int duration;
        };

        std::vector<Entry> entries;
    };

    struct TileSet final
    {
        TileSet() = default;

        // Returns the vector containing all marked tiles
        const std::vector<TileInfo>& getTilesInfo() const;

        // Returns the tile size parsed from the tileset data
        int getTileSize() const;

        // Returns the info or nullptr (or error and the first info)
        TileInfo& getInfo(uint8_t tileId);
        const TileInfo* getInfo(uint8_t tileId) const;
        const TileInfo* getInfo(std::string_view image) const;

        // Returns the amount of total tiles in the set (maximum amount)
        int getTileCount() const;

        const std::vector<TileAnimation>& getAnimations() const;

    private:
        M_MAKE_PUB()
        int tileSize = -1; // Default
        int tileCount = 0;
        std::vector<TileInfo> infoVec;
        std::vector<TileAnimation> animations;
    };

} // namespace magique

#endif // MAGIQUE_TILESET_H
