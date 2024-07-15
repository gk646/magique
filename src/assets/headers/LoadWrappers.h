#ifndef LOADWRAPPERS_H
#define LOADWRAPPERS_H

#include <magique/assets/container/AssetContainer.h>
#include <magique/internal/Macros.h>
#include <raylib/raylib.h>

namespace magique::internal
{
    inline bool AssetBaseCheck(const Asset& asset)
    {
        return asset.path != nullptr && asset.data != nullptr && asset.size > 0;
    }

    inline Image LoadImage(const Asset& asset)
    {
        M_ASSERT(AssetBaseCheck(asset), "Failed asset check");

        auto ext = GetFileExtension(asset.path);

        M_ASSERT(ext != nullptr, "No valid extension");

        auto img = LoadImageFromMemory(ext, (unsigned char*)asset.data, asset.size);

        M_ASSERT(img.data != nullptr, "No image data loaded");

        return img;
    }


} // namespace magique::internal

#endif //LOADWRAPPERS_H