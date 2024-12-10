// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_UTIL_H
#define MAGIQUE_ASSET_UTIL_H

#include <cstring>

#include <raylib/config.h>
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

    inline bool IsSupportedSoundFormat(const char* fileType)
    {
        if (fileType == nullptr)
            return false;
        return ((false)
#if defined(SUPPORT_FILEFORMAT_WAV)
                || ((strcmp(fileType, ".wav") == 0) || (strcmp(fileType, ".WAV") == 0))
#endif
#if defined(SUPPORT_FILEFORMAT_OGG)
                || ((strcmp(fileType, ".ogg") == 0) || (strcmp(fileType, ".OGG") == 0))
#endif
#if defined(SUPPORT_FILEFORMAT_MP3)
                || ((strcmp(fileType, ".mp3") == 0) || (strcmp(fileType, ".MP3") == 0))
#endif
#if defined(SUPPORT_FILEFORMAT_QOA)
                || ((strcmp(fileType, ".qoa") == 0) || (strcmp(fileType, ".QOA") == 0))
#endif
#if defined(SUPPORT_FILEFORMAT_FLAC)
                || ((strcmp(fileType, ".flac") == 0) || (strcmp(fileType, ".FLAC") == 0))
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

#endif //MAGIQUE_ASSET_UTIL_H