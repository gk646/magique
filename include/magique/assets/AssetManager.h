// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSETMANAGER_H
#define MAGIQUE_ASSETMANAGER_H

#include <vector>
#include <magique/core/Types.h>

//===============================================
// Asset Management Module
//===============================================
// ................................................................................
// This modules allows to register as resources to the asset management system
// This means both loading the raw asset into its correct type (e.g. RegisterTexture() asset -> Texture) AND storing it
// internally so it can be retrieved at any point.
// It uses a handle system, which means you get a number that identifies this resource from the Register method
// This might seem cumbersome (the handle has to be stored and accessed aswell) but has some advantages:
//     - All access to the resource use the same thing -> To change all usages you just need to change 1 handle
//     -

// Textures are automatically stitched into the default atlas or the specified one
// Try to group all textures that are drawn together into the same atlas
// Note: If you load many texture you have to specify custom AtlasID's or else the default one will be full -> error
// However this handle is only unique for its type, so don't use a handle from a SpriteSheet for a texture or a sound
// In practice this doesn't mean anything as you organize handles anyway
// This handle system is very fast - all items from a resource type (Sound, TextureRegion) are stored in a contiguous vector
// The handles are direct indices for lookups in those vectors O(1)
//
// IMPORTANT: All methods return handle::null if there are errors!
// Note: Most method also take the finished resources e.g. RegisterTexture(asset) and RegisterTexture(texture)
//       This allows you have control over loading resources but still register them to the asset manager
// ................................................................................

enum class AtlasID : int; // User implemented - per default all textures will be loaded into atlas 0

namespace magique
{
    enum class handle : uint32_t
    {
        null = UINT32_MAX // The null handle - identifies invalid handles
    };

    //================= Textures =================//

    // Allows to register an already existing or custom loaded texture onto the given atlas
    handle RegisterTexture(const Texture& texture, AtlasID atlas = {});

    // Loads the whole image as texture into the given atlas
    // scale    - controls the final dimensions of the resulting texture
    handle RegisterTexture(Asset asset, AtlasID atlas = {}, float scale = 1);

    // Tries to load a .png file as sprite sheet
    // Starts at (0,0) top left and then tries to split the image into frames row by row with the given dimensions
    // 'scale' allows to scale the resulting texture (rounded down)
    // Note: Combined width of all frames must not exceed MAGIQUE_TEXTURE_ATLAS_SIZE!
    handle RegisterSpriteSheet(Asset asset, int width, int height, AtlasID atlas = {}, float scale = 1);

    // Sames as RegisterSpriteSheet but allows to specify an offset from the top left and the amount of frames to load
    // Useful for loading part of a bigger sprite sheet - supports line breaks
    handle RegisterSpriteSheetEx(Asset asset, int width, int height, int frames, int offX, int offY, AtlasID atlas = {},
                                 float scale = 1);

    // Register a sprite sheet out of single images - must all have the same dimensions
    // Useful if you have textures as separate images instead of a single SpriteSheet
    // Use with iterateDirectory()
    handle RegisterSpriteSheetVec(const std::vector<Asset>& assets, AtlasID atlas = {}, float scale = 1);

    //================= Audio =================//

    // Allows to register an already existing or custom loaded sound
    handle RegisterSound(const Sound& sound);

    // Registers a sound file - can be any raylib supported file type (.mp3, .wav)
    handle RegisterSound(Asset asset);

    // Register a music file (streamed audio) - can be any raylib supported type (.mp3)
    // Everything above 10s should be loaded as music (and compressed with .mp3) instead of sound!
    handle RegisterMusic(Asset asset);

    // Tries to load a playlist from the given assets - they all have to be supported raylib music types (.mp3, .wav, .ogg, ...)
    handle RegisterPlaylist(const std::vector<Asset>& assets);

    //================= Tile Exports =================//
    // Note: Generally you have multiple TileMaps, but only 1 TileSet and 1 TileSheet!
    // Note: A TilSheet has its own atlas - you can access it manually as well and get a TextureRegion back

    // Registers a tilemap from an export file - Supported: ".tsx" (Tiled),
    // Supports loading multiple layers - all layers must have same dimensions!
    handle RegisterTileMap(Asset asset);

    // Registers a tilemap from the given custom layer data
    // This creates a new tilemap out of the given data like so:
    // Outer vector: Layers  |  Inner vector 1: holds the column vectors | Inner vector 2: holds tile indices for the column
    // -> This means column major order -> layerData[1][10][5] = the tile number in layer 1, in the 10th column and 5th row
    handle RegisterTileMap(const std::vector<std::vector<std::vector<uint16_t>>>& layerData);

    // Registers a tileset - defines the details of all tiles in a project
    handle RegisterTileSet(Asset asset);

    // Registers a tile sheet from a single ".png" file
    // IMPORTANT: Make sure the TileSheet has exact dimensions and no padding!
    // 'textureSize'    - specify the width and height of each source tile
    // 'scale'          - allows to scale the resulting texture (rounded down)
    handle RegisterTileSheet(Asset asset, int tileSize, float scale = 1);

    // Same as 'RegisterTileSheet()' but takes a list of pictures
    // All assets provided must have the same dimensions and be pictures
    // This is useful if you have split images instead of a single TileSheet - Use with iterateDirectory()
    handle RegisterTileSheet(const std::vector<Asset>& assets, int tileSize, float scale = 1);


    //================= GET =================//

    // Returns the resources identified by this handle
    TextureRegion GetTexture(handle handle);
    SpriteSheet GetSpriteSheet(handle handle);
    Sound& GetSound(handle handle);
    TileMap& GetTileMap(handle handle);
    TileSet& GetTileSet(handle handle);
    TileSheet& GetTileSheet(handle handle);
    Music& GetMusic(handle handle);
    Playlist& GetPlaylist(handle handle);

    //================= DIRECT GET =================//
    // Note: These methods assume you registered the handle with RegisterHandle() (see assets/HandleRegistry.h for info)
    // They call GetHandle() internally thus skipping the manual call

    TextureRegion GetTexture(HandleID hash);
    TextureRegion GetTexture(uint32_t hash);

    TileMap& GetTileMap(HandleID id);
    TileMap& GetTileMap(uint32_t hash);

    TileSheet& GetTileSheet(HandleID handle);
    TileSheet& GetTileSheet(uint32_t hash);

    TileSet& GetTileSet(HandleID handle);
    TileSet& GetTileSet(uint32_t hash);

    Sound& GetSound(HandleID handle);
    Sound& GetSound(uint32_t handle);


} // namespace magique


#endif //MAGIQUE_ASSETMANAGER_H