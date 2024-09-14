#include "WizardQuest.h"

#include <magique/assets/AssetPacker.h>
#include <magique/gamedev/ShareCode.h>

int main()
{
    ShareCodeFormat format;
    format.addTextProperty("name");
    format.addIntegerProperty(255, "age");
    format.addIntegerProperty(300, "height");
    format.addFloatProperty(55, "foot width");

    auto data = format.getFormatData();

    data.setData("name", "gk646");
    data.setData("age", 22);
    data.setData(2, 178);
    data.setData(3, 2.2F);

    auto shareCode = format.getShareCodeString(data);

    return 0;
    magique::CompileImage("../res");
    WizardQuest game{};
    return game.run();
}