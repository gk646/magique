// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_ASSET_PACKER_H
#define MAGIQUE_ASSET_PACKER_H

#include <magique/fwd.hpp>
#include <magique/core/Types.h>

//===============================================
// AssetPacker
//===============================================
// ................................................................................
// The asset management in magique expects all resources (used ingame) in a single folder!
// This folder is then packed into a single file - this makes it easy to distribute and handle
// This has to be done MANUALLY before calling Game.run() - so probably in main.cpp
// Note: In a production build you should NOT call AssetPackCompile() !
// Note: A new pack is only written if any changes were made to any file
// IMPORTANT: You do NOT need to call AssetPackLoad() if you use the game template (call in Game.run() automatically)
// .....................................................................

namespace magique
{
    // Compiles an asset pack from all files inside the given directory as root and saves it to disk (recursively)
    // Given name is relative to the current working directory - enable compression to make the pack smaller
    //      - key: key used to encrypt the data (0 = no encryption)
    // Note: Only writes a new pack if any changes to files happened
    // Failure: Returns false
    bool AssetPackCompile(const char* dir, const char* name = "data.bin", uint64_t key = 0, bool compress = true);

    // Loads a compiled asset pack from disk into the given container
    // Note: compression is automatically detected and decompressed
    //      - key: key used to encrypt the data (0 = no encryption)
    // Failure: Returns false
    bool AssetPackLoad(AssetPack& assets, const char* path = "data.bin", uint64_t key = 0);

    // Returns the checksum (hash) of the pack (using MD5)
    // Note: This should not be used in a production build
    // The intended workflow:
    //       - 1. Generate the checksum of the final asset pack
    //       - 2. Copy the checksum into the code: Checksum{"copied-string"} CHECK_SUM;
    //       - 3. Use this now to verify the integrity of the asset pack (detect tampering or corrupted download)
    //          -  if( !AssetPackValidate(CHECK_SUM)) ... LOG_ERROR("Corrupted asset pack");
    Checksum AssetPackChecksum(const char* path = "data.bin");

    // Returns true if the asset pack at the specified path has the same checksum
    bool AssetPackValidate(Checksum checksum, const char* path = "data.bin");

} // namespace magique

#endif
