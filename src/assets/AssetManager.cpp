// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>

#include <magique/assets/AssetManager.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>
#include <magique/assets/types/TileSet.h>
#include <magique/assets/types/Playlist.h>
#include <magique/assets/HandleRegistry.h>
#include <magique/internal/Macros.h>
#include <magique/core/Types.h>

#include "internal/globals/TextureAtlas.h"
#include "internal/globals/AssetManager.h"
#include "assets/headers/AssetUtil.h"

namespace magique
{
    handle RegisterTexture(const Texture& texture, AtlasID atlas)
    {
        MAGIQUE_ASSERT(texture.id != 0, "Trying to register an invalid texture");
        auto& texAtlas = global::ATLAS_DATA.getAtlas(atlas);
        const Image textImg = LoadImageFromTexture(texture);
        const auto region = texAtlas.addTexture(textImg, texture.width, texture.height);
        UnloadImage(textImg);
        return global::ASSET_MANAGER.addResource(region);
    }

    handle RegisterTexture(Asset asset, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        const Image image = LoadImage(asset);
        const int tarWidth = static_cast<int>(static_cast<float>(image.width) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(image.height) * scale);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto region = atlas.addTexture(image, tarWidth, tarHeight);
        return global::ASSET_MANAGER.addResource(region);
    }

