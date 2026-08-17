#ifndef MAGIQUE_LOCALIZATION_DEMO_H
#define MAGIQUE_LOCALIZATION_DEMO_H

#include <raylib/raylib.h>
#include <magique/magique.hpp>

using namespace magique;

struct LocalizationDemo final : Game
{
    void onStartup(AssetLoader& loader) override
    {
        const auto loadLocalization = [](AssetPack& assets)
        {
            LocalizationAdd(ImportMTF(assets["english.mtf"]));
            LocalizationAdd(ImportMTF(assets["german.mtf"]));
        };
        loader.registerTask(loadLocalization, MAIN_THREAD);
        LocalizationSetLanguage(Language::EN);
    }

    void onLoadingFinished() override
    {
        LocalizationAdd("jam", "Marmelade", Language::DE);
        LocalizationValidate(Language::DE);
    }

    void updateGame(GameState gameState) override
    {
        if (IsKeyPressed(KEY_SPACE)) // Toggle between the languages
        {
            if (LocalizationGetLanguage() == Language::DE)
                LocalizationSetLanguage(Language::EN);
            else
                LocalizationSetLanguage(Language::DE);
        }
    }

    void onDrawGame(GameState gameState, Camera2D& camera2D) override
    {
        const auto* msg = Localize("greeting");
        const auto text = TextFormat("Current Language: %s", enchantum::to_string(LocalizationGetLanguage()).data());
        DrawText(text, 50, 50, 25, BLACK);
        DrawText(msg, 50, 100, 25, BLACK);
    }
};

#endif //MAGIQUE_LOCALIZATION_DEMO_H
