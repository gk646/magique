// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_PACKER_H
#define MAGIQUE_ASSET_PACKER_H

#include <magique/fwd.hpp>
#include <magique/core/Types.h>

//===============================================
// Assetpacker Module
//===============================================
// ................................................................................
// The asset management in magique expects an all resources in a single folder!
// This folder then has to be compiled into an asset pack - a single file which can simply be distributed
// Just add it before you start the game but remove it for release builds!
// Note: To force a recompile just delete the index.magique in the root of the resource directory
//        -> this is necessary e.g. when your changes do not change the file size of any file
// IMPORTANT: You do NOT need to call the 'AssetPackLoad' function if you use the game template (e.g. subclass Game)
// .....................................................................

namespace magique
{
    // Loads a compiled asset pack from disk into the given container
    // Note: compression is automatically detected and decompressed
    //      - key: key used to encrypt the data (0 = no encryption)
    // Failure: Returns false
    bool AssetPackLoad(AssetContainer& assets, const char* path = "data.bin", uint64_t key = 0);

    // Compiles an asset pack from all files inside the given directory as root and saves it to disk (recursively)
    // Given name is relative to the current working directory - enable compression to make the pack smaller
    //      - key: key used to encrypt the data (0 = no encryption)
    // Note: Creates an index file in the working dir and skips recompiling if no files (sizes) have changed!
    // Failure: Returns false
    bool AssetPackCompile(const char* dir, const char* name = "data.bin", uint64_t key = 0, bool compress = false);

    // Returns the checksum (hash) of the pack (using MD5)
    // Note: This should not be used in the final shipped game - The intended workflow:
    //       - 1. Generate the checksum of the final asset pack
    //       - 2. Print it out with: Checksum::print(); and copy the string
    //       - 3. Declare a variable in the main.cpp file with the copied string: Checksum{"copied-string"} CHECK_SUM;
    //       - 4. Use this now to verify the integrity of the asset pack (detect tampering or corrupted download)
    //          -  if( !AssetPackValidate(CHECK_SUM)) ... LOG_ERROR("Corrupted asset pack");
    Checksum AssetPackChecksum(const char* path = "data.bin");

    // Returns true if the asset pack at the specified path has the same checksum
    bool AssetPackValidate(Checksum checksum, const char* path = "data.bin");

} // namespace magique

#endif
