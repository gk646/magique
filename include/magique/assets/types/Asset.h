// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_H
#define MAGIQUE_ASSET_H

#include <string_view>
#include <magique/fwd.hpp>

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

        // True if the asset path starts with the given prefix
        bool startsWith(const char* prefix) const;

        // True if the asset path ends with the given prefix e.g. '.png"
        bool endsWith(const char* suffix) const;

        // True if the asset path contains the given sequence anywhere
        bool contains(const char* str) const;

        // Returns true if this asset is valid and can be used
        bool isValid() const;

        // Returns the direct file name without the asset - This means all characters after the last separator ("/")
        // Failure: returns path if the asset's path is empty or no filename can be found
        //      extension - include file extension or not (".png",".wav", ...)
        [[nodiscard]] std::string_view getFileName(bool extension = true) const;

        // Returns the file extension of the asset - includes all character form last dot until the end e.g. '.png'
        // Failure: returns empty if the asset's path is empty or has no extension
        [[nodiscard]] std::string_view getExtension() const;

    private:
        M_MAKE_PUB()
        std::string_view path;                  // Full path of the file - from the compile root
        std::string_view data;                  // File data
    };

} // namespace magique
#endif // MAGIQUE_ASSET_H
