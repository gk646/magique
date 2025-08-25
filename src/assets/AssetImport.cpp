// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>

#include <magique/assets/AssetImport.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>
#include <magique/assets/types/TileSet.h>
#include <magique/assets/types/Playlist.h>
#include <magique/internal/Macros.h>
#include <magique/core/Types.h>

#include "internal/globals/TextureAtlas.h"
#include "assets/headers/AssetUtil.h"
#include "magique/core/Animations.h"

#define CUTE_ASEPRITE_IMPLEMENTATION
#define CUTE_TILED_NO_EXTERNAL_TILESET_WARNING
#define CUTE_TILED_IMPLEMENTATION

#include "external/cute_asprite.h"
#include "external/cute_tiled.h"

namespace magique
{
    TextureRegion ImportTexture(const Texture& texture, AtlasID atlas)
    {
        MAGIQUE_ASSERT(texture.id != 0, "Trying to register an invalid texture");
        auto& texAtlas = global::ATLAS_DATA.getAtlas(atlas);
        const Image textImg = LoadImageFromTexture(texture);
        const auto region = texAtlas.addTexture(textImg, texture.width, texture.height);
        UnloadImage(textImg);
        return region;
    }

    TextureRegion ImportTexture(Asset asset, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        const Image image = LoadImage(asset);
        const int tarWidth = static_cast<int>(static_cast<float>(image.width) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(image.height) * scale);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto region = atlas.addTexture(image, tarWidth, tarHeight);
        return region;
    }

    SpriteSheet RegisterSpriteSheet(Asset asset, const int w, const int h, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        Image image = LoadImage(asset);
        // Checks
        MAGIQUE_ASSERT(image.width >= w && image.height >= h, "Image is smaller than a single frame");
        ASSET_CHECK_IMAGE_DIVISIBILITY(image, w, h);
        const int tarWidth = static_cast<int>(static_cast<float>(w) * scale);
        const int frames = image.width / w * (image.height / h);
        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(frames * tarWidth);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const SpriteSheet sheet = atlas.addSpriteSheet(image, w, h, scale);
        return sheet;
    }

    SpriteSheet RegisterSpriteSheetEx(Asset asset, const int w, const int h, const int frames, const int offX,
                                      const int offY, const AtlasID at, const float scale)
    {

        ASSET_CHECK(asset);
        Image image = LoadImage(asset);
        // Checks
        MAGIQUE_ASSERT(image.width >= w && image.height >= h, "Image is smaller than a single frame");
        MAGIQUE_ASSERT(offX < image.width && offY < image.height, "Offset is outside image bounds");
        const int tarWidth = static_cast<int>(static_cast<float>(w) * scale);
        ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(frames * tarWidth);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto sheet = atlas.addSpriteSheetEx(image, w, h, scale, frames, offX, offY);
        return sheet;
    }

    SpriteSheet ImportSpriteSheetVec(const std::vector<Asset>& assets, AtlasID atlas, float scale)
    {
        if (assets.empty())
        {
            LOG_WARNING("Cannot import sprite sheet from empty vector");
            return {};
        }

        std::vector<Image> images;
        images.reserve(assets.size() + 1);

        // Load first image to check dimensions
        ASSET_CHECK(assets[0]);
        Image firstImage = LoadImage(assets[0]);
        if (firstImage.data == nullptr)
        {
            LOG_WARNING("Failed to load first image: %s", assets[0].getFileName(true));
            return {};
        }

        const int width = firstImage.width;
        const int height = firstImage.height;
        images.push_back(firstImage);

        // Check if any single image would exceed atlas size
        if (width > MAGIQUE_TEXTURE_ATLAS_SIZE)
        {
            LOG_WARNING("SpriteSheet width would exceed texture atlas width! Skipping: %s", assets[0].getFileName(true));
            for (auto& img : images)
            {
                UnloadImage(img);
            }
            return {};
        }

        // Load remaining images
        for (size_t i = 1; i < assets.size(); ++i)
        {
            ASSET_CHECK(assets[i]);
            Image loadedImage = LoadImage(assets[i]);
            if (loadedImage.data == nullptr)
            {
                LOG_WARNING("Failed to load image: %s", assets[i].getFileName(true));
                for (auto& img : images)
                {
                    UnloadImage(img);
                }
                return {};
            }

            // Check dimensions match
            if (loadedImage.width != width || loadedImage.height != height)
            {
                LOG_WARNING("Image size mismatch: %s (expected %dx%d, got %dx%d)", assets[i].getFileName(true), width,
                            height, loadedImage.width, loadedImage.height);
                UnloadImage(loadedImage);
                for (auto& img : images)
                {
                    UnloadImage(img);
                }
                return {};
            }
            images.push_back(loadedImage);
        }
        const SpriteSheet result = ImportSpriteSheetVec(images, atlas, scale);
        return result;
    }

