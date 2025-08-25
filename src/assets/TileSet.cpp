// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/assets/types/TileSet.h>


namespace magique
{
    const std::vector<TileInfo>& TileSet::getTileInfo() const { return infoVec; }

    int TileSet::getTileSize() const { return tileSize; }

} // namespace magique