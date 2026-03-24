// SPDX-License-Identifier: zlib-acknowledgement
#include <cmath>
#include <raylib/raylib.h>

#include <magique/assets/AssetImport.h>
#include <magique/assets/types/TextLines.h>
#include <magique/assets/types/Asset.h>
#include <magique/assets/types/TileMap.h>
#include <magique/assets/types/TileSheet.h>
#include <magique/assets/types/TileSet.h>
#include <magique/assets/types/Playlist.h>
#include <magique/core/Types.h>

#include "internal/globals/TextureAtlas.h"
#include "assets/headers/AssetUtil.h"
#include "magique/core/Animation.h"

#define CUTE_ASEPRITE_IMPLEMENTATION
#define CUTE_TILED_NO_EXTERNAL_TILESET_WARNING
#define CUTE_TILED_IMPLEMENTATION

#define CUTE_TILED_WARNING(msg) LOG_ERROR(msg);

#include "enchantum/enchantum.hpp"
#include "external/cute_asprite.h"
#include "external/cute_tiled.h"

namespace magique
{
    TextureRegion ImportTexture(const Texture& texture, AtlasID atlas)
    {
        MAGIQUE_ASSERT(texture.id != 0, "Trying to register an invalid texture");
        return ImportTexture(LoadImageFromTexture(texture), atlas);
    }

    TextureRegion ImportTexture(const Image& img, AtlasID atlas)
    {
        auto& texAtlas = global::ATLAS_DATA.getAtlas(atlas);
        const auto region = texAtlas.addTexture(img, img.width, img.height);
        return region;
    }

    TextureRegion ImportTexture(const Asset& asset, const AtlasID at, const float scale)
    {
        ASSET_CHECK(asset);
        const Image image = ImportImage(asset);
        const int tarWidth = static_cast<int>(static_cast<float>(image.width) * scale);
        const int tarHeight = static_cast<int>(static_cast<float>(image.height) * scale);
        auto& atlas = global::ATLAS_DATA.getAtlas(at);
        const auto region = atlas.addTexture(image, tarWidth, tarHeight);
        return region;
    }

