#include <magique/assets/AssetManager.h>
#include <magique/assets/container/AssetContainer.h>

#include <cxutil/cxstring.h>

#include "core/CoreData.h"


namespace magique
{
    bool ValidityCheck(Image& img, const Asset& asset, const AtlasType at)
    {
        if (at > CUSTOM_2)
        {
            LOG_ERROR("Trying to load texture into invalid atlas");
            return false;
        }

        const auto ext = GetFileExtension(asset.name);
        if (ext == nullptr)
        {
            LOG_WARNING("Loading texture with bad extension: %s", ext);
            return false;
        }
        img = LoadImageFromMemory(ext, asset.getData(), asset.size);
        if (img.data == nullptr)
        {
            LOG_ERROR("Error loading the texture: %s", asset.name);
            UnloadImage(img);
            return false;
        }
        return true;
    }


    handle RegisterSpritesheet(const Asset& asset, int width, int height, AtlasType atlas)
    {
        return handle::null;
    }


    handle RegisterSpritesheetEx(const Asset& asset, int width, int height, AtlasType atlas, int frames, int offX,
                                 int offY)
    {

        return handle::null;
    }


    handle RegisterTexture(const Asset& asset, const AtlasType at)
    {
        Image image;
        if (!ValidityCheck(image, asset, at))
            return handle::null;

        auto& atlas = TEXTURE_ATLASES[at];

        const auto region = atlas.addImage(image);

        const auto handle = ASSET_MANAGER.addResource(region);
        assert(handle != handle::null, "Error registering");

        return handle;
    }


    Sound& GetSound(const handle handle) { return ASSET_MANAGER.getResource<Sound>(handle); }

    TextureRegion GetTextureRegion(const handle handle) { return ASSET_MANAGER.getResource<TextureRegion>(handle); }


} // namespace magique