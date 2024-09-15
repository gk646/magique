#include "WizardQuest.h"

#include <chrono>
#include <magique/assets/AssetPacker.h>
#include <magique/gamedev/ShareCode.h>

int main()
{
    ShareCodeFormat format;
    format.addTextProperty("name");
    format.addIntegerProperty(255, "age");
    format.addIntegerProperty(300, "height");
    format.addFloatProperty("foot width");

    auto exportData = format.getFormatData();

    exportData.setData("name", "gk646");
    exportData.setData("age", 22);
    exportData.setData(2, 178);
    exportData.setData(3, 2.2F);

    auto shareCode = format.getShareCode(exportData);

    printf("Generated Share Code: %s\n", shareCode.getCode());

    auto code2 = ShareCode(shareCode.getCode());

    auto importData = format.getShareCodeData(shareCode);
    for (int i = 0; i < exportData.getSize(); ++i)
    {
        auto exp = exportData.getData(i);
        auto imp = importData.getData(i);

        if (exp.isInteger())
        {
            MAGIQUE_ASSERT(exp.getInteger() == imp.getInteger(), "Not matching integer");
        }
        else if (exp.isFloat())
        {
            MAGIQUE_ASSERT(exp.getFloat() == imp.getFloat(), "Not matching float");
        }
        else if (exp.isString())
        {
            MAGIQUE_ASSERT(strcmp(exp.getString(), imp.getString()) == 0, "Not matching string");
        }
    }

    return 0;
    magique::CompileImage("../res");
    WizardQuest game{};
    return game.run();
}