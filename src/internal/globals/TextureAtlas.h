#ifndef TEXTUREATLAS_H
#define TEXTUREATLAS_H

#include <cxstructs/StackVector.h>

#include <magique/util/Logging.h>

namespace magique
{
    // Uses naive 'scheduling' - if a sequence cant be deterministically described we skip to the next row
    // So if a spritesheet doesnt fit in the current row we just skip it and put it in the next wasting the space
    // Also uses lazy initialization - only loads image and texture if actually used
    struct TextureAtlas final
    {
        bool initialized = false;
        uint16_t id = 0;                           // Texture id
        int width = MAGIQUE_TEXTURE_ATLAS_WIDTH;   // Total width
        int height = MAGIQUE_TEXTURE_ATLAS_HEIGHT; // Total height
        int posX = 0;                              // Current offset from the top left
        int posY = 0;                              // Current offset from the top let
        int currentStepHeight = 0;                 // Highest height of a texture in current row
        void* imageData = nullptr;                 // Save memory by only saving data ptr

        TextureRegion addTexture(const Image& image, const int tarW, const int tarH)
        {
            initialize();
            TextureRegion region = {0};
            if (!checkStep(tarW, tarH))
                return region;

            region.width = static_cast<uint16_t>(tarW);
            region.height = static_cast<uint16_t>(tarH);
            region.offX = static_cast<uint16_t>(posX);
            region.offY = static_cast<uint16_t>(posY);
            region.id = id;

            Image atlasImage = getImg();
            // Add the image
            ImageDraw(&atlasImage, image, {0, 0, (float)image.width, (float)image.height},
                      {(float)posX, (float)posY, (float)tarW, (float)tarH}, WHITE);

            posX += tarW;

            UnloadImage(image);

            return region;
        }

        SpriteSheet addSpritesheet(const Image& image, const int frames, const int tarW, const int tarH, int offX,
                                   int offY)
        {
            initialize();
            SpriteSheet sheet = {0};
            const int totalWidth = frames * tarW;

            if (!checkStep(totalWidth, tarH))
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
        bool checkStep(const int texWidth, const int texHeight)
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

    namespace global
    {
        inline cxstructs::StackVector<TextureAtlas, ENTITIES_2 + 1> TEXTURE_ATLASES;
    }
} // namespace magique
#endif //TEXTUREATLAS_H