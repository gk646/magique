#include <magique/assets/types/TileSheet.h>
#include <magique/util/Macros.h>
#include <magique/core/Types.h>

namespace magique
{
    TileSheet::TileSheet(const Asset& asset)
    {

       
    }

    TileSheet::TileSheet(const std::vector<const Asset&>& assets)
    {

    }

    TextureRegion TileSheet::getRegion(uint16_t tileNum) const
    {
        M_ASSERT(offsets.size() > tileNum, "No texture stored for that tileNum");

        const uint32_t texOff = offsets[tileNum];
        return {static_cast<uint16_t>(texOff), static_cast<uint16_t>(texOff << 16), texSize, texSize, textureID};
    }
} // namespace magique