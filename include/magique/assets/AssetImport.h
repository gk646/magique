// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSETMANAGER_H
#define MAGIQUE_ASSETMANAGER_H

#include <vector>
#include <raylib/raylib.h>
#include <magique/core/Types.h>
#include <magique/assets/types/Asset.h>
#include <magique/graphics/Animation.h>
#include <magique/gamedev/Localization.h>
#include <magique/internal/cute_asprite.h>

//===============================================
// Asset Import
//===============================================
// ................................................................................
// This modules allows to import asset and load them into usable types
// Textures are automatically stitched into the default atlas or the specified one
// Try to group all textures that are drawn together into the same atlas
// Note: If you load many texture you have to specify a custom AtlasID's or else the default one will be full -> error
// ................................................................................

enum class AtlasID : int; // User implemented - per default all textures will be loaded into atlas 0

namespace magique
{
    //================= Textures =================//

    // Allows to register an already existing or custom loaded texture onto the given atlas
    TextureRegion ImportTexture(const Texture& texture, AtlasID atlas = {});
    TextureRegion ImportTexture(const Image& img, AtlasID atlas = {});

    // Loads the whole image as texture into the given atlas
    // scale    - controls the final dimensions of the resulting texture
    TextureRegion ImportTexture(Asset asset, AtlasID atlas = {}, float scale = 1);

    // Loads the asset as image
    Image ImportImage(Asset asset);

    //================= Animations =================//

    // Tries to load a .png file as sprite sheet - parses row wise from left to right and top to bottom
    //      - dims: size of a single frame
    //      - atlas: which atlas to load the spritesheet into
    //      - scale: allows scaling the result at creation time
    // Note: Combined width of all frames must not exceed MAGIQUE_TEXTURE_ATLAS_SIZE!
    SpriteSheet ImportSprite(Asset asset, Point dims, AtlasID atlas = {}, float scale = 1);

    // Extended parameters
    //      - frames: how many frames to load
    //      - offset: offset into the image where to start parsing
    SpriteSheet ImportSprite(Asset asset, Point dims, AtlasID atlas, Point offset, int frames, float scale = 1);

    // Register a sprite sheet out of single images - must all have the same dimensions
    // Useful if you have textures as separate images instead of a single SpriteSheet
    // Use with iterateDirectory()
    SpriteSheet ImportSpriteVec(std::span<const Asset> assets, AtlasID atlas = {}, float scale = 1);
    SpriteSheet ImportSpriteVec(std::span<const Image> images, AtlasID atlas = {}, float scale = 1);

    // Imports all tags from the given aseprite - tags are mapped to enum values of AnimationState
    // If not specified rotation anchor will be the center
    // Note: Only imports frames that are part of a tag
    // IMPORTANT: AnimationState enum NEEDS to be defined with the tag values
    template <typename AnimationEnum = AnimationState>
    Animation ImportAnimation(Asset asset, AtlasID atlas = {}, float scale = 1, Point offset = {});

    template <typename AnimationEnum = AnimationState>
    Animation ImportAnimation(Asset asset, AtlasID atlas, float scale, Point offset, Point anchor);

    using LayerMapFunc = LayeredAnimation (*)(const char* layerName);

    // Imports each layer separately into its own entity animation
    // Note: Uses the mapping function to map layer name in the editor to AnimationLayer values
    template <typename AnimationEnum = AnimationState>
    std::vector<std::pair<LayeredAnimation, Animation>> ImportAsepriteLayers(Asset asset, LayerMapFunc mapping,
                                                                             AtlasID atlas = {}, float scale = 1,
                                                                             Point offset = {}, Point anchor = {-1});

    //================= Audio =================//

    // Registers a sound file - can be any raylib supported file type (.mp3, .wav)
    Sound ImportSound(const Asset& asset);

    // Register a music file (streamed audio) - can be any raylib supported type (.mp3)
    // Everything above 10s should be loaded as music (and compressed with .mp3) instead of sound!
    Music ImportMusic(const Asset& asset);

    // Tries to load a playlist from the given assets - they all have to be supported raylib music types
    Playlist ImportPlaylist(const std::vector<Asset>& assets);

    //================= Tiled Exports =================//

    // Note: Generally you have multiple TileMaps, but only 1 TileSet and 1 TileSheet!
    // Note: A TilSheet has its own atlas - you can access it manually as well and get a TextureRegion back

    // Registers a tilemap from an export file - Supported: ".tsj" (Tiled),
    // Supports loading multiple layers - all layers must have same dimensions!
    TileMap ImportTileMap(const Asset& asset);

    // Registers a tilemap from the given custom layer data
    // This creates a new tilemap out of the given data like so:
    // Outer vector: Layers  |  Inner vector 1: holds the column vectors | Inner vector 2: holds tile indices for the column
    // -> This means column major order -> layerData[1][10][5] = the tile number in layer 1, in the 10th column and 5th row
    TileMap ImportTileMapRaw(const std::vector<std::vector<std::vector<uint16_t>>>& layerData);

    // Registers a tileset - defines the details of all tiles in a project
    TileSet ImportTileSet(const Asset& asset, TileClassMapFunc func = nullptr);

    // Registers a tile sheet from a single ".png" file
    // IMPORTANT: Make sure the TileSheet has exact dimensions and no padding!
    // 'textureSize'    - specify the width and height of each source texture tile
    // 'scale'          - allows to scale the resulting texture (rounded down)
    // Note: The tilesheet will have its own texture
    TileSheet ImportTileSheet(const Asset& asset, int tileSize, float scale = 1);

