#include <magique/assets/types/TileSheet.h>
#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/util/Defines.h>

#include "assets/LoadWrappers.h"

namespace magique
{
    TileSheet::TileSheet(const Asset& asset, int textureSize, float scale)
    {
        auto img = internal::LoadImage(asset);
        auto texImage = GenImageColor(MAGIQUE_TEXTURE_ATLAS_WIDTH, MAGIQUE_TEXTURE_ATLAS_HEIGHT, BLANK);

        Rectangle src{0, 0, (float)textureSize, (float)textureSize};
        Rectangle dst{0, 0, std::floor(src.width * scale), std::floor(src.height * scale)};

        texPerRow = texImage.width / static_cast<int>(dst.width);
        texSize = static_cast<int16_t>(dst.width);

        while (src.y < img.height)
        {
            for (int i = 0; i < texPerRow; ++i)
            {
                ImageDraw(&texImage, img, src, dst, WHITE);
                src.x += src.width;
                dst.x += dst.width;
                if (src.x >= img.width)
                {
                    src.x = 0;
                    src.y += src.height;
                }
            }
            dst.x = 0;
            dst.y += dst.height;
            if (dst.y >= texImage.height)
            {
                LOG_ERROR("TileSheet doesnt fit into a single atlas! Skipping: %s", asset.name);
                break;
            }
        }

        const auto tex = LoadTextureFromImage(texImage);
        if (tex.id == 0)
        {
            LOG_ERROR("Failed to load tilesheet to GPU: %s", asset.name);
        }
        textureID = tex.id;

        UnloadImage(texImage);
        UnloadImage(img);
    }

    TileSheet::TileSheet(const std::vector<const Asset*>& assets, int textureSize, float scale) {}

    TextureRegion TileSheet::getRegion(const uint16_t tileNum) const
    {
        if (tileNum == 0)
        {
            return {static_cast<int16_t>(MAGIQUE_TEXTURE_ATLAS_WIDTH - texSize),
                    static_cast<int16_t>(MAGIQUE_TEXTURE_ATLAS_HEIGHT - texSize), texSize, texSize, textureID};
        }
        // M_ASSERT(offsets.size() > tileNum, "No texture stored for that tileNum");
        //const uint32_t texOff = 1;

        const int16_t row = (tileNum-1) / texPerRow;
        const int16_t colum = (tileNum-1) - row * texPerRow;

        return {static_cast<int16_t>(colum * texSize), static_cast<int16_t>(row * texSize), texSize, texSize,
                textureID};

        // return {static_cast<uint16_t>(texOff), static_cast<uint16_t>(texOff << 16), texSize, texSize, textureID};
    }
} // namespace magique