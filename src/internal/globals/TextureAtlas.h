// SPDX-License-Identifier: zlib-acknowledgement
#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <magique/util/Logging.h>

#include "internal/datastructures/VectorType.h"

namespace magique
{
    // Uses naive 'scheduling' - if a sequence doesn't fit into the same row we skip to the next row
    // So if a SpriteSheet doesn't fit in the current row we just skip it and put it in the next wasting the space
    // Also uses lazy initialization - only loads image and texture if actually used
    struct TextureAtlas final
    {
        bool initialized = false;
        uint16_t id = 0;                         // Texture id
        int width = MAGIQUE_TEXTURE_ATLAS_SIZE;  // Total width
        int height = MAGIQUE_TEXTURE_ATLAS_SIZE; // Total height
        int posX = 0;                            // Current offset from the top left
        int posY = 0;                            // Current offset from the top let
        int currentStepHeight = 0;               // Highest height of a texture in current row
        void* imageData = nullptr;               // Save memory by only saving data ptr

        TextureRegion addTexture(const Image& image, const int tarW, const int tarH)
        {
            lazyInit();
            TextureRegion region = {};
            if (!isFullAndSkipToNextRowIfNeeded(tarW, tarH))
                return region;

            region.width = static_cast<int16_t>(tarW);
            region.height = static_cast<int16_t>(tarH);
            region.offX = static_cast<uint16_t>(posX);
            region.offY = static_cast<uint16_t>(posY);
            region.id = id;

            Image atlasImage = getImg();
            // Add the image
            const Rectangle src = {0, 0, static_cast<float>(image.width), static_cast<float>(image.height)};
            const Rectangle dest = {static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(tarW),
                                    static_cast<float>(tarH)};
            ImageDraw(&atlasImage, image, src, dest, WHITE);
            posX += tarW;
            UnloadImage(image);
            return region;
        }

        SpriteSheet addSpriteSheet(Image& img, const int srcW, const int srcH, const float scale)
        {
            lazyInit(); // Only load a texture if atlas is actually used
            // Cache
            const int tarW = static_cast<int>(static_cast<float>(srcW) * scale);
            const int tarH = static_cast<int>(static_cast<float>(srcH) * scale);
            const int frames = img.width / srcW * (img.height / srcH);
            const int totalWidth = frames * tarW;

            SpriteSheet sheet = {};
            if (!isFullAndSkipToNextRowIfNeeded(totalWidth, tarH))
                return sheet;

            // Assign sheet
            assignSheet(sheet, tarW, tarH, frames);

            Image atlasImage = getImg(); // The current image of the atlas in the RAM
            Rectangle src = {0, 0, static_cast<float>(srcW), static_cast<float>(srcH)};
            Rectangle dest = {static_cast<float>(posX), static_cast<float>(posY), (float)tarW, (float)tarH};

            while (true)
            {
                ImageDraw(&atlasImage, img, src, dest, WHITE);
                src.x += static_cast<float>(srcW);
                dest.x += static_cast<float>(tarW);
                if (src.x >= static_cast<float>(img.width))
                {
                    src.x = 0.0F;
                    src.y += static_cast<float>(tarH);
                    if (src.y >= static_cast<float>(img.height))
                        break; // We reached the end by going row by row
                }
            }
            posX = static_cast<int>(dest.x);
            UnloadImage(img);
            return sheet;
        }

        SpriteSheet addSpriteSheetEx(Image& img, const int srcW, const int srcH, const float scale, const int frames,
                                     const int offX, const int offY)
        {
            lazyInit(); // Only load a texture if atlas is actually used

            // Cache
            const int tarW = static_cast<int>(static_cast<float>(srcW) * scale);
            const int tarH = static_cast<int>(static_cast<float>(srcH) * scale);
            const int totalWidth = frames * tarW;

            SpriteSheet sheet = {};
            if (!isFullAndSkipToNextRowIfNeeded(totalWidth, tarH))
                return sheet;

            // Assign sheet
            assignSheet(sheet, tarW, tarH, frames);

            Image atlasImage = getImg(); // The current image of the atlas in the RAM
            Rectangle src = {static_cast<float>(offX), static_cast<float>(offY), static_cast<float>(srcW),
                             static_cast<float>(srcH)};
            Rectangle dest = {static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(tarW),
                              static_cast<float>(tarH)};

            for (int i = 0; i < frames; ++i)
            {
                ImageDraw(&atlasImage, img, src, dest, WHITE);
                src.x += static_cast<float>(srcW);
                dest.x += static_cast<float>(tarW);
                if (src.x >= static_cast<float>(img.width))
                {
                    src.x = 0.0F;
                    src.y += static_cast<float>(tarH);
                    if (src.y >= static_cast<float>(img.height))
                        break; // We reached the end by going row by row
                }
            }

            posX = static_cast<int>(dest.x);
            UnloadImage(img);
            return sheet;
        }

        void loadToGPU() const
        {
            if (!initialized)
                return;
            // Always same format as image
            UpdateTexture({id, width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8}, imageData);
            UnloadImage(getImg());
        }

    private:
        void assignSheet(SpriteSheet& sheet, const int tarW, const int tarH, const int frames) const
        {
            sheet.width = static_cast<int16_t>(tarW);
            sheet.height = static_cast<int16_t>(tarH);
            sheet.offX = static_cast<uint16_t>(posX);
            sheet.offY = static_cast<uint16_t>(posY);
            sheet.id = id;
            sheet.frames = static_cast<uint16_t>(frames);
        }

        void lazyInit()
        {
            if (initialized)
                return;
            initialized = true;

            // Always PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 and 1 mipmap
            const auto img = GenImageColor(width, height, BLANK);
            imageData = img.data;

            const auto tex = LoadTextureFromImage(img);
            if (tex.id == 0)
            {
                LOG_ERROR("Failed to load texture atlas texture! No textures will work");
                UnloadImage(getImg());
            }

            id = static_cast<uint16_t>(tex.id);
        }

        [[nodiscard]] Image getImg() const
        {
            Image img;
            img.data = imageData;
            img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            img.mipmaps = 1;
            img.width = width;
            img.height = height;
            return img;
        }

        bool isFullAndSkipToNextRowIfNeeded(const int texWidth, const int texHeight)
        {
            if (posX + texWidth > width)
            {
                posY += currentStepHeight;
                posX = 0;
                currentStepHeight = texHeight;
                if (posY >= height)
                {
                    LOG_ERROR("Texture atlas is full!");
                    return false;
                }
            }
            if (texHeight > currentStepHeight) // Keep track of the highest image
                currentStepHeight = texHeight;
            return true;
        }
    };

    struct AtlasData final
    {
        vector<TextureAtlas> atlases;

        TextureAtlas& getAtlas(AtlasID type)
        {
            const auto atlasNum = static_cast<int>(type);
            if (atlasNum >= atlases.size())
                atlases.resize(atlasNum + 1);
            return atlases[atlasNum];
        }

        void loadToGPU()
        {
            for (const auto& atlas : atlases)
            {
                atlas.loadToGPU();
            }
        }
    };

    namespace global
    {
        inline AtlasData ATLAS_DATA{};
    }
} // namespace magique
#endif //TEXTURE_ATLAS_H