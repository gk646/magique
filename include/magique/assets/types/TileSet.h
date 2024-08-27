#ifndef MAGIQUE_TILESET_H
#define MAGIQUE_TILESET_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// TileSet
//-----------------------------------------------
// .....................................................................
// The tileset stores meta data about tiles and allows to retrieve it
// .....................................................................

namespace magique
{
    struct TileSet final
    {
        [[nodiscard]] const std::vector<TileInfo>& getTileInfo() const;

    private:
        explicit TileSet(const Asset& asset);
        std::vector<TileInfo> infoVec;
        friend handle RegisterTileSet(const Asset&);
    };

} // namespace magique

#endif //MAGIQUE_TILESET_H