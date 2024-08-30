#ifndef MAGIQUE_ASSETMANAGER_H
#define MAGIQUE_ASSETMANAGER_H

#include <vector>
#include <magique/core/Types.h>

//-----------------------------------------------
// Asset Management Module
//-----------------------------------------------
// ................................................................................
// All Register__ methods load and store the resource internally
// The returned handle is used by Get__ methods to retrieve the resource again
// Per default all methods are made to load from memory to work with the asset image
// Textures are automatically stitched into the default atlas or the specified one
// Try to group all textures that are drawn together into the same atlas
// Uses a handle system, which means you get a number that identifies this resource
// However this handle is only unique for its type, so don't use a handle from a SpriteSheet for a texture or a sound
// In practice this doesn't mean anything as you organize handles anyway
// This handle system is very fast - all items from a resource type (Sound, TextureRegion) are stored in a contiguous vector
// The handles are direct indices for lookups in those vectors O(1)
// All methods return handle::null if there are errors!
// ................................................................................

enum class AtlasID : int; // User implemented - per default all textures will be loaded into atlas 0

namespace magique
{
    enum class handle : uint32_t
    {
        null = UINT32_MAX // The null handle - identifies invalid handles
    };

    //----------------- Textures -----------------//

    // Loads the whole image as texture into the given atlas
    // scale    - controls the final dimensions of the resulting texture
    handle RegisterTexture(const Asset& asset, AtlasID atlas = {}, float scale = 1);

    // Tries to load a .png file as sprite sheet
    // Starts at (0,0) top left and then tries to split the image into frames row by row with the given dimensions
    // 'scale' allows to scale the resulting texture (rounded down)
    // Note: Combined width of all frames must not exceed MAGIQUE_TEXTURE_ATLAS_SIZE!
    handle RegisterSpriteSheet(const Asset& asset, int width, int height, AtlasID atlas = {}, float scale = 1);

    // Sames as RegisterSpriteSheet but allows to specify an offset from the top left and the amount of frames to load
    // Useful for loading part of a bigger sprite sheet - supports line breaks
    handle RegisterSpriteSheetEx(const Asset& asset, int width, int height, int frames, int offX, int offY,
                                 AtlasID atlas = {}, float scale = 1);

    // Register a sprite sheet out of single images
    // Useful if you have textures as separate images instead of a single SpriteSheet
    // Use with iterateDirectory()
    handle RegisterSpriteSheetVec(const std::vector<const Asset*>& assets, AtlasID atlas = {}, float scale = 1);

    //----------------- Audio -----------------//

    // Registers a sound file - can be any raylib supported file type (.mp3, .wav)
    handle RegisterSound(const Asset& asset);

    // Register a music file (streamed audio) - can be any raylib supported type (.mp3)
    // Everything above 10s should be loaded as music (and compressed with .mp3) instead of sound!
    handle RegisterMusic(const Asset& asset);

    // Tries to load a playlist from the given assets - they all have to be supported raylib music types (.mp3, .wav, .ogg, ...)
    handle RegisterPlaylist(const std::vector<const Asset*>& assets);

    //----------------- Tile Exports -----------------//
    // Note: Generally you have multiple TileMaps, but only 1 TileSet and 1 TileSheet!
    // Note: A TilSheet has its own atlas - you can access it manually as well and get a TextureRegion back

    // Registers a tilemap from an export file - Supported: ".tsx" (Tiled),
    // Supports loading multiple layers - all layers must have same dimensions!
    handle RegisterTileMap(const Asset& asset);

    // Registers a tileset - defines the details of all tiles in a project
    handle RegisterTileSet(const Asset& asset);

    // Registers a tile sheet from a single ".png" file
    // IMPORTANT: Make sure the TileSheet has exact dimensions and no padding!
    // 'textureSize'    - specify the width and height of each source tile
    // 'scale'          - allows to scale the resulting texture (rounded down)
    handle RegisterTileSheet(const Asset& asset, int tileSize, float scale = 1);

    // Same as 'RegisterTileSheet()' but takes a list of pictures
    // All assets provided must have the same dimensions and be pictures
    // This is useful if you have split images instead of a single TileSheet - Use with iterateDirectory()
    handle RegisterTileSheet(const std::vector<const Asset*>& assets, int tileSize, float scale = 1);

    //----------------- GET -----------------//

    // Returns the texture identified by this handle
    TextureRegion GetTexture(handle handle);

    // Returns the sprite sheet identified by this handle
    SpriteSheet GetSpriteSheet(handle handle);

    // Returns the sound identified by this handle
    Sound& GetSound(handle handle);

    // Returns the tilemap identified by this handle
    TileMap& GetTileMap(handle handle);

    // Returns the tileset identified by this handle
    TileSet& GetTileSet(handle handle);

    // Returns the TileSheet identified by this handle
    TileSheet& GetTileSheet(handle handle);

    // Returns the music identified by this handle
    Music& GetMusic(handle handle);

    // Returns the playlist identified by this handle
    Playlist& GetPlaylist(handle handle);

    //----------------- DIRECT GET -----------------//
    // Note: These methods assume you registered the handle with RegisterHandle() (see assets/HandleRegistry.h for info)
    // They call GetHandle() internally thus skipping the manual call

    TileMap& GetTileMap(HandleID id);
    TileMap& GetTileMap(uint32_t hash);

    TileSheet& GetTileSheet(HandleID handle);
    TileSheet& GetTileSheet(uint32_t hash);

    TileSet& GetTileSet(HandleID handle);
    TileSet& GetTileSet(uint32_t hash);

} // namespace magique


#endif //MAGIQUE_ASSETMANAGER_H