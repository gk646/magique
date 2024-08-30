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
#include "assets/headers/LoadWrappers.h"

namespace magique
{
    handle RegisterTexture(const Asset& asset, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        const Image image = LoadImage(asset);
        const int tarWidth = static_cast<int>(static_cast<float>(image.width) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(image.height) * scale);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto region = atlas.addTexture(image, tarWidth, tarHeight);
        return global::ASSET_MANAGER.addResource(region);
    }

    handle RegisterSpriteSheet(const Asset& asset, const int w, const int h, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        const Image image = LoadImage(asset);
        MAGIQUE_ASSERT(image.width >= w && image.height >= h, "Image is smaller than a single frame");

        const int tarWidth = static_cast<int>(static_cast<float>(w) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(h) * scale);
        const int frames = image.width / w * (image.height / h);

        // All images will be layout out horizontally for fast direct access without calculations
        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(frames * tarWidth);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const SpriteSheet sheet = atlas.addSpriteSheet(image, frames, tarWidth, tarHeight, 0, 0);
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSpriteSheetEx(const Asset& asset, const int w, const int h, const int frames, const int offX,
                                 const int offY, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        const Image image = LoadImage(asset);
        MAGIQUE_ASSERT(image.width >= w && image.height >= h, "Image is smaller than a single frame");
        MAGIQUE_ASSERT(offX < image.width && offY < image.height, "Offset is outside image bounds");
        const int tarWidth = static_cast<int>(static_cast<float>(w) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(h) * scale);

        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(frames * tarWidth);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto sheet = atlas.addSpriteSheet(image, frames, tarWidth, tarHeight, offX, offY);
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSpriteSheetVec(const std::vector<const Asset*>& assets, const AtlasID at, const float scale)
    {
        for (const auto asset : assets)
        {
            ASSET_CHECK(*asset);
            ASSET_IS_SUPPORTED_IMAGE_TYPE((*asset));
        }
        const Image image = LoadImage(*assets[0]);
        const int width = image.width;
        const int height = image.height;
        const auto tarWidth = static_cast<int>(static_cast<float>(width) * scale);
        const auto tarHeight = static_cast<int>(static_cast<float>(height) * scale);

        const auto& asset = *assets[0]; // For the macro
        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(assets.size() * tarWidth);

        const Rectangle source = {0, 0, static_cast<float>(width), static_cast<float>(height)};
        Image singleImage = GenImageColor(static_cast<int>(assets.size()) * width, height, BLANK);
        ImageDraw(&singleImage, image, source, {0, 0, static_cast<float>(width), static_cast<float>(height)}, WHITE);

        auto offX = static_cast<float>(width);
        for (int i = 1; i < assets.size(); ++i)
        {
            const auto& a = *assets[i];
            const Image newImg = LoadImage(a);
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

        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto sheet = atlas.addSpriteSheet(singleImage, (int)assets.size(), tarWidth, tarHeight, 0, 0);
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSound(const Asset& asset)
    {
        ASSET_CHECK(asset);
        const auto ext = asset.getExtension();
        if (ext == nullptr)
        {
            LOG_ERROR("Asset file type is not a sound file!: %s", ext);
        }
        const Wave wave = LoadWaveFromMemory(ext, (unsigned char*)asset.data, asset.size);
        const Sound sound = LoadSoundFromWave(wave);
        const auto handle = global::ASSET_MANAGER.addResource<Sound>(sound);
        UnloadWave(wave);
        return handle;
    }

    handle RegisterMusic(const Asset& asset)
    {
        const auto ext = asset.getExtension();
        if (ext == nullptr)
        {
            LOG_ERROR("Asset file type is not a sound file!: %s", ext);
        }
        // Duplicate data as stream isnt copied
        // This is leaked but until the end of the programm / still should make a manager for it
        auto* newData = new char[asset.size];
        std::memcpy(newData, asset.data, asset.size);
        const auto music = LoadMusicStreamFromMemory(ext, (unsigned char*)newData, asset.size);
        const auto handle = global::ASSET_MANAGER.addResource<Music>(music);
        return handle;
    }

    handle RegisterPlaylist(const std::vector<const Asset*>& assets) { return handle::null; }

    handle RegisterTileMap(const Asset& asset)
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

    handle RegisterTileMapGen(const std::vector<std::vector<std::vector<uint16_t>>>& layerData) { return handle::null; }

    handle RegisterTileSet(const Asset& asset)
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

    handle RegisterTileSheet(const Asset& asset, const int tileSize, const float scale)
    {
        ASSET_CHECK(asset);
        const auto sheet = TileSheet(asset, tileSize, scale);
        if (strcmp(asset.getExtension(), ".png") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileSheet: %s | Supported: .png", asset.getFileName(true));
            return handle::null;
        }
        if (sheet.textureID == 0)
        {
            return handle::null;
        }
        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterTileSheet(const std::vector<const Asset*>& assets, const int tileSize, const float scale)
    {
        for (const auto asset : assets)
        {
            ASSET_CHECK(*asset);
            if (strcmp(asset->getExtension(), ".png") != 0)
            {
                LOG_WARNING("Invalid extensions for a TileSheet: %s | Supported: .png", asset->getFileName(true));
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

} // namespace magique