// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_H
#define MAGIQUE_ASSET_H

#include <magique/internal/Macros.h>

//===============================================
// Asset
//===============================================
// ................................................................................
// Every resources loaded into the game is treated as an asset.
// At startup the asset pack specified in Game::run() is loaded
// Single assets become accessible by adding loading tasks that operate on the asset container
// ................................................................................

namespace magique
{
    struct Asset final
    {
        // Returns the path of this asset relative to the root directory the image was compiled from
        const char* getPath() const;

        // Returns the data pointer of this asset
        const char* getData() const;
        const unsigned char* getUData() const;

        // Returns the valid size of the data pointer
        int getSize() const;

        // Checks if the assets extension matches the given one e.g '.png' myTexture.png
        // Includes the dot
        bool hasExtension(const char* extension) const;

        // True if the asset path starts with the given prefix
        bool startsWith(const char* prefix) const;

        // True if the asset path ends with the given prefix
        bool endsWith(const char* suffix) const;

        // True if the asset path contains the given sequence anywhere
        bool contains(const char* str) const;

        // Returns true if this asset is valid and can be used
        bool isValid() const;

        //================= STRING GETTERS =================//
        // IMPORTANT: The string returned by these methods is only valid until any of these methods are called again

        // Returns the direct file name without the asset - This means all characters after the last separator ("/")
        // Failure: returns nullptr if the asset's path is empty or no filename can be found
        //      extension - include file extension or not (".png",".wav", ...)
        [[nodiscard]] const char* getFileName(bool extension = true) const;

        // Returns the file extension of the asset - This means all characters from the last dot to the end e.g. '.png'
        // Failure: returns nullptr if the asset's path is empty or has no extension
        [[nodiscard]] const char* getExtension() const;

    private:
        M_MAKE_PUB()
        const char* path;                       // Full path of the file - from the compile root
        int size;                               // File size
        const char* data;                       // File data
        inline static char stringBuffer[128]{}; // Shared string buffer for string returns
    };

} // namespace magique
#endif //MAGIQUE_ASSET_H