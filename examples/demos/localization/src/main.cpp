#include "LocalizationDemo.h"

int main()
{
    // Dont forget to compile the asset image
    AssetPackCompile("../res");
    LocalizationDemo demo{};
    return demo.run();
}