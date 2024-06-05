#include <magique/assets/AssetManager.h>
#include <magique/assets/container/AssetContainer.h>

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


    handle RegisterSpritesheet(const Asset& asset, int width, int height, AtlasType at)
    {
        Image image;
        if (!ValidityCheck(image, asset, at))
            return handle::null;

        assert(image.width >= width && image.height >= height && "Image is smaller than a single frame");

        auto& atlas = TEXTURE_ATLASES[at];

        const auto sheet = atlas.addSpritesheet(image, 1, width, height, 0, 0);

        return ASSET_MANAGER.addResource(sheet);
    }


    handle RegisterSpritesheetEx(const Asset& asset, int width, int height, AtlasType at, int frames, int offX,
                                 int offY)
    {
        Image image;
        if (!ValidityCheck(image, asset, at))
            return handle::null;

        assert(image.width >= width && image.height >= height && "Image is smaller than a single frame");
        assert(offX < image.width && offY < image.height && "Offset is outside image bounds");

        auto& atlas = TEXTURE_ATLASES[at];

        const auto sheet = atlas.addSpritesheet(image, frames, width, height, offX, offY);

        return ASSET_MANAGER.addResource(sheet);
    }


    handle RegisterTexture(const Asset& asset, const AtlasType at)
    {
        Image image;
        if (!ValidityCheck(image, asset, at))
            return handle::null;

        auto& atlas = TEXTURE_ATLASES[at];

        const auto region = atlas.addTexture(image);

        return ASSET_MANAGER.addResource(region);
    }


    Sound& GetSound(const handle handle) { return ASSET_MANAGER.getResource<Sound>(handle); }

    TextureRegion GetTextureRegion(const handle handle) { return ASSET_MANAGER.getResource<TextureRegion>(handle); }

    SpriteSheet GetSpriteSheet(const handle handle) { return ASSET_MANAGER.getResource<SpriteSheet>(handle); }


} // namespace magique