#ifndef MAGIQUE_ASSET_PACKER_H
#define MAGIQUE_ASSET_PACKER_H

#include <cstdint>

#include "magique/fwd.hpp"

//-----------------------------------------------
// Assetpacker Module
//-----------------------------------------------

// .....................................................................
// You do not need to call the 'LoadAssetImage' function if you use the game template
// The compile image can be helpful when iterating to always have the latest version of your resources compiled
// Just add it before you start the game but remove it for release builds!

namespace magique
{
    //----------------- IN GAME -----------------//

    // Loads a compiled asset image from disk into the given container
    // Failure: Returns false
    bool LoadAssetImage(const char* path, AssetContainer& assets, uint64_t encryptionKey = 0);


    //----------------- UTILS -----------------//

    // Compiles an asset image from all files inside the given directory as root
    // Saves it to disk with the given name relative to the current working directory
    // Failure: Returns false
    bool CompileImage(const char* directory, const char* fileName = "data.bin", uint64_t encryptionKey = 0);

} // namespace magique::assets
#endif