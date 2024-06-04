#ifndef MAGIQUE_ASSETMANAGER_H
#define MAGIQUE_ASSETMANAGER_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Asset Management Module
//-----------------------------------------------
// .....................................................................
// Per default all methods are made to load from memory to work with the asset image
// Textures are automatically stitched into a atlas
// Try to group all textures that are drawn together into the same atlas
// Uses a handle system, which means you get a number that identifies this resoure
// However this handle is only unique for its type, so dont use a handle from a sprite sheet method for a texture or sound
//
// So in most cases its just a number lookup into a pre allocated dense map!
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

    // IMPORTANT: All assets are named with their path from the asset content root
    // Example :
    // The texture:       resources/textures/player.png
    // Compile Image:     assets::CompileImage("../resources");
    // While loading:     RegisterTexture(assets.getAsset("textures/player.png");

    //----------------- Loading -----------------//

    // Loads the whole texture into the given atlas
    // Failure: returns false
    handle RegisterTexture(const Asset& asset, AtlasType atlas = DEFAULT);

    // Tries to load a .png file as sprite sheet
    // Starts at the given offset and then tries to split the image into frames row by row with the given dimensions
    // Failure: returns false
    handle RegisterSpritesheet(const Asset& asset, int width, int height, AtlasType atlas = DEFAULT);

    // Tries to load a .png file as sprite sheet
    // Sames as RegisterSpritesheet but allows to specify an offset from the top left and the amount of frames to load
    // Goes from left to right
    // Failure: returns false
    handle RegisterSpritesheetEx(const Asset& asset, int width, int height, AtlasType atlas, int frames, int offX = 0,
                                 int offY = 0);

    //----------------- Getting -----------------//

    // Returns a texture identified by its name from the content root
    TextureRegion GetTextureRegion(handle handle);

    // Returns a spritesheet identified by its name from the content root
    SpriteSheet GetSpriteSheet(handle handle);

    // Returns a sound identified by its name from the content root
    Sound& GetSound(handle handle);

} // namespace magique


#endif //MAGIQUE_ASSETMANAGER_H