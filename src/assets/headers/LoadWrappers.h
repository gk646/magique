#ifndef LOADWRAPPERS_H
#define LOADWRAPPERS_H

#include <magique/assets/types/Asset.h>
#include <magique/internal/Macros.h>

namespace magique
{
    inline bool AssetBaseCheck(const Asset& asset)
    {
        MAGIQUE_ASSERT(asset.path != nullptr && asset.data != nullptr, "Internal error");
        return asset.path != nullptr && asset.data != nullptr && asset.size > 0;
    }

    inline bool IsSupportedImageFormat(const char* fileType)
    {
        if (fileType == nullptr)
            return false;
        return ((false)
#if defined(SUPPORT_FILEFORMAT_PNG)
                || (strcmp(fileType, ".png") == 0) || (strcmp(fileType, ".PNG") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_BMP)
                || (strcmp(fileType, ".bmp") == 0) || (strcmp(fileType, ".BMP") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_TGA)
                || (strcmp(fileType, ".tga") == 0) || (strcmp(fileType, ".TGA") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_JPG)
                || (strcmp(fileType, ".jpg") == 0) || (strcmp(fileType, ".jpeg") == 0) ||
                (strcmp(fileType, ".JPG") == 0) || (strcmp(fileType, ".JPEG") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_GIF)
                || (strcmp(fileType, ".gif") == 0) || (strcmp(fileType, ".GIF") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_PIC)
                || (strcmp(fileType, ".pic") == 0) || (strcmp(fileType, ".PIC") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_PNM)
                || (strcmp(fileType, ".ppm") == 0) || (strcmp(fileType, ".pgm") == 0) ||
                (strcmp(fileType, ".PPM") == 0) || (strcmp(fileType, ".PGM") == 0)
#endif
#if defined(SUPPORT_FILEFORMAT_PSD)
                || (strcmp(fileType, ".psd") == 0) || (strcmp(fileType, ".PSD") == 0)
#endif
        );
    }

    inline Image LoadImage(const Asset& asset)
    {
        const auto* ext = asset.getExtension();
        MAGIQUE_ASSERT(IsSupportedImageFormat(ext), "No valid extension");
        const auto img = LoadImageFromMemory(ext, (unsigned char*)asset.data, asset.size);
        MAGIQUE_ASSERT(img.data != nullptr, "No image data loaded");
        return img;
    }


} // namespace magique

#endif //LOADWRAPPERS_H