    SpriteSheet ImportSpriteSheetVec(const std::vector<Image>& images, AtlasID atlas, float scale)
    {
        if (images.empty())
        {
            LOG_WARNING("Cannot import sprite sheet from empty image vector");
            return {};
        }

        const int width = images[0].width;
        const int height = images[0].height;

        for (const auto& img : images)
        {
            if (img.width != width || img.height != height)
            {
                LOG_WARNING("Image size mismatch in sprite sheet import");
                return {};
            }
        }

        const Rectangle source = {0, 0, static_cast<float>(width), static_cast<float>(height)};
        Rectangle dest = {0, 0, static_cast<float>(width), static_cast<float>(height)};
        Image singleImage = GenImageColor(static_cast<int>(images.size()) * width, height, BLANK);
        for (size_t i = 0; i < images.size(); ++i)
        {
            ImageDraw(&singleImage, images[i], source, dest, WHITE);
            dest.x += static_cast<float>(width);
        }

        for (auto& img : images)
        {
            UnloadImage(img);
        }

        auto& atlasData = global::ATLAS_DATA.getAtlas(atlas);
        const auto sheet = atlasData.addSpriteSheet(singleImage, width, height, scale);
        return sheet;
    }

    EntityAnimation ImportAseprite(Asset asset, StateMapFunc func, AtlasID atlas, float scale)
    {
        EntityAnimation animation{scale};
        if (!(asset.hasExtension(".ase") || asset.hasExtension(".aseprite")))
        {
            LOG_WARNING("Invalid extensions for a asprite file");
            return animation;
        }

        auto* import = cute_aseprite_load_from_memory(asset.getData(), asset.getSize(), nullptr);
        const auto width = import->w;
        const auto height = import->h;

        std::vector<Image> images;
        for (int i = 0; i < import->tag_count; ++i)
        {
            auto& tag = import->tags[i];
            images.clear();
            DurationArray durations{};
            for (int l = tag.from_frame; l < tag.to_frame; ++l)
            {
                if (images.size() >= MAGIQUE_MAX_ANIM_FRAMES)
                {
                    LOG_WARNING("Too many frames in animation!");
                    break;
                }
                const auto& frame = import->frames[l];
                Image img = GenImageColor(width, height, BLANK);
                auto ticks = (float)frame.duration_milliseconds / (MAGIQUE_TICK_TIME * 1000.0F);
                durations[(int)images.size()] = (int)std::round(ticks);

                // Draw the frame
                for (int j = 0; j < height; ++j)
                {
                    for (int k = 0; k < width; ++k)
                    {
                        Color color;
                        ase_color_t aColor = frame.pixels[(j * width) + k];
                        color.a = aColor.a;
                        color.r = aColor.r;
                        color.g = aColor.g;
                        color.b = aColor.b;
                        ImageDrawPixel(&img, k, j, color);
                    }
                }

                images.push_back(img);
            }
            const auto sheet = ImportSpriteSheetVec(images, atlas, scale);
            const auto state = func(tag.name);
            animation.addAnimationEx(state, sheet, durations);
        }
        cute_aseprite_free(import);
        return animation;
    }

    Sound ImportSound(Asset asset)
    {
        ASSET_CHECK(asset);
        const auto ext = asset.getExtension();
        if (!IsSupportedSoundFormat(ext))
        {
            LOG_ERROR("Asset file type is not a sound file!: %s", ext);
            return {};
        }
        const Wave wave = LoadWaveFromMemory(ext, asset.getUData(), asset.getSize());
        const Sound sound = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return sound;
    }

    Music ImportMusic(Asset asset)
    {
        ASSET_CHECK(asset);
        const auto ext = asset.getExtension();
        if (!IsSupportedSoundFormat(ext))
        {
            LOG_ERROR("Asset file type is not a sound file!: %s", ext);
            return {};
        }
        // Duplicate data as stream isn't copied
        // This is leaked but until the end of the program - only loaded once?
        auto* newData = new char[asset.getSize()];
        std::memcpy(newData, asset.getData(), asset.getSize());
        const auto music = LoadMusicStreamFromMemory(ext, (unsigned char*)newData, asset.getSize());
        return music;
    }

