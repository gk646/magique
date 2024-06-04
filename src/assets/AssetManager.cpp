#include <magique/assets/AssetManager.h>
#include <magique/assets/container/AssetContainer.h>
#include <magique/util/Hash.h>

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




    bool RegisterSpritesheet(const Asset& asset, int width, int height, AtlasType atlas) { return true; }


    bool RegisterSpritesheetEx(const Asset& asset, int width, int height, AtlasType atlas, int frames, int offX,
                               int offY)
    {

        return true;
    }


    bool RegisterTexture(const Asset& asset, const AtlasType at)
    {
        Image image;
        if (!ValidityCheck(image, asset, at))
            return false;

        auto& atlas = TEXTURE_ATLASES[at];

        const auto region = atlas.addImage(image);

        const auto hash = util::HashString(asset.name, HASH_SALT);

        assert(!ASSET_MANAGER.getResourceMap<TextureRegion>().contains(hash),
               "Collision! You either registered a texture twice (with the same name) or suffered an unlucky hash "
               "collision. Use 'SetHashSalt' to change the hash function until no collision occur!");

        ASSET_MANAGER.addResource(hash, region);
        return true;
    }


    Sound& GetSound(const uint32_t hash) { return ASSET_MANAGER.getResource<Sound>(hash); }

    TextureRegion GetTextureRegion(const uint32_t hash)
    {
        return ASSET_MANAGER.getResource<TextureRegion>(hash);
    }


} // namespace magique