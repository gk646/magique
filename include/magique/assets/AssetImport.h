// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSETMANAGER_H
#define MAGIQUE_ASSETMANAGER_H

#include <vector>
#include <raylib/raylib.h>
#include <magique/core/Types.h>

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

    // Loads the whole image as texture into the given atlas
    // scale    - controls the final dimensions of the resulting texture
    TextureRegion ImportTexture(const Asset& asset, AtlasID atlas = {}, float scale = 1);

    // Loads the asset as image
    Image ImportImage(const Asset& asset);

    //================= Animations =================//

    // Tries to load a .png file as sprite sheet
    // Starts at (0,0) top left and then tries to split the image into frames row by row with the given dimensions
    // 'scale' allows to scale the resulting texture (rounded down)
    // Note: Combined width of all frames must not exceed MAGIQUE_TEXTURE_ATLAS_SIZE!
    SpriteSheet ImportSpriteSheet(Asset asset, int width, int height, AtlasID atlas = {}, float scale = 1);

    // Sames as RegisterSpriteSheet but allows to specify an offset from the top left and the amount of frames to load
    // Useful for loading part of a bigger sprite sheet - supports line breaks
    SpriteSheet ImportSpriteSheetEx(Asset asset, int width, int height, int frames, int offX, int offY,
                                    AtlasID atlas = {}, float scale = 1);

    // Register a sprite sheet out of single images - must all have the same dimensions
    // Useful if you have textures as separate images instead of a single SpriteSheet
    // Use with iterateDirectory()
    SpriteSheet ImportSpriteSheetVec(const std::vector<Asset>& assets, AtlasID atlas = {}, float scale = 1);
    SpriteSheet ImportSpriteSheetVec(const std::vector<Image>& images, AtlasID atlas = {}, float scale = 1);

    // Imports an aseprite file with all the frames and duration set (.ase,.aseprite)
    using StateMapFunc = AnimationState (*)(const char* tagName);
    // Uses the mapping function to get the animation state from the aseprite tag name
    EntityAnimation ImportAseprite(const Asset& asset, StateMapFunc func, AtlasID atlas = {}, float scale = 1.0F);

    // Only imports the given layers
    EntityAnimation ImportAsepriteEx(const Asset& asset, const std::vector<const char*>& layers, StateMapFunc func,
                                     AtlasID atlas = {}, float scale = 1.0F, Point offset = {}, Point anchor = {-1});

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

    // Imports a font with the given basesize
    // Note: Starts from character 32 onwards
    Font ImportFont(const Asset& asset, int baseSize);

    // Imports a shader by loading the given assets as vertex and fragment shaders
    // Note: Either one can be empty - its allowed to load with only a fragment, only a vertex or both
    Shader ImportShader(const Asset& vertex, const Asset& fragment);

    // Imports the given asset into a simple container
    // The file is separated and stored line-wise
    // Failure: Returns empty container
    TextLines ImportText(Asset asset, char delimiter = '\n');

} // namespace magique


#endif // MAGIQUE_ASSETMANAGER_H