    Image ImportImage(const Asset& asset)
    {
        ASSET_CHECK(asset);
        const auto* ext = asset.getExtension().data();
        MAGIQUE_ASSERT(IsSupportedImageFormat(ext), "No valid extension");
        const auto img = LoadImageFromMemory(ext, asset.getUData(), asset.getSize());
        MAGIQUE_ASSERT(img.data != nullptr, "No image data loaded");
        return img;
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
        Image firstImage = ImportImage(assets[0]);
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
            Image loadedImage = ImportImage(assets[i]);
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

    static Image CellToImg(ase_cel_t& cell)
    {
        Image image = GenImageColor(cell.w, cell.h, BLANK);
        std::memcpy(image.data, cell.pixels, sizeof(Color) * cell.w * cell.h);
        return image;
    }

    static Image FrameToImg(ase_frame_t& frame)
    {
        Image image = GenImageColor(frame.ase->w, frame.ase->h, BLANK);
        std::memcpy(image.data, frame.pixels, sizeof(Color) * frame.ase->w * frame.ase->h);
        return image;
    }

    static ase_cel_t FindLayerCell(const ase_frame_t& frame, const ase_layer_t& layer)
    {
        for (const auto& cell : std::span{frame.cels, (size_t)frame.cel_count})
        {
            if (cell.layer == &layer)
            {
                return cell;
            }
        }
        return ase_cel_t{};
    }

    template <typename Func>
    static EntityAnimation IterateTags(ase_t* import, Func func, StateMapFunc mapFunc, Point offset, Point anchor,
                                       AtlasID atlas, float scale)
    {
        EntityAnimation animation{scale};
        std::vector<Image> images = {};
        for (const auto& tag : std::span{import->tags, (size_t)import->tag_count})
        {
            DurationArray durations{};
            images.clear();
            if (tag.to_frame - tag.from_frame >= MAGIQUE_MAX_ANIM_FRAMES)
            {
                LOG_WARNING("Too many frames in animation!");
                continue;
            }

            for (int l = tag.from_frame; l <= tag.to_frame; ++l)
            {
                auto& frame = import->frames[l];
                durations[(int)images.size()] = frame.duration_milliseconds;
                func(images, frame);
            }

            const auto sheet = ImportSpriteSheetVec(images, atlas, scale);
            const auto state = mapFunc(tag.name);
            animation.addAnimationEx(state, sheet, durations, offset, anchor);
        }
        return animation;
    }

    EntityAnimation ImportAseprite(const Asset& asset, StateMapFunc mapFunc, AtlasID atlas, float scale, Point offset,
                                   Point anchor)
    {
        if (!(asset.endsWith(".ase") || asset.endsWith(".aseprite")))
        {
            LOG_WARNING("Invalid extensions for a aseprite file");
            return {};
        }

        auto* import = cute_aseprite_load_from_memory(asset.getData(), asset.getSize(), nullptr);
        if (anchor == -1)
            anchor = Point{(float)import->w, (float)import->h} / 2;

        auto frameFunc = [](std::vector<Image>& images, ase_frame_t& frame)
        {
            images.push_back(FrameToImg(frame));
        };

        auto animation = IterateTags(import, frameFunc, mapFunc, offset, anchor, atlas, scale);
        cute_aseprite_free(import);
        return animation;
    }

    std::vector<std::pair<AnimationLayer, EntityAnimation>> ImportAsepriteLayers(Asset asset, StateMapFunc stateMap,
                                                                                 AtlasID atlas, LayerMapFunc layerMap,
                                                                                 float scale, Point offset, Point anchor)
    {
        std::vector<std::pair<AnimationLayer, EntityAnimation>> animations;
        auto* import = cute_aseprite_load_from_memory(asset.getData(), asset.getSize(), nullptr);
        if (anchor == -1)
            anchor = Point{(float)import->w, (float)import->h} / 2;

        for (const auto& layer : std::span{import->layers, (size_t)import->layer_count})
        {
            auto frameFunc = [&](std::vector<Image>& images, ase_frame_t& frame)
            {
                auto cell = FindLayerCell(frame, layer);
                images.push_back(CellToImg(cell));
            };
            auto animation = IterateTags(import, frameFunc, stateMap, offset, anchor, atlas, scale);
            animations.emplace_back(layerMap(layer.name), std::move(animation));
        }

        cute_aseprite_free(import);
        return animations;
    }

    Sound ImportSound(const Asset& asset)
    {
        ASSET_CHECK(asset);
        const auto* ext = asset.getExtension().data();
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
        const auto* ext = asset.getExtension().data();
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
            const auto* ext = asset.getExtension().data();
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

    struct TiledPropertyParser final
    {
        // TODO is leaking memory with name and property value / is it bad?

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
        if (asset.getExtension() != ".tmj")
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

        tilemap.tileSize = map->tileheight;
        tilemap.height = map->height;
        tilemap.width = map->width;

        const cute_tiled_layer_t* layer = map->layers;

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
                auto& objectLayer = tilemap.objectLayers.emplace_back();
                objectLayer.name = layer->name.ptr;
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
                        object.bounds.y -= object.bounds.height;
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

                    objectLayer.objects.push_back(object);
                    objectPtr = objectPtr->next;
                }
            }
            else if (strcmp(layer->type.ptr, "tilelayer") == 0)
            {
                auto& tileLayer = tilemap.tileLayers.emplace_back();
                tileLayer.name = layer->name.ptr;
                tileLayer.dims = {(float)layer->width, (float)layer->height};
                tileLayer.tiles.reserve((tilemap.width * tilemap.height) + 1);

                for (int i = 0; i < layer->data_count; ++i)
                {
                    int hFlip{};
                    int vFlip{};
                    int dFlip{};

                    auto tileId = layer->data[i];
                    cute_tiled_get_flags(tileId, &hFlip, &vFlip, &dFlip);
                    tileId = cute_tiled_unset_flags(tileId);

                    tileLayer.tiles.push_back({(int16_t)tileId, hFlip == 1, vFlip == 1, dFlip == 1});
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
        if (asset.getExtension() != ".tsj")
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
                info.bounds = {std::round(object.x), std::round(object.y), std::round(object.width),
                               std::round(object.height)};
                if (object.next != nullptr)
                {
                    object = *object.next;
                    info.secBounds = {std::round(object.x), std::round(object.y), std::round(object.width),
                                      std::round(object.height)};
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
        if (sheet.getRegion(0).id == 0)
        {
            LOG_WARNING("Failed to load TileSheet: %s", asset.getFileName());
            return sheet;
        }
        return sheet;
    }

    Font ImportFont(const Asset& asset, int baseSize, int characters)
    {
        return LoadFontFromMemory(".ttf", asset.getUData(), asset.getSize(), baseSize, nullptr, characters);
    }

    TextLines ImportText(Asset asset, char delimiter) { return TextLines{asset.getData(), delimiter}; }

    static size_t findAfter(const std::string_view& str, const char* what, size_t pos = 0)
    {
        size_t found = str.find(what, pos);
        if (found != std::string::npos)
        {
            return found + strlen(what);
        }
        return found;
    }

    LocalizedLanguage ImportGettext(Asset asset)
    {
        LocalizedLanguage language{};
        if (asset.getExtension() != ".po")
        {
            LOG_WARNING("Invalid extension for gettext file: %s", asset.getExtension().data());
            return language;
        }

        const std::string_view file{asset.getData(), (size_t)asset.getSize()};

        const auto langStr = file.substr(findAfter(file, "Language: "), 2);
        language.language = LocalizationParseLanguage(langStr);
        if (language.language == Language::None)
        {
            LOG_WARNING("Failed to import gettext file %s: No such language: %.2s", asset.getFileName().data(),
                        langStr.data());
            return language;
        }

        // Remove newlines and " and comments
        auto removeSymbols = [](std::string& str)
        {
            size_t pos = str.find("\n#");
            while (pos != std::string::npos)
            {
                auto end = str.find("\n", pos + 1);
                str.erase(pos, end - pos);
                pos = str.find("\n#");
            }

            pos = str.find("\"\n");
            while (pos != std::string::npos)
            {
                str.erase(pos, 3);
                pos = str.find("\"\n");
            }
        };

        std::string key{};
        size_t pos = file.find("msgid", findAfter(file, "msgid")); // Start with the second
        while (pos < (size_t)asset.getSize())
        {
            auto msgidBegin = findAfter(file, "msgid ", pos);
            if (msgidBegin == std::string_view::npos)
                break;
            auto msgidEnd = file.find("msgstr ", pos);

            auto msgstrBegin = findAfter(file, "msgstr ", msgidBegin);
            if (msgstrBegin == std::string_view::npos)
                break;

            auto msgstrEnd = file.find("msgid ", msgstrBegin);

            if (msgstrEnd == std::string_view::npos)
                msgstrEnd = asset.getSize() - 2;

            pos = msgstrEnd;

            auto msgid = file.substr(msgidBegin + 1, msgidEnd - msgidBegin);
            auto msgstr = file.substr(msgstrBegin + 1, (msgstrEnd - msgstrBegin) - 1);

            key = msgid;
            removeSymbols(key);
            auto& value = language.translations[key];
            value = msgstr;
            removeSymbols(value);
        }
        return language;
    }

    LocalizedLanguage ImportMTF(Asset asset)
    {
        LocalizedLanguage language{};
        if (asset.getExtension() != ".mtf")
        {
            LOG_WARNING("Invalid extension for magique traslation file: %s", asset.getExtension().data());
            return language;
        }
        return language;
    }

} // namespace magique