    handle RegisterSpriteSheet(Asset asset, const int w, const int h, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        ASSET_IS_SUPPORTED_IMAGE_TYPE(asset);
        Image image = LoadImage(asset);

        // Checks
        MAGIQUE_ASSERT(image.width >= w && image.height >= h, "Image is smaller than a single frame");
        ASSET_CHECK_IMAGE_DIVISIBILITY(image, w, h);

        const int tarWidth = static_cast<int>(static_cast<float>(w) * scale);
        const int frames = image.width / w * (image.height / h);
        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(frames * tarWidth); // Check if sprite fits within one row

        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const SpriteSheet sheet = atlas.addSpriteSheet(image, w, h, scale);
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSpriteSheetEx(Asset asset, const int w, const int h, const int frames, const int offX, const int offY,
                                 const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        ASSET_IS_SUPPORTED_IMAGE_TYPE(asset);
        Image image = LoadImage(asset);

        // Checks
        MAGIQUE_ASSERT(image.width >= w && image.height >= h, "Image is smaller than a single frame");
        MAGIQUE_ASSERT(offX < image.width && offY < image.height, "Offset is outside image bounds");
        const int tarWidth = static_cast<int>(static_cast<float>(w) * scale);
        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(frames * tarWidth);

        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto sheet = atlas.addSpriteSheetEx(image, w, h, scale, frames, offX, offY);
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSpriteSheetVec(const std::vector<Asset>& assets, const AtlasID at, const float scale)
    {
        if (assets.empty()) // Check empty
        {
            LOG_WARNING("Passed empty asset vector to RegisterSpriteSheetVec()");
            return handle::null;
        }
        for (const auto& asset : assets) // Check valid and supported image type
        {
            ASSET_CHECK(asset);
            ASSET_IS_SUPPORTED_IMAGE_TYPE(asset);
        }

        // Load first image to check
        Image image = LoadImage(assets[0]);
        const int width = image.width;
        const int height = image.height;

        // Check if fits within one atlas row
        const auto tarW = static_cast<float>(width) * scale;
        const auto& asset = assets[0]; // For the macro
        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(assets.size() * tarW);

        // Make a single image out of the vector - always use the full source image
        const Rectangle source = {0, 0, static_cast<float>(width), static_cast<float>(height)};
        Rectangle dest = {0, 0, source.width, source.height}; // copy 1:1 they are scaled later
        Image singleImage = GenImageColor(static_cast<int>(assets.size()) * width, height, BLANK);

        for (int i = 1; i < static_cast<int>(assets.size()); ++i)
        {
            ImageDraw(&singleImage, image, source, dest, WHITE);
            dest.x += static_cast<float>(width);
            UnloadImage(image);

            image = LoadImage(assets[i]);
            if (image.width != width || image.height != height)
            {
                LOG_WARNING("Image is not the same size as others: %s", asset.getFileName(true));
                UnloadImage(image);
                UnloadImage(singleImage);
                return handle::null;
            }
        }
        ImageDraw(&singleImage, image, source, dest, WHITE); // Draw the last picture
        UnloadImage(image);

        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto sheet = atlas.addSpriteSheet(singleImage, width, height, scale);
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSound(const Sound& sound)
    {
        if (!IsSoundValid(sound))
        {
            LOG_WARNING("Trying to register invalid sound");
            return handle::null;
        }
        const auto handle = global::ASSET_MANAGER.addResource<Sound>(sound);
        return handle;
    }

    handle RegisterSound(Asset asset)
    {
        ASSET_CHECK(asset);
        const auto ext = asset.getExtension();
        if (!IsSupportedSoundFormat(ext))
        {
            LOG_ERROR("Asset file type is not a sound file!: %s", ext);
            return handle::null;
        }
        const Wave wave = LoadWaveFromMemory(ext, (unsigned char*)asset.data, asset.size);
        const Sound sound = LoadSoundFromWave(wave);
        const auto handle = global::ASSET_MANAGER.addResource<Sound>(sound);
        UnloadWave(wave);
        return handle;
    }

    handle RegisterMusic(Asset asset)
    {
        ASSET_CHECK(asset);
        const auto ext = asset.getExtension();
        if (!IsSupportedSoundFormat(ext))
        {
            LOG_ERROR("Asset file type is not a sound file!: %s", ext);
            return handle::null;
        }
        // Duplicate data as stream isn't copied
        // This is leaked but until the end of the programm
        auto* newData = new char[asset.size];
        std::memcpy(newData, asset.data, asset.size);
        const auto music = LoadMusicStreamFromMemory(ext, (unsigned char*)newData, asset.size);
        const auto handle = global::ASSET_MANAGER.addResource<Music>(music);
        return handle;
    }

    handle RegisterPlaylist(const std::vector<Asset>& assets)
    {
        if (assets.empty()) // Check empty
        {
            LOG_WARNING("Passed empty asset vector to RegisterPlaylist()");
            return handle::null;
        }

        Playlist playlist{};
        playlist.tracks.reserve(assets.size() + 1);
        for (const auto& asset : assets)
        {
            ASSET_CHECK(asset);
            const auto* ext = asset.getExtension();
            if (!IsSupportedSoundFormat(ext))
            {
                LOG_ERROR("Asset file type is not a sound file!: %s", ext);
                return handle::null;
            }
            auto* newData = new char[asset.size];
            std::memcpy(newData, asset.data, asset.size);
            const auto music = LoadMusicStreamFromMemory(ext, (unsigned char*)newData, asset.size);
            playlist.tracks.emplace_back(music);
        }
        const auto handle = global::ASSET_MANAGER.addResource<Playlist>(std::move(playlist));
        return handle;
    }

    handle RegisterTileMap(Asset asset)
    {
        ASSET_CHECK(asset);
        if (strcmp(asset.getExtension(), ".tmx") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileMap: %s | Supported: .tmx", asset.getFileName(true));
            return handle::null;
        }
        auto tileMap = TileMap(asset);
        return global::ASSET_MANAGER.addResource(std::move(tileMap));
    }

    handle RegisterTileMap(const std::vector<std::vector<std::vector<uint16_t>>>& layerData)
    {
        LOG_FATAL("Not implemented");
        return handle::null;
    }

    handle RegisterTileSet(Asset asset)
    {
        ASSET_CHECK(asset);
        if (strcmp(asset.getExtension(), ".tsx") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileSet: %s | Supported: .tsx", asset.getFileName(true));
            return handle::null;
        }
        auto tileSet = TileSet(asset);
        return global::ASSET_MANAGER.addResource(std::move(tileSet));
    }

    handle RegisterTileSheet(Asset asset, const int tileSize, const float scale)
    {
        ASSET_CHECK(asset);
        const auto sheet = TileSheet(asset, tileSize, scale);
        if (strcmp(asset.getExtension(), ".png") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileSheet: %s | Supported: .png", asset.getFileName(true));
            return handle::null;
        }
        if (sheet.getTextureID() == 0)
        {
            return handle::null;
        }
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterTileSheet(const std::vector<Asset>& assets, const int tileSize, const float scale)
    {
        for (const auto asset : assets)
        {
            ASSET_CHECK(asset);
            if (strcmp(asset.getExtension(), ".png") != 0)
            {
                LOG_WARNING("Invalid extensions for a TileSheet: %s | Supported: .png", asset.getFileName(true));
                return handle::null;
            }
        }
        const auto sheet = TileSheet(assets, tileSize, scale);
        if (sheet.textureID == 0)
        {
            return handle::null;
        }
        return global::ASSET_MANAGER.addResource(sheet);
    }

    //----------------- GET -----------------//

    Sound& GetSound(const handle handle) { return global::ASSET_MANAGER.getResource<Sound>(handle); }

    TextureRegion GetTexture(const handle handle) { return global::ASSET_MANAGER.getResource<TextureRegion>(handle); }

    SpriteSheet GetSpriteSheet(const handle handle) { return global::ASSET_MANAGER.getResource<SpriteSheet>(handle); }

    TileMap& GetTileMap(const handle handle) { return global::ASSET_MANAGER.getResource<TileMap>(handle); }

    TileSheet& GetTileSheet(const handle handle) { return global::ASSET_MANAGER.getResource<TileSheet>(handle); }

    Music& GetMusic(const handle handle) { return global::ASSET_MANAGER.getResource<Music>(handle); }

    Playlist& GetPlaylist(const handle handle) { return global::ASSET_MANAGER.getResource<Playlist>(handle); }


    //----------------- DIRECT GET -----------------//

    TextureRegion GetTexture(const HandleID id)
    {
        return global::ASSET_MANAGER.getResource<TextureRegion>(GetHandle(id));
    }
    TextureRegion GetTexture(const uint32_t hash)
    {
        return global::ASSET_MANAGER.getResource<TextureRegion>(GetHandle(hash));
    }

    TileMap& GetTileMap(const HandleID id) { return global::ASSET_MANAGER.getResource<TileMap>(GetHandle(id)); }
    TileMap& GetTileMap(const uint32_t hash) { return global::ASSET_MANAGER.getResource<TileMap>(GetHandle(hash)); }

    TileSheet& GetTileSheet(const HandleID handle)
    {
        return global::ASSET_MANAGER.getResource<TileSheet>(GetHandle(handle));
    }
    TileSheet& GetTileSheet(const uint32_t hash)
    {
        return global::ASSET_MANAGER.getResource<TileSheet>(GetHandle(hash));
    }

    TileSet& GetTileSet(const HandleID handle) { return global::ASSET_MANAGER.getResource<TileSet>(GetHandle(handle)); }
    TileSet& GetTileSet(const uint32_t hash) { return global::ASSET_MANAGER.getResource<TileSet>(GetHandle(hash)); }

    Sound& GetSound(const HandleID handle) { return global::ASSET_MANAGER.getResource<Sound>(GetHandle(handle)); }
    Sound& GetSound(const uint32_t hash) { return global::ASSET_MANAGER.getResource<Sound>(GetHandle(hash)); }

} // namespace magique