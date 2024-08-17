#ifndef LOADWRAPPERS_H
#define LOADWRAPPERS_H

#include <magique/assets/types/Asset.h>
#include <magique/internal/Macros.h>

namespace magique::internal
{
    inline bool AssetBaseCheck(const Asset& asset)
    {
        return asset.path != nullptr && asset.data != nullptr && asset.size > 0;
    }

    inline Image LoadImage(const Asset& asset)
    {
        ASSERT(AssetBaseCheck(asset), "Failed asset check");

        const auto* ext = GetFileExtension(asset.path);

        ASSERT(ext != nullptr, "No valid extension");

        auto img = LoadImageFromMemory(ext, (unsigned char*)asset.data, asset.size);

        ASSERT(img.data != nullptr, "No image data loaded");

        return img;
    }

} // namespace magique::internal

#endif //LOADWRAPPERS_H