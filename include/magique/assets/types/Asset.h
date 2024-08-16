#ifndef MAGIQUE_ASSET_H
#define MAGIQUE_ASSET_H

//-----------------------------------------------
// Asset
//-----------------------------------------------
// ................................................................................
// Every resources thats loaded into the game is an asset first.
// By using the AssetLoad
// ................................................................................

namespace magique
{
    struct Asset final
    {
        const char* path; // Full path of the file - from the compile root
        int size;         // File size
        const char* data; // File data

        // Checks if the assets extension matches the given one e.g '.png' myTexture.png
        // Includes the dot
        bool hasExtension(const char* extension) const;

        // True if the asset path starts with the given prefix
        bool startsWith(const char* prefix) const;

        // True if the asset path ends with the given prefix
        bool endsWith(const char* suffix) const;

        // True if the asset path contains the given sequence anywhere
        bool contains(const char* str) const;

        //----------------- GETTERS -----------------//
        // IMPORTANT: The string returned by these methods is only correct until any of these methods are called again

        // Returns a the direct file name without the asset - This means all characters after the last separator ("/")
        // Failure: returns nullptr if the asset's path is empty or no filename can be found
        //      extension - include file extension or not (".png",".wav", ...)
        [[nodiscard]] const char* getFileName(bool extension = true) const;

        // Returns the file extension of the asset - This means all characters after the last dot (".")
        // Failure: returns nullptr if the asset's path is empty or has no extension
        [[nodiscard]] const char* getExtension() const;

    private:
        inline static char stringBuffer[64]{}; // Shared string buffer for string returns
    };

}
#endif //MAGIQUE_ASSET_H