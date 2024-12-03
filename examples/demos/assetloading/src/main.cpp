
#include "AssetLoadingDemo.h"

#include <magique/assets/AssetPacker.h>

int main()
{
    // Which directory to recursively compile into an asset image
    const char* pathToCompile = "../res";      // Relative to the executable that lies in the build folder
    const char* imageName = "MyImageName.bin"; // Path/Name of the image - can be anything
    uint64_t encryptionKey = 0;                // Arbitrary encryption key - 0 means no encryption
    bool compressImage = false; // Make the resulting image smaller - is automatically detected and decompressed

    // Compiles the asset image - only recompiles the image if changes are detected
    CompileAssetImage(pathToCompile, imageName, encryptionKey, compressImage);
    Demo game{};

    const char* configPath = "config.cfg"; // Path/Name of the game config
    return game.run(imageName, configPath, encryptionKey);
}