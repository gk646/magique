#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include <magique/assets/AssetManager.h>
#include <magique/util/Defines.h>
#include <magique/util/Logging.h>
#include <magique/core/Types.h>

#include <cxstructs/StackVector.h>

namespace magique
{
    // Uses naive 'scheduling' - if a sequence cant be deterministically described we skip to the next row
    // So if a spritesheet doesnt fit in the current row we just skip it and put it in the next wasting the space
    struct TextureAtlas final
    {
        int width = MAGIQUE_TEXTURE_ATLAS_WIDTH;   // Total width
        int height = MAGIQUE_TEXTURE_ATLAS_HEIGHT; // Total height
        int posX = 0;                              // Current offset from the top left
        int posY = 0;                              // Current offset from the top let
        unsigned int id = 0;                       // Texture id
        int currentStepHeight = 0;                 // Highest height of a texture in current row
        void* imageData = nullptr;                 // Save memory by only saving data ptr

        explicit TextureAtlas(Color color) // Just passed so its not auto constructed anywhere
        {
            // Always PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 and 1 mipmap
            const auto img = GenImageColor(width, height, color);
            imageData = img.data;
            const auto tex = LoadTextureFromImage(img);
            if (tex.id == 0)
            {
                LOG_ERROR("Failed to load texture atlas texture! No textures will work");
                UnloadImage(img);
            }
            id = tex.id;
        }

        TextureRegion addTexture(const Image& image)
        {
            TextureRegion region = {0};
            if (!checkStep(image.width, image.height))
                return region;

            region.width = static_cast<uint16_t>(image.width);
            region.height = static_cast<uint16_t>(image.height);
            region.offX = static_cast<uint16_t>(posX);
            region.offY = static_cast<uint16_t>(posY);
            region.id = static_cast<uint16_t>(id);


            Image atlasImage = getImg();
            // Add the image
            ImageDraw(&atlasImage, image, {0, 0, (float)image.width, (float)image.height},
                      {(float)posX, (float)posY, (float)image.width, (float)image.height}, WHITE);

            posX += image.width;

            UnloadImage(image);

            return region;
        }

        SpriteSheet addSpritesheet(const Image& image, const int frames, int tarW, int tarH, int offX, int offY)
        {
            SpriteSheet sheet = {0};
            const int totalWidth = frames * tarW;

            if (!checkStep(totalWidth, tarH))
                return sheet;

            sheet.width = static_cast<uint16_t>(tarW);
            sheet.height = static_cast<uint16_t>(tarH);
            sheet.offX = static_cast<uint16_t>(posX);
            sheet.offY = static_cast<uint16_t>(posY);
            sheet.id = static_cast<uint16_t>(id);
            sheet.frames = frames;

            Image atlasImage = getImg();

            if (offX == 0) // Load whole image
            {
                while (true)
                {
                    ImageDraw(&atlasImage, image, {(float)offX, (float)offY, (float)tarW, (float)tarH},
                              {(float)posX, (float)posY, (float)tarW, (float)tarH}, WHITE);
                    offX += tarW;
                    posX += tarW;
                    if (offX >= image.width)
                    {
                        offX = 0;
                        offY += tarH;
                        if (offY >= image.height)
                            break;
                    }
                }
            }
            else // Load part of image
            {
                for (int i = 0; i < frames; ++i)
                {
                    ImageDraw(&atlasImage, image, {(float)offX, (float)offY, (float)tarW, (float)tarH},
                              {(float)posX, (float)posY, (float)tarW, (float)tarH}, WHITE);
                    posX += tarW; // We check atlas line upfront - only support sheet in contious line
                    offX += tarW;
                    if (offX > image.width)
                    {
                        offX = 0;
                        offY += tarH;
                    }
                }
            }

            UnloadImage(image);

            return sheet;
        }

        bool checkStep(int texWidth, int texHeight)
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

        void loadToGPU() const
        {
            // Always same format as image
            UpdateTexture({id, width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8}, imageData);
            UnloadImage(getImg());
        }
    };

    namespace global
    {
        inline cxstructs::StackVector<TextureAtlas, ENTITIES_2+1 > TEXTURE_ATLASES;

    }
} // namespace magique
#endif //TEXTUREATLAS_H