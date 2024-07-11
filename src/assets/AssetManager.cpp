#include <cmath>

#include <magique/assets/AssetManager.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>
#include <magique/internal/Macros.h>
#include <magique/core/Types.h>

#include <raylib/raylib.h>

#include "core/globals/TextureAtlas.h"
#include "core/globals/AssetManager.h"
#include "assets/LoadWrappers.h"

namespace magique
{
    bool ImageCheck(Image& img, const Asset& asset, const AtlasID at)
    {
        if (at > ENTITIES_2)
        {
            LOG_ERROR("Trying to load texture into invalid atlas");
            return false;
        }
        img = internal::LoadImage(asset);
        return true;
    }

    handle RegisterSpritesheet(const Asset& asset, const int width, const int height, const AtlasID at, float scale)
    {
        Image image;
        if (!ImageCheck(image, asset, at))
            return handle::null;

        M_ASSERT(image.width >= width && image.height >= height, "Image is smaller than a single frame");

        const int tarWidth = static_cast<int>(static_cast<float>(width) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(height) * scale);

        const int frames = image.width / width * (image.height / height);
        // All images will be layout out horizontally for fast direct access without calculations
        if (frames * tarWidth > MAGIQUE_TEXTURE_ATLAS_WIDTH)
        {
            LOG_WARNING("Spritesheet width would exceed texture atlas width! Skipping: %s", asset.path);
            UnloadImage(image);
            return handle::null;
        }

        auto& atlas = global::TEXTURE_ATLASES[at];
        const SpriteSheet sheet = atlas.addSpritesheet(image, frames, tarWidth, tarHeight, 0, 0);

        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSpritesheetEx(const Asset& asset, const int width, const int height, const int frames,
                                 const int offX, const int offY, const AtlasID at, const float scale)
    {
        Image image;
        if (!ImageCheck(image, asset, at))
            return handle::null;

        M_ASSERT(image.width >= width && image.height >= height, "Image is smaller than a single frame");
        M_ASSERT(offX < image.width && offY < image.height, "Offset is outside image bounds");

        const int tarWidth = static_cast<int>(static_cast<float>(width) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(height) * scale);

        if (frames * tarWidth > MAGIQUE_TEXTURE_ATLAS_WIDTH)
        {
            LOG_WARNING("Spritesheet width would exceed texture atlas width! Skipping: %s", asset.path);
            UnloadImage(image);
            return handle::null;
        }

        auto& atlas = global::TEXTURE_ATLASES[at];
        const auto sheet = atlas.addSpritesheet(image, frames, tarWidth, tarHeight, offX, offY);

        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSpritesheetVec(const std::vector<const Asset*>& assets, AtlasID at, float scale)
    {
        Image image;
        if (!ImageCheck(image, *assets[0], at))
            return handle::null;

        const int width = image.width;
        const int height = image.height;

        const float tarWidth = std::floor(static_cast<float>(width) * scale);
        const float tarHeight = std::floor(static_cast<float>(height) * scale);

        if (assets.size() * tarWidth > MAGIQUE_TEXTURE_ATLAS_WIDTH)
        {
            LOG_WARNING("Spritesheet width would exceed texture atlas width! Skipping: %s", assets[0]->path);
            UnloadImage(image);
            return handle::null;
        }

        const Rectangle source = {0, 0, (float)width, (float)height};
        Image singleImage = GenImageColor(assets.size() * width, height, BLANK);
        ImageDraw(&singleImage, image, source, {0, 0, (float)width, (float)height}, WHITE);

        auto offX = static_cast<float>(width);
        for (int i = 1; i < assets.size(); ++i)
        {
            const auto& a = *assets[i];
            Image newImg;
            if (!ImageCheck(newImg, a, at))
            {
                UnloadImage(newImg);
                UnloadImage(singleImage);
                return handle::null;
            }
            if (image.width != width || image.height != height)
            {
                LOG_WARNING("Image is not the same size as others: %s", a.path);
                UnloadImage(newImg);
                UnloadImage(singleImage);
                return handle::null;
            }
            ImageDraw(&singleImage, newImg, source, {offX, 0, (float)width, (float)height}, WHITE);
            offX += static_cast<float>(width);
            UnloadImage(newImg);
        }

        auto& atlas = global::TEXTURE_ATLASES[at];
        const auto sheet = atlas.addSpritesheet(singleImage, assets.size(), tarWidth, tarHeight, 0, 0);

        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSound(const Asset& asset)
    {
        const auto ext = asset.getExtension();
        if (ext == nullptr)
        {
            LOG_ERROR("Asset file type is not a sound file!: %s", ext);
        }
        const Wave wave = LoadWaveFromMemory(ext, (unsigned char*)asset.data, asset.size);
        Sound sound = LoadSoundFromWave(wave);
        const auto handle = global::ASSET_MANAGER.addResource<Sound>(sound);
        UnloadWave(wave);
        return handle;
    }

    handle RegisterTexture(const Asset& asset, const AtlasID at, float scale)
    {
        Image image;
        if (!ImageCheck(image, asset, at))
            return handle::null;

        auto& atlas = global::TEXTURE_ATLASES[at];

        const auto region = atlas.addTexture(image, (int)(scale * image.width), (int)(scale * image.height));

        return global::ASSET_MANAGER.addResource(region);
    }

    handle RegisterTileMap(const Asset& asset)
    {
        auto tileMap = TileMap(asset);
        return global::ASSET_MANAGER.addResource(std::move(tileMap));
    }

    handle RegisterTileSet(const Asset& asset) { return handle::null; }

    handle RegisterTileSheet(const Asset& asset, int size, float scale)
    {
        const auto sheet = TileSheet(asset, size, scale);

        if (sheet.textureID == 0)
        {
            return handle::null;
        }

        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterTileSheet(std::vector<const Asset*>& assets, int size, float scale) { return handle::null; }

    //----------------- GET -----------------//

    Sound& GetSound(const handle handle) { return global::ASSET_MANAGER.getResource<Sound>(handle); }

    TextureRegion GetTexture(const handle handle) { return global::ASSET_MANAGER.getResource<TextureRegion>(handle); }

    SpriteSheet GetSpriteSheet(const handle handle) { return global::ASSET_MANAGER.getResource<SpriteSheet>(handle); }

    TileMap& GetTileMap(const handle handle) { return global::ASSET_MANAGER.getResource<TileMap>(handle); }

    TileSheet& GetTileSheet(const handle handle) { return global::ASSET_MANAGER.getResource<TileSheet>(handle); }
} // namespace magique