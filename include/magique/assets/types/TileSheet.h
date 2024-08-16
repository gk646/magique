#ifndef MAGIQUE_TILESHEET_H
#define MAGIQUE_TILESHEET_H

#include <vector>
#include <magique/fwd.hpp>

//-----------------------------------------------
// TileSheet
//-----------------------------------------------
// ................................................................................
// Defines the textures associated with the tile numbers from a TileMap
// You should only have 1 per project and all TileMaps should use that TileSheet
// But if you want you can have multiple - just pay attention to what map is using which indices to map textures correctly
// Note that after creating a TileSheet its final and cannot be changed
// Only supports quadratic textures and no rotations
// ................................................................................

namespace magique
{
    struct TileSheet final
    {
        // Returns the region for the given tileNum
        [[nodiscard]] TextureRegion getRegion(uint16_t tileNum) const;

        [[nodiscard]] Vector2 getOffset(uint16_t tileNum) const;

    private:
        explicit TileSheet(const Asset& asset, int size, float scale);
        explicit TileSheet(const std::vector<const Asset*>& assets, int size, float scale);

        int16_t texSize;        // Size of each texture
        uint16_t texPerRow = 0; // Textures per row
        uint16_t textureID = 0; // id of the underlying texture
    };
} // namespace magique

#endif //MAGIQUE_TILESHEET_H