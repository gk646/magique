#ifndef MAGIQUE_ASSETMANAGER_H
#define MAGIQUE_ASSETMANAGER_H

#include <vector>
#include <magique/fwd.hpp>

//-----------------------------------------------
// Asset Management Module
//-----------------------------------------------
// .....................................................................
// Per default all methods are made to load from memory to work with the asset image
// Textures are automatically stitched into the default or a specified one
// Try to group all textures that are drawn together into the same atlas
// Uses a handle system, which means you get a number that identifies this resoure
// However this handle is only unique for its type, so dont use a handle from a spritesheet for a texture or a sound
// In practice this doesnt mean anything as you organize handles anyway
// This handle syste is very fast - all items from a resource type (Sound, TextureRegion) are stored in a contigous vector
// The handles are direct indices for lookups in those vectors O(1)
// All method return handle::null if there are errors!
// .....................................................................

namespace magique
{
    enum class handle : uint32_t
    {
        null = UINT32_MAX // The null handle - identifies invalid handles
    };

    enum AtlasType : uint8_t
    {
        DEFAULT, // Default atlas
        WORLD,
        USER_INTERFACE, // All UI related textures
        ENTITIES,       // All entity textures
        CUSTOM_1,       // User defineable
        CUSTOM_2,       // User defineable
    };

    //----------------- Textures -----------------//

    // Loads the whole texture into the given atlas
    handle RegisterTexture(const Asset& asset, AtlasType atlas = DEFAULT);

    // Tries to load a .png file as sprite sheet
    // Starts at (0,0) topleft and then tries to split the image into frames row by row with the given dimensions
    // Note: Meant for entity animations - combined width of all frames must not exceed 4096 pixels!
    handle RegisterSpritesheet(const Asset& asset, int width, int height, AtlasType atlas = DEFAULT);

    // Sames as RegisterSpritesheet but allows to specify an offset from the top left and the amount of frames to load
    // Useful for loading part of a bigger sprite sheet - supports line breaks
    // Note: Meant for entity animations - combined width of all frames must not exceed 4096 pixels!
    handle RegisterSpritesheetEx(const Asset& asset, int width, int height, AtlasType atlas, int frames, int offX = 0,
                                 int offY = 0);

    // Register a sprite sheet out of single images
    // Useful if you the have textures as separate images instead of a single spritesheet
    // Use with iterateDirectory()
    handle RegisterSpritesheetVec(const std::vector<const Asset&>& asset, AtlasType atlas);

    //----------------- Sound -----------------//

    // Registers a sound file - can be any raylib supported file type (.mp3, .wav)
    handle RegisterSound(const Asset& asset);


    //----------------- TileMaps -----------------//
    // Note: Generally you have multiple TileMaps, but only 1 TileSet and 1 TileSheet!
    // Note: magique only supports setting 1 TileSheet at a time -  Use SetTileSet() in the Core module
    // Note: A Tilsheet has its own atlas - you can access it manually aswell and get a TextureRegion back

    // Registers a "Tiled" (the level editor) map export from a ".tsx" file!
    // Supports loading multiple layers - all layers must have same dimensions!
    handle RegisterTileMap(const Asset& asset);

    // Registers a tileset - defines the details all tiles in a project
    handle RegisterTileSet(const Asset& asset);

    // Registers a tile sheet of textures from a single ".png" file
    // width and height specify the dimensions of each tile - scale allows to scale the resulting texture (rounded down)
    handle RegisterTileSheet(const Asset& asset, int width, int height, float scale = 1.0F);

    // Registers a tile sheet from single textures - ".png"
    // All assets provided must have the same dimensions and be pictures
    // width and height specify the dimensions of each tile - scale allows to scale the resulting texture (rounded down)
    handle RegisterTileSheet(std::vector<const Asset&>& assets, int width, int height, float scale = 1.0F);

    //----------------- Getting -----------------//

    // Returns the texture identified by this handle
    TextureRegion GetTextureRegion(handle handle);

    SpriteSheet GetSpriteSheet(handle handle);

    Sound& GetSound(handle handle);

    TileMap& GetTileMap(handle handle);

    TileSet& GetTileSet(handle handle);

    TileSheet& GetTileSheet(handle handle);
} // namespace magique


#endif //MAGIQUE_ASSETMANAGER_H