#ifndef MAGIQUE_ASSETMANAGER_H
#define MAGIQUE_ASSETMANAGER_H

#include <magique/fwd.hpp>
#include <magique/util/Hash.h>

//-----------------------------------------------
// Asset Management Module
//-----------------------------------------------
// .....................................................................
// Per default all methods are made to load from memory to work with the asset image
// Textures are automatically stitched into a atlas
// Try to group all textures that are drawn together into the same atlas
// All textures are registered with their path from the asset content root
// Uses compile time string hashing!
// So in most cases its just a number lookup into a pre allocated dense map!
// .....................................................................

namespace magique
{
    // Customizable parameter to handle collisions - Influences all hashing
    // https://en.wikipedia.org/wiki/Salt_(cryptography)
    inline constexpr int HASH_SALT = 0;

    enum AtlasType : uint8_t
    {
        DEFAULT,        // Default atlas
        USER_INTERFACE, // All UI related textures
        ENTITIES,       // All entity textures
        CUSTOM_1,       // User defineable
        CUSTOM_2,       // User defineable
    };

    // IMPORTANT: All textures are registered with their path from the asset content root
    // Example :
    // The texture:       resources/textures/player.png
    // Compile Image:     assets::CompileImage("../resources");
    // While loading:     RegisterTexture(assets.getAsset("textures/player.png");
    // Draw Texture:      DrawRegion(GetTexture("textures/player.png");

    //----------------- Loading -----------------//

    // Loads the whole texture into the given atlas
    // Failure: returns false
    bool RegisterTexture(const Asset& asset, AtlasType atlas = DEFAULT);

    // Tries to load a .png file as sprite sheet
    // Starts at the given offset and then tries to split the image into frames row by row with the given dimensions
    // Failure: returns false
    bool RegisterSpritesheet(const Asset& asset, int width, int height, AtlasType atlas = DEFAULT);

    // Tries to load a .png file as sprite sheet
    // Sames as RegisterSpritesheet but allows to specify an offset from the top left and the amount of frames to load
    // Goes from left to right
    // Failure: returns false
    bool RegisterSpritesheetEx(const Asset& asset, int width, int height, AtlasType atlas, int frames, int offX = 0,
                               int offY = 0);

    //----------------- Getting -----------------//

    // IMPORTANT: You have to use this macro to get the hash - sadly you can pass constexpr strings
#define H(msg) magique::util::HashStringEval(msg, HASH_SALT)
    // Example: GetTexture(H("textures/player.png"));

    // Returns a texture identified by its name from the content root
    TextureRegion GetTextureRegion(uint32_t hash);

    // Returns a spritesheet identified by its name from the content root
    SpriteSheet GetSpriteSheet(uint32_t hash);

    // Returns a sound identified by its name from the content root
    Sound& GetSound(uint32_t hash);

} // namespace magique


#endif //MAGIQUE_ASSETMANAGER_H