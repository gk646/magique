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
            initialize();
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

        SpriteSheet addSpriteSheet(const Image& img, const int frames, const int tarW, const int tarH, int offX,
                                   int offY)
        {
            initialize();
            SpriteSheet sheet = {};
            const int totalWidth = frames * tarW;
            if (!isFullAndSkipToNextRowIfNeeded(totalWidth, tarH))
                return sheet;

            sheet.width = static_cast<int16_t>(tarW);
            sheet.height = static_cast<int16_t>(tarH);
            sheet.offX = static_cast<uint16_t>(posX);
            sheet.offY = static_cast<uint16_t>(posY);
            sheet.id = id;
            sheet.frames = static_cast<uint16_t>(frames);
            Image atlasImage = getImg();

            if (offX == 0) // Load whole image
            {
                while (true)
                {
                    const Rectangle src = {(float)offX, (float)offY, (float)tarW, (float)tarH};
                    const Rectangle dest = {(float)posX, (float)posY, (float)tarW, (float)tarH};
                    ImageDraw(&atlasImage, img, src, dest, WHITE);
                    offX += tarW;
                    posX += tarW;
                    if (offX >= img.width)
                    {
                        offX = 0;
                        offY += tarH;
                        if (offY >= img.height)
                            break;
                    }
                }
            }
            else // Load part of image
            {
                for (int i = 0; i < frames; ++i)
                {
                    const Rectangle src = {(float)offX, (float)offY, (float)tarW, (float)tarH};
                    const Rectangle dest = {(float)posX, (float)posY, (float)tarW, (float)tarH};
                    ImageDraw(&atlasImage, img, src, dest, WHITE);
                    posX += tarW; // We check atlas line upfront - only support sheet in continuous line
                    offX += tarW;
                    if (offX > img.width)
                    {
                        offX = 0;
                        offY += tarH;
                    }
                }
            }
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
        void initialize()
        {
            if (initialized)
                return;
            initialized = true;

            // Always PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 and 1 mipmap
            const auto img = GenImageColor(width, height, BLANK);
            imageData = img.data;

            const auto tex = LoadTextureFromImage(getImg());
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
        inline AtlasData ATLAS_DATA;
    }
} // namespace magique
#endif //TEXTURE_ATLAS_H