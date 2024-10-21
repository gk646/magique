#include <magique/assets/AssetPacker.h>
#include "LocalizationDemo.h"

int main()
{
    CompileImage("../res"); // Dont forget to compile the asset image
    LocalizationDemo demo{};
    return demo.run();
}