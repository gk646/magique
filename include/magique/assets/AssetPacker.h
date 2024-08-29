#ifndef MAGIQUE_ASSET_PACKER_H
#define MAGIQUE_ASSET_PACKER_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Assetpacker Module
//-----------------------------------------------
// ................................................................................
// The asset management in magique expects an all resources in a single folder!
// This folder then has to be compiled into an asset image - a single file which can simply be distributed
// Just add it before you start the game but remove it for release builds!
// Note: You do NOT need to call the 'LoadAssetImage' function if you use the game template
// Note: To force a recompile just delete the index.magique in the root of the resource directory
//        -> this is necessary e.g. when your changes do not change the filesize of any file
// .....................................................................

namespace magique
{
    // Loads a compiled asset image from disk into the given container
    // Failure: Returns false
    bool LoadAssetImage(const char* path, AssetContainer& assets, uint64_t encryptionKey = 0);

    // Compiles an asset image from all files inside the given directory as root and saves it to disk (recursively)
    // Given name is relative to the current working directory - compression to make the image smaller
    // Note: Creates an index file at the root of the directory and skips recompiling image if no files have changed!
    // Failure: Returns false
    bool CompileImage(const char* dir, const char* name = "data.bin", uint64_t encryptionKey = 0, bool compress = false);

} // namespace magique
#endif