#ifndef MAGIQUE_SPRITESHEET_H
#define MAGIQUE_SPRITESHEET_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// TileSheet
//-----------------------------------------------

// Defines the textures associated with the tile numbers from a TileMap
// You should only have 1 per project and all TileMaps should use that TileSheet
// But if you want you can have multiple - just pay attention to what map is using which indices to map textures correctly

namespace magique
{
    struct TileSheet final
    {
        explicit TileSheet(const Asset& asset);                      // Internal constructor
        explicit TileSheet(const std::vector<const Asset&>& assets); // Internal constructor


        [[nodiscard]] TextureRegion getRegion(uint16_t tileNum) const;
    };

} // namespace magique

#endif //MAGIQUE_SPRITESHEET_H