    Playlist ImportPlaylist(const std::vector<Asset>& assets)
    {
        Playlist playlist{};
        if (assets.empty()) // Check empty
        {
            LOG_WARNING("Passed empty asset vector to RegisterPlaylist()");
            return playlist;
        }

        for (const auto& asset : assets)
        {
            ASSET_CHECK(asset);
            const auto* ext = asset.getExtension();
            if (!IsSupportedSoundFormat(ext))
            {
                LOG_ERROR("Asset file type is not a sound file!: %s", ext);
                return playlist;
            }
            auto music = ImportMusic(asset);
            playlist.addTrack(music);
        }
        return playlist;
    }

    TileMap ImportTileMap(Asset asset)
    {
        ASSET_CHECK(asset);
        TileMap tilemap{};
        if (strcmp(asset.getExtension(), ".tmj") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileMap: %s | Supported: .tmx", asset.getFileName());
            return TileMap{};
        }

        cute_tiled_map_t* map = cute_tiled_load_map_from_memory(asset.getData(), asset.getSize(), nullptr);
        if (map == nullptr)
        {
            LOG_ERROR("Failed to load TileMaps: %s", asset.getFileName());
            return tilemap;
        }

        tilemap.height = map->height;
        tilemap.width = map->width;

        cute_tiled_layer_t* layer = map->layers;
        while (layer != nullptr)
        {
            if (strcmp(layer->type.ptr, "objectgroup") == 0)
            {
                auto& objects = tilemap.objectLayers.emplace_back();
                cute_tiled_object_t* objectPtr = layer->objects;
                while (objectPtr != nullptr)
                {
                    TileObject object;
                    object.x = objectPtr->x;
                    object.y = objectPtr->y;
                    object.height = objectPtr->height;
                    object.width = objectPtr->width;
                    object.visible = objectPtr->visible == 1;
                    object.name = strdup(objectPtr->name.ptr);
                    object.id = objectPtr->id;

                    for (int i = 0; i < objectPtr->property_count; ++i)
                    {
                        if (i >= MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES)
                        {
                            LOG_WARNING("Too many custom properties in object %s", objectPtr->name);
                            break;
                        }
                        auto& propertyPtr = objectPtr->properties[i];
                        TileObjectCustomProperty property;
                        property.name = strdup(propertyPtr.name.ptr);
                        property.type = (TileObjectPropertyType)propertyPtr.type;
                        if (property.type == TileObjectPropertyType::STRING)
                        {
                            property.string = strdup(propertyPtr.data.string.ptr);
                        }
                        else
                        {
                            std::memcpy(&property.integer, &propertyPtr.data.integer, 4);
                        }
                        object.customProperties[i] = property;
                    }

                    objects.push_back(object);
                    objectPtr = objectPtr->next;
                }
            }
            else if (strcmp(layer->type.ptr, "tilelayer") == 0)
            {
                auto& dataVec = tilemap.tileLayers.emplace_back();
                dataVec.reserve((tilemap.width * tilemap.height) + 1);

                for (int i = 0; i < layer->data_count; ++i)
                {
                    dataVec.push_back(static_cast<uint16_t>(layer->data[i]));
                }
            }
            else
            {
                LOG_WARNING("Invalid tile map type: %s", layer->type.ptr);
            }
            layer = layer->next;
        }

        cute_tiled_free_map(map);
        return tilemap;
    }

    TileSet ImportTileSet(Asset asset)
    {
        ASSET_CHECK(asset);
        TileSet tileset{};
        if (strcmp(asset.getExtension(), ".tsj") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileSet: %s | Supported: .tsx", asset.getFileName(true));
            return tileset;
        }
        auto* import = cute_tiled_load_external_tileset_from_memory(asset.getData(), asset.getSize(), nullptr);


        cute_tiled_free_external_tileset(import);
        return tileset;
    }

    TileSheet ImportTileSheet(Asset asset, const int tileSize, const float scale)
    {
        ASSET_CHECK(asset);
        ASSET_IS_SUPPORTED_IMAGE_TYPE(asset);

        const auto sheet = TileSheet(asset, tileSize, scale);
        if (sheet.getTextureID() == 0)
        {
            LOG_WARNING("Failed to load TileSheet: %s", asset.getFileName());
            return sheet;
        }
        return sheet;
    }

} // namespace magique