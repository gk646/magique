#include "WizardQuest.h"

#include <chrono>
#include <iostream>
#include <magique/assets/AssetPacker.h>
#include <magique/gamedev/ShareCode.h>

int main()
{
    ShareCodeFormat format;
    format.addTextProperty("name");
    format.addIntegerProperty(255, "age");
    format.addIntegerProperty(300, "height");
    format.addFloatProperty( "foot width");

    auto exportData = format.getFormatData();

    exportData.setData("name", "gk646");
    exportData.setData("age", 22);
    exportData.setData(2, 178);
    exportData.setData(3, 2.2F);

    auto startGen = std::chrono::high_resolution_clock::now();

    auto shareCode = format.getShareCode(exportData);

    auto endGen = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> generationTime = endGen - startGen;
    std::cout << "Share code generation took: " << generationTime.count() << " seconds\n";

    printf("Generated Share Code: %s\n", shareCode.getCode());

    auto startImport = std::chrono::high_resolution_clock::now();

    auto importData = format.importFromString(shareCode);

    auto endImport = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> importTime = endImport - startImport;
    std::cout << "Share code import took: " << importTime.count() << " seconds\n";


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