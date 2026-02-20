// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>

#include <magique/assets/types/TileSheet.h>
#include <magique/assets/types/Asset.h>
#include <magique/assets/AssetImport.h>
#include <magique/core/Types.h>
#include <magique/util/Logging.h>

namespace magique
{
    TileSheet::TileSheet(const Asset& asset, const int textureSize, const float scale)
    {
        const auto img = ImportImage(asset);
        const Point scaledDims = {(float)img.width * scale, (float)img.height * scale};

        Rectangle src{0, 0, static_cast<float>(textureSize), static_cast<float>(textureSize)};
        Rectangle dst{0, 0, std::floor(src.width * scale), std::floor(src.height * scale)};

        if (scaledDims.x >= MAGIQUE_TEXTURE_ATLAS_SIZE || scaledDims.y >= MAGIQUE_TEXTURE_ATLAS_SIZE)
        {
            LOG_ERROR("Cannot load image as TilSheet: Too big!");
            return;
        }

        Image texImage = GenImageColor(MAGIQUE_TEXTURE_ATLAS_SIZE, MAGIQUE_TEXTURE_ATLAS_SIZE, BLANK);

        texPerRow = static_cast<uint16_t>(texImage.width / static_cast<int>(dst.width));
        texSize = static_cast<int16_t>(dst.width);

        // TODO Can be optimized a lot by manually doing the image resizing and reusing a buffer
        while (src.y < img.height)
        {
            if (dst.y >= texImage.height)
            {
                LOG_ERROR("TileSheet doesnt fit into a single atlas! Skipping: %s", asset.path);
                break;
            }
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
        }

        const auto tex = LoadTextureFromImage(texImage);
        if (tex.id == 0)
        {
            LOG_ERROR("Failed to load TileSheet to GPU: %s", asset.getPath());
        }
        textureID = static_cast<uint16_t>(tex.id);

        UnloadImage(texImage);
        UnloadImage(img);
    }

    TextureRegion TileSheet::getRegion(const uint16_t tileNum) const
    {
        if (tileNum == 0) [[unlikely]]
        {
            return {static_cast<uint16_t>(MAGIQUE_TEXTURE_ATLAS_SIZE - texSize),
                    static_cast<uint16_t>(MAGIQUE_TEXTURE_ATLAS_SIZE - texSize), texSize, texSize, textureID};
        }

        const int row = (tileNum - 1) / texPerRow;
        const int colum = tileNum - 1 - row * texPerRow;
        return {static_cast<uint16_t>(colum * texSize), static_cast<uint16_t>(row * texSize), texSize, texSize,
                textureID};
    }

    Vector2 TileSheet::getOffset(const uint16_t tileNum) const
    {
        if (tileNum == 0) [[unlikely]]
        {
            return {static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE - texSize),
                    static_cast<float>(MAGIQUE_TEXTURE_ATLAS_SIZE - texSize)};
        }
        const int row = (tileNum - 1) / texPerRow;
        const int colum = tileNum - 1 - row * texPerRow;
        return {static_cast<float>(colum * texSize), static_cast<float>(row * texSize)};
    }

    float TileSheet::getTextureSize() const { return texSize; }

    unsigned int TileSheet::getTextureID() const { return textureID; }

} // namespace magique