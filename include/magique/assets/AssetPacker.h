#ifndef MAGIQUE_ASSET_PACKER_H
#define MAGIQUE_ASSET_PACKER_H

#include <magique/fwd.hpp>
#include <magique/core/Types.h>

//===============================================
// Assetpacker Module
//===============================================
// ................................................................................
// The asset management in magique expects an all resources in a single folder!
// This folder then has to be compiled into an asset image - a single file which can simply be distributed
// Just add it before you start the game but remove it for release builds!
// Note: To force a recompile just delete the index.magique in the root of the resource directory
//        -> this is necessary e.g. when your changes do not change the file size of any file
// IMPORTANT: You do NOT need to call the 'LoadAssetImage' function if you use the game template (e.g. subclass Game)
// .....................................................................

namespace magique
{
    // Loads a compiled asset image from disk into the given container
    // Note: compression is automatically detected and decompressed
    //      - key: key used to encrypt the data (0 = no encryption)
    // Failure: Returns false
    bool LoadAssetImage(AssetContainer& assets, const char* path = "data.bin", uint64_t key = 0);

    // Compiles an asset image from all files inside the given directory as root and saves it to disk (recursively)
    // Given name is relative to the current working directory - enable compression to make the image smaller
    //      - key: key used to encrypt the data (0 = no encryption)
    // Note: Creates an index file at the root of the directory and skips recompiling if no files have changed!
    // Failure: Returns false
    bool CompileAssetImage(const char* dir, const char* path = "data.bin", uint64_t key = 0, bool compress = false);

    // Returns the checksum (hash) of the image (using MD5)
    // Note: This should not be used in the final shipped game - The intended workflow:
    //       - 1. Generate the checksum of the final asset image
    //       - 2. Print it out with: Checksum::print(); and copy the string
    //       - 3. Declare a variable in the main.cpp file with the copied string: Checksum{"copied-string"} CHECK_SUM;
    //       - 4. Use this now to verify the integrity of the asset image (detect tampering or corrupted download)
    //          -  if( !ValidateAssetImage(CHECK_SUM)) ... LOG_ERROR("Corrupted asset image");
    // IMPORTANT: AssetImage can
    Checksum GetAssetImageChecksum(const char* path = "data.bin");

    // Returns true if the asset image at the specified path has the same checksum
    bool ValidateAssetImage(Checksum checksum, const char* path = "data.bin");

} // namespace magique

#endif