    //================= Other =================//

    // Imports a font with the given basesize and amount of characters
    // Note: Starts from character 32 onwards
    Font ImportFont(const Asset& asset, int baseSize, int characters = 0);

    // Imports a shader by loading the given assets as vertex and fragment shaders
    // Note: Either one can be empty - It's allowed to load with only a fragment, only a vertex or both
    Shader ImportShader(const Asset& vertex, const Asset& fragment);

    // Imports the given asset into a simple container
    // The file is separated and stored line-wise
    // Failure: Returns empty container
    TextLines ImportText(Asset asset, char delimiter = '\n');

    // Tries to import a language from the given file
    // Supported types:
    //      - .po  (GNU gettext format - see: https://github.com/vslavik/poedit)
    LocalizedLanguage ImportGettext(Asset asset);

    // Imports a .pot file - this is the base of file that only contains keys not translation
    // The translation will be set to the keys itself and the language to the given one (as none is specified in the file)
    LocalizedLanguage ImportGettextBase(Asset asset, Language lang = Language::EN);

} // namespace magique

// IMPLEMENTATION


namespace magique
{
    namespace internal
    {
        static Image AsepriteCellToImg(ase_cel_t& cell)
        {
            Image image = GenImageColor(cell.w, cell.h, BLANK);
            std::memcpy(image.data, cell.pixels, sizeof(Color) * cell.w * cell.h);
            return image;
        }

        static Image AsepriteFrameToImg(ase_frame_t& frame)
        {
            Image image = GenImageColor(frame.ase->w, frame.ase->h, BLANK);
            std::memcpy(image.data, frame.pixels, sizeof(Color) * frame.ase->w * frame.ase->h);
            return image;
        }

        static ase_cel_t AsepriteFindLayerCell(const ase_frame_t& frame, const ase_layer_t& layer)
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

        template <typename AnimationEnum, typename Func>
        static Animation AsepriteIterateTags(ase_t* import, Func func, Point offset, Point anchor, AtlasID atlas,
                                             float scale)
        {
            Animation animation{scale};
            std::vector<Image> images = {};
            for (const auto& tag : std::span{import->tags, (size_t)import->tag_count})
            {
                FrameDuration durations{};
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

                const auto sheet = ImportSpriteVec(images, atlas, scale);

                auto stateVal = EnumFromString<AnimationEnum>(tag.name);
                if (!stateVal.has_value())
                    LOG_ERROR("Cannot map aseprite tag: %s\n Add as enum value to AnimationState", tag.name);

                animation.addAnimationEx(stateVal.value_or({}), sheet, durations, offset, anchor);
            }
            return animation;
        }
    } // namespace internal


    template <typename AnimationEnum>
    std::vector<std::pair<LayeredAnimation, Animation>>
    ImportAsepriteLayers(Asset asset, LayerMapFunc layerMap, AtlasID atlas, float scale, Point offset, Point anchor)
    {
        std::vector<std::pair<LayeredAnimation, Animation>> animations;
        auto* import = cute_aseprite_load_from_memory((const char*)asset, asset.getSize(), nullptr);
        if (anchor == -1)
            anchor = Point{(float)import->w, (float)import->h} / 2;

        for (const auto& layer : std::span{import->layers, (size_t)import->layer_count})
        {
            auto frameFunc = [&](std::vector<Image>& images, ase_frame_t& frame)
            {
                auto cell = internal::AsepriteFindLayerCell(frame, layer);
                images.push_back(internal::AsepriteCellToImg(cell));
            };
            auto animation =
                internal::AsepriteIterateTags<AnimationEnum>(import, frameFunc, offset, anchor, atlas, scale);
            animations.emplace_back(layerMap(layer.name), std::move(animation));
        }

        cute_aseprite_free(import);
        return animations;
    }

    template <typename AnimationEnum>
    Animation ImportAnimation(Asset asset, AtlasID atlas, float scale, Point offset)
    {
        if (!(asset.endsWith(".ase") || asset.endsWith(".aseprite")))
        {
            LOG_WARNING("Invalid extensions for a aseprite file: %s", asset.getExtension().data());
            return {};
        }

        auto* import = cute_aseprite_load_from_memory((const char*)asset, asset.getSize(), nullptr);
        auto frameFunc = [](std::vector<Image>& images, ase_frame_t& frame)
        {
            images.push_back(internal::AsepriteFrameToImg(frame));
        };

        const Point anchor = Point{(float)import->w, (float)import->h} / 2.0F;
        auto animation = internal::AsepriteIterateTags<AnimationEnum>(import, frameFunc, offset, anchor, atlas, scale);
        cute_aseprite_free(import);
        return animation;
    }

    template <typename AnimationEnum>
    Animation ImportAnimation(Asset asset, AtlasID atlas, float scale, Point offset, Point anchor)
    {
        if (!(asset.endsWith(".ase") || asset.endsWith(".aseprite")))
        {
            LOG_WARNING("Invalid extensions for a aseprite file");
            return {};
        }

        auto* import = cute_aseprite_load_from_memory((const char*)asset, asset.getSize(), nullptr);
        auto frameFunc = [](std::vector<Image>& images, ase_frame_t& frame)
        {
            images.push_back(internal::AsepriteFrameToImg(frame));
        };

        auto animation = internal::AsepriteIterateTags<AnimationEnum>(import, frameFunc, offset, anchor, atlas, scale);
        cute_aseprite_free(import);
        return animation;
    }
} // namespace magique

#endif // MAGIQUE_ASSETMANAGER_H
