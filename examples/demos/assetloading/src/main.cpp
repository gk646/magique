
#include "AssetLoadingDemo.h"

int main()
{
    std::string_view pathToCompile = "../res";      // Relative to the executable that lies in the build folder
    std::string_view imageName = "MyImageName.bin"; // Path/Name of the assetpack - can be anything
    EncryptionKey encryptionKey = 0;                // Arbitrary encryption key - 0 means no encryption

    // Compiles the asset image - only rewrites the image if changes are detected
    // Automatically compressed to save size
    AssetPackCompile(pathToCompile, imageName, encryptionKey);

    Demo game{};

    // The passed pack is automatically loaded and can be access in onStartup()
    return game.run(imageName, encryptionKey);
}
