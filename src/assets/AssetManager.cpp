#include <magique/assets/AssetManager.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/assets/types/TileMap.h>
#include <magique/util/Macros.h>
#include <magique/core/Types.h>

#include <raylib/raylib.h>

#include "core/globals/AssetManager.h"
#include "core/globals/TextureAtlas.h"


namespace magique
{
    bool ImageCheck(Image& img, const Asset& asset, const AtlasType at)
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
        img = LoadImageFromMemory(ext, (unsigned char*)asset.data, asset.size);
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
        if (!ImageCheck(image, asset, at))
            return handle::null;

        M_ASSERT(image.width >= width && image.height >= height, "Image is smaller than a single frame");

        auto& atlas = global::TEXTURE_ATLASES[at];

        const SpriteSheet sheet = atlas.addSpritesheet(image, 1, width, height, 0, 0);

        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterSpritesheetEx(const Asset& asset, int width, int height, AtlasType at, int frames, int offX,
                                 int offY)
    {
        Image image;
        if (!ImageCheck(image, asset, at))
            return handle::null;

        M_ASSERT(image.width >= width && image.height >= height, "Image is smaller than a single frame");
        M_ASSERT(offX < image.width && offY < image.height, "Offset is outside image bounds");

        auto& atlas = global::TEXTURE_ATLASES[at];

        const auto sheet = atlas.addSpritesheet(image, frames, width, height, offX, offY);

        return global::ASSET_MANAGER.addResource(sheet);
    }

    handle RegisterTexture(const Asset& asset, const AtlasType at)
    {
        Image image;
        if (!ImageCheck(image, asset, at))
            return handle::null;

        auto& atlas = global::TEXTURE_ATLASES[at];

        const auto region = atlas.addTexture(image);

        return global::ASSET_MANAGER.addResource(region);
    }


    handle RegisterTileMap(const Asset& asset)
    {
        auto tileMap = TileMap(asset);
        return global::ASSET_MANAGER.addResource(std::move(tileMap));
    }

    handle RegisterTileSet(const Asset& asset) { return handle::null; }


    handle RegisterTileSheet(const Asset& asset, int width, int height, AtlasType atlas, float scale)
    {

        return handle::null;
    }

    handle RegisterTileSheet(std::vector<const Asset&>& assets, int width, int height, AtlasType atlas, float scale)
    {
        return handle::null;
    }

    //----------------- GET -----------------//

    Sound& GetSound(const handle handle) { return global::ASSET_MANAGER.getResource<Sound>(handle); }

    TextureRegion GetTextureRegion(const handle handle)
    {
        return global::ASSET_MANAGER.getResource<TextureRegion>(handle);
    }

    SpriteSheet GetSpriteSheet(const handle handle) { return global::ASSET_MANAGER.getResource<SpriteSheet>(handle); }


} // namespace magique