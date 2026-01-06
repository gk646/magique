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
        return region;
    }

    TextureRegion ImportTexture(const Asset& asset, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        const Image image = LoadImage(asset);
        const int tarWidth = static_cast<int>(static_cast<float>(image.width) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(image.height) * scale);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto region = atlas.addTexture(image, tarWidth, tarHeight);
        return region;
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
        bool blank = true;

        for (const auto& img : images)
        {
            if (img.width != width || img.height != height)
            {
                LOG_WARNING("Image size mismatch in sprite sheet import");
                return {};
            }
            for (int i = 0; i < img.height; ++i)
            {
                for (int j = 0; j < img.width; ++j)
                {
                    const auto col = GetImageColor(img, j, i);
                    if (col.a != 0 || col.b != 0 || col.g != 0 || col.r != 0)
                    {
                        blank = false;
                        break;
                    }
                }
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

        for (const auto& img : images)
        {
            UnloadImage(img);
        }

        auto& atlasData = global::ATLAS_DATA.getAtlas(atlas);
        auto sheet = atlasData.addSpriteSheet(singleImage, width, height, scale);
        sheet.blank = blank;
        return sheet;
    }

    static void DrawCellFrame(Image& img, ase_t* ase, const ase_frame_t& frame, const std::vector<const char*>& layers)
    {
        bool none = true;
        for (int j = 0; j < frame.cel_count; ++j)
        {
            bool validLayer = true;
            const ase_cel_t* cel = frame.cels + j;
            for (const auto* name : layers)
            {
                if (strcmp(cel->layer->name, name) != 0)
                {
                    validLayer = false;
                    break;
                }
            }
            if (!validLayer)
            {
                continue;
            }
            none = false;

            while (cel->is_linked != 0)
            {
                ase_frame_t* frame = ase->frames + cel->linked_frame_index;
                for (int k = 0; k < frame->cel_count; ++k)
                {
                    if (frame->cels[k].layer == cel->layer)
                    {
                        cel = frame->cels + k;
                        break;
                    }
                }
            }
            void* src = cel->pixels;
            int cx = cel->x;
            int cy = cel->y;
            int cw = cel->w;
            int ch = cel->h;
            int cl = -s_min(cx, 0);
            int ct = -s_min(cy, 0);
            int dl = s_max(cx, 0);
            int dt = s_max(cy, 0);
            int dr = s_min(ase->w, cw + cx);
            int db = s_min(ase->h, ch + cy);
            for (int dx = dl, sx = cl; dx < dr; dx++, sx++)
            {
                for (int dy = dt, sy = ct; dy < db; dy++, sy++)
                {
                    ase_color_t src_color = s_color(ase, src, cw * sy + sx);
                    Color color;
                    color.a = src_color.a;
                    color.r = src_color.r;
                    color.g = src_color.g;
                    color.b = src_color.b;
                    ImageDrawPixel(&img, dx, dy, color);
                }
            }
        }
        if (none)
        {
            //LOG_WARNING("No matching layer!");
        }
    }

    EntityAnimation ImportAseprite(const Asset& asset, StateMapFunc func, AtlasID atlas, float scale)
    {
        return ImportAsepriteEx(asset, {}, func, atlas, scale);
    }

    EntityAnimation ImportAsepriteEx(const Asset& asset, const std::vector<const char*>& layers, StateMapFunc func,
                                     AtlasID atlas, float scale, Point offset, Point anchor)
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
            for (int l = tag.from_frame; l <= tag.to_frame; ++l)
            {
                if (images.size() >= MAGIQUE_MAX_ANIM_FRAMES)
                {
                    LOG_WARNING("Too many frames in animation!");
                    break;
                }
                const auto& frame = import->frames[l];
                Image img = GenImageColor(width, height, BLANK);
                durations[(int)images.size()] = frame.duration_milliseconds;

                DrawCellFrame(img, import, frame, layers);
                images.push_back(img);
            }
            const auto sheet = ImportSpriteSheetVec(images, atlas, scale);
            const auto state = func(tag.name);
            animation.addAnimationEx(state, sheet, durations, offset, anchor);
        }
        cute_aseprite_free(import);
        return animation;
    }

    Shader ImportShader(const Asset& vertex, const Asset& fragment)
    {
        if (vertex.getSize() != 0)
        {
            if (fragment.getSize() != 0)
            {
                return LoadShaderFromMemory(vertex.getData(), fragment.getData());
            }
            return LoadShaderFromMemory(vertex.getData(), nullptr);
        }
        else if (fragment.getSize() != 0)
        {
            return LoadShaderFromMemory(nullptr, fragment.getData());
        }
        LOG_WARNING("Passed empty asset for both vertex and fragment: Cant load shader");
        return {};
    }

    Sound ImportSound(const Asset& asset)
    {
        ASSET_CHECK(asset);
        const auto* ext = asset.getExtension();
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

    Music ImportMusic(const Asset& asset)
    {
        ASSET_CHECK(asset);
        const auto* ext = asset.getExtension();
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

    struct TiledPropertyParser final
    {
        //TODO is leaking memory with name and property value / is it bad?

        static void ParseProperty(TiledProperty& prop, const cute_tiled_property_t& tileProp)
        {
            prop.type = static_cast<TileObjectPropertyType>(tileProp.type);
            if (prop.type == TileObjectPropertyType::STRING)
            {
                prop.string = strdup(tileProp.data.string.ptr);
            }
            else
            {
                std::memcpy(&prop.integer, &tileProp.data.integer, 4);
            }
            prop.name = strdup(tileProp.name.ptr);
        }
    };

    TileMap ImportTileMap(const Asset& asset)
    {
        ASSET_CHECK(asset);
        TileMap tilemap{};
        if (strcmp(asset.getExtension(), ".tmj") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileMap: %s | Supported: .tmj", asset.getFileName());
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

        // Parse tilemap properties
        for (int i = 0; i < map->property_count; ++i)
        {
            TiledProperty property;
            TiledPropertyParser::ParseProperty(property, map->properties[i]);
            tilemap.properties.push_back(property);
        }

        while (layer != nullptr)
        {
            if (strcmp(layer->type.ptr, "objectgroup") == 0)
            {
                auto& objects = tilemap.objectLayers.emplace_back();
                cute_tiled_object_t* objectPtr = layer->objects;
                while (objectPtr != nullptr)
                {
                    TileObject object;
                    object.bounds = {objectPtr->x, objectPtr->y, objectPtr->width, objectPtr->height};
                    object.rotation = objectPtr->rotation;
                    object.visible = objectPtr->visible == 1;
                    object.name = strdup(objectPtr->name.ptr);
                    object.id = objectPtr->id;
                    object.tileId = objectPtr->gid;
                    if (object.tileId != 0)
                    {
                        object.bounds.y -= object.bounds.h;
                    }

                    // Parsing properties for objects
                    for (int i = 0; i < objectPtr->property_count; ++i)
                    {
                        if (i >= MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES)
                        {
                            LOG_WARNING("Too many custom properties in object %s", objectPtr->name);
                            break;
                        }
                        TiledProperty property;
                        TiledPropertyParser::ParseProperty(property, objectPtr->properties[i]);
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
                    dataVec.push_back(static_cast<int16_t>(layer->data[i]));
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

    TileSet ImportTileSet(const Asset& asset, TileClassMapFunc func)
    {
        ASSET_CHECK(asset);
        TileSet tileset{};
        if (strcmp(asset.getExtension(), ".tsj") != 0)
        {
            LOG_WARNING("Invalid extensions for a TileSet: %s | Supported: .tsj", asset.getFileName(true));
            return tileset;
        }
        auto* import = cute_tiled_load_external_tileset_from_memory(asset.getData(), asset.getSize(), nullptr);
        tileset.tileSize = import->tilewidth;
        tileset.tileCount = import->tilecount;
        cute_tiled_tile_descriptor_t* tile = import->tiles;
        while (tile != nullptr)
        {
            TileInfo info{};
            if (tile->type.ptr != nullptr && func != nullptr)
            {
                info.tileClass = func(tile->type.ptr);
            }
            info.hasCollision = tile->objectgroup != nullptr;
            if (info.hasCollision)
            {
                auto& object = *tile->objectgroup->objects;
                info.x = std::round(object.x);
                info.y = std::round(object.y);
                info.width = std::round(object.width);
                info.height = std::round(object.height);

                if (object.next != nullptr)
                {
                    object = *object.next;
                    info.sx = std::round(object.x);
                    info.sy = std::round(object.y);
                    info.swidth = std::round(object.width);
                    info.sheight = std::round(object.height);
                }
            }
            const auto* ptr = tile->image.ptr;
            if (ptr != nullptr)
            {
                info.image = strdup(tile->image.ptr);
            }
            info.tileID = tile->tile_index;

            for (int i = 0; i < tile->property_count; ++i)
            {
                if (i >= MAGIQUE_TILE_SET_CUSTOM_PROPERTIES)
                {
                    LOG_WARNING("Too many custom properties for tileID %d", tile->tile_index);
                    break;
                }
                TiledProperty property;
                TiledPropertyParser::ParseProperty(property, tile->properties[i]);
                info.customProperties[i] = property;
            }

            tileset.infoVec.push_back(info);
            tile = tile->next;
        }
        cute_tiled_free_external_tileset(import);
        return tileset;
    }

    TileSheet ImportTileSheet(const Asset& asset, const int tileSize, const float scale)
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
