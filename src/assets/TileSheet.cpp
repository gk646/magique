#include <cmath>
#include <raylib/raylib.h>

#include <magique/assets/types/TileSheet.h>
#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/util/Defines.h>

#include "assets/headers/LoadWrappers.h"

namespace magique
{
    TileSheet::TileSheet(const Asset& asset, const int textureSize, const float scale)
    {
        const auto img = internal::LoadImage(asset);
        auto texImage = GenImageColor(MAGIQUE_TEXTURE_ATLAS_WIDTH, MAGIQUE_TEXTURE_ATLAS_HEIGHT, BLANK);

        Rectangle src{0, 0, static_cast<float>(textureSize), static_cast<float>(textureSize)};
        Rectangle dst{0, 0, std::floor(src.width * scale), std::floor(src.height * scale)};

        texPerRow = static_cast<uint16_t>(texImage.width / static_cast<int>(dst.width));
        texSize = static_cast<int16_t>(dst.width);

        // TODO Can be optimized a lot by manually doing the image resizing and reusing a buffer
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
                LOG_ERROR("TileSheet doesnt fit into a single atlas! Skipping: %s", asset.path);
                break;
            }
        }

        const auto tex = LoadTextureFromImage(texImage);
        if (tex.id == 0)
        {
            LOG_ERROR("Failed to load tilesheet to GPU: %s", asset.path);
        }
        textureID = static_cast<uint16_t>(tex.id);

        UnloadImage(texImage);
        UnloadImage(img);
    }

    TileSheet::TileSheet(const std::vector<const Asset*>& assets, const int textureSize, const float scale) {}

    TextureRegion TileSheet::getRegion(const uint16_t tileNum) const
    {
        if (tileNum == 0)
        {
            return {static_cast<uint16_t>(MAGIQUE_TEXTURE_ATLAS_WIDTH - texSize),
                    static_cast<uint16_t>(MAGIQUE_TEXTURE_ATLAS_HEIGHT - texSize), texSize, texSize, textureID};
        }

        const int row = (tileNum - 1) / texPerRow;
        const int colum = tileNum - 1 - row * texPerRow;

        return {static_cast<uint16_t>(colum * texSize), static_cast<uint16_t>(row * texSize), texSize, texSize,
                textureID};
    }

    Vector2 TileSheet::getOffset(const uint16_t tileNum) const
    {
        if (tileNum == 0)
        {
            return {static_cast<float>(MAGIQUE_TEXTURE_ATLAS_WIDTH - texSize),
                    static_cast<float>(MAGIQUE_TEXTURE_ATLAS_HEIGHT - texSize)};
        }
        const int row = (tileNum - 1) / texPerRow;
        const int colum = tileNum - 1 - row * texPerRow;

        return {static_cast<float>(colum * texSize), static_cast<float>(row * texSize)};
    }

    float TileSheet::getTextureSize() const { return texSize; }

    unsigned int TileSheet::getTextureID() const { return textureID; }

} // namespace magique