// SPDX-License-Identifier: zlib-acknowledgement
#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <magique/util/Logging.h>

namespace magique
{
    // Uses naive 'scheduling' - if a sequence doesn't fit into the same row we skip to the next row
    // So if a SpriteSheet doesn't fit in the current row we just skip it and put it in the next wasting the space
    // Also uses lazy initialization - only loads image and texture if actually used
    struct TextureAtlas final
    {
        AtlasID atlas;
        int gap = 1;
        bool initialized = false;
        uint16_t id = 0;                            // Texture id
        Point maxDims = {MAGIQUE_MAX_TEXTURE_SIZE}; // Max dimensions
        Point cursor{};                             // Current offset
        int currentStepHeight = 0;                  // Highest height of a texture in current row
        void* imageData = nullptr;                  // Save memory by only saving data ptr

        TextureRegion addTexture(const Image& image, Point targetDims)
        {
            lazyInit();
            TextureRegion region{};
            if (!isFullAndSkipToNextRowIfNeeded(targetDims))
            {
                return region;
            }

            region.width = static_cast<int16_t>(targetDims.x);
            region.height = static_cast<int16_t>(targetDims.y);
            region.offX = static_cast<uint16_t>(cursor.x);
            region.offY = static_cast<uint16_t>(cursor.y);
            region.id = id;

            Image atlasImage = getImg();
            // Add the image
            const Rect src = {{}, {static_cast<float>(image.width), static_cast<float>(image.height)}};
            const Rect dest = {cursor, targetDims};
            ImageDraw(&atlasImage, image, src, dest, WHITE);
            cursor.x += targetDims.x + gap;
            UnloadImage(image);
            return region;
        }

        SpriteSheet addSpriteSheet(Image& img, Point srcDims, float scale)
        {
            const int frames = img.width / srcDims.x * (img.height / srcDims.y);
            return addSpriteSheetEx(img, srcDims, scale, frames, {});
        }

        SpriteSheet addSpriteSheetEx(const Image& img, Point srcDims, const float scale, const int frames, Point offset)
        {
            lazyInit(); // Only load a texture if atlas is actually used

            // Cache
            Point targetDims = srcDims * scale;
            const auto totalWidth = frames * targetDims.x;

            SpriteSheet sheet{};
            if (!isFullAndSkipToNextRowIfNeeded({totalWidth, targetDims.y}))
                return sheet;

            // Assign sheet
            assignSheet(sheet, targetDims, frames);

            Image atlasImage = getImg(); // The current image of the atlas in the RAM
            Rect src = {offset, srcDims};
            Rect dest = {cursor, targetDims};

            for (int i = 0; i < frames; ++i)
            {
                ImageDraw(&atlasImage, img, src, dest, WHITE);
                src.x += srcDims.x;
                dest.x += targetDims.x;
                if (src.x >= static_cast<float>(img.width))
                {
                    src.x = 0.0F;
                    src.y += srcDims.y;
                    if (src.y >= static_cast<float>(img.height))
                        break; // We reached the end by going row by row
                }
            }

            cursor.x = static_cast<int>(dest.x) + gap;
            UnloadImage(img);
            return sheet;
        }

        void loadToGPU() const
        {
            if (!initialized)
                return;
            // Always same format as image
            Texture2D tex = {id, (int)maxDims.x, (int)maxDims.y, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
            UpdateTexture(tex, imageData);
            UnloadImage(getImg());
        }

    private:
        void assignSheet(SpriteSheet& sheet, Point dims, const int frames) const
        {
            sheet.region.width = static_cast<int16_t>(dims.x);
            sheet.region.height = static_cast<int16_t>(dims.y);
            sheet.region.offX = static_cast<uint16_t>(cursor.x);
            sheet.region.offY = static_cast<uint16_t>(cursor.y);
            sheet.region.id = id;
            sheet.frames = static_cast<uint16_t>(frames);
        }

        void lazyInit()
        {
            if (initialized)
            {
                return;
            }
            initialized = true;

            // Always PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 and 1 mipmap
            const auto img = GenImageColor(maxDims.x, maxDims.y, BLANK);
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
            img.width = maxDims.x;
            img.height = maxDims.y;
            return img;
        }

        bool isFullAndSkipToNextRowIfNeeded(Point dims)
        {
            if (cursor.x + dims.x > maxDims.x)
            {
                cursor.y += currentStepHeight;
                cursor.x = 0;
                currentStepHeight = dims.y + gap;
                if (cursor.y >= maxDims.y)
                {
                    LOG_ERROR("TextureAtlas with AtlasID %d is full!", (int)atlas);
                    return false;
                }
            }
            if (dims.y > currentStepHeight) // Keep track of the highest image
                currentStepHeight = dims.y + gap;
            return true;
        }
    };

    struct AtlasData final
    {
        std::vector<TextureAtlas> atlases;

        TextureAtlas& getAtlas(AtlasID type)
        {
            const auto atlasNum = static_cast<int>(type);
            if (atlasNum >= (int)atlases.size())
                atlases.resize(atlasNum + 1);
            auto& atlas = atlases[atlasNum];
            atlas.atlas = type;
            return atlas;
        }

        void loadToGPU() const
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
#endif // TEXTURE_ATLAS_H
