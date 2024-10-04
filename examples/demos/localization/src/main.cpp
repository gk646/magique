#include <magique/assets/AssetPacker.h>
#include "LocalizationDemo.h"

int main()
{
    CompileImage("../res");
    LocalizationDemo demo{};
    return demo.run();
}