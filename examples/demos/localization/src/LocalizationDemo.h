#ifndef MAGIQUE_LOCALIZATION_DEMO_H
#define MAGIQUE_LOCALIZATION_DEMO_H

#include <magique/assets/AssetLoader.h>
#include <magique/gamedev/Localization.h>
#include <magique/core/Game.h>
#include <raylib/raylib.h>

using namespace magique;

struct LocalizationDemo final : Game
{
    void onStartup(AssetLoader& loader) override
    {
        const auto loadLocalization = [](AssetContainer& assets)
        {
            LoadLocalization(assets.getAsset("english.mtf"));
            LoadLocalization(assets.getAsset("german.mtf"));
        };
        loader.registerTask(loadLocalization, MAIN_THREAD);
        SetLocalizationLanguage("EN");
    }

    void onLoadingFinished() override
    {
        AddLocalization("jam", "DE", "Marmelade");
        ValidateLocalizations("DE");
    }

    void updateGame(GameState gameState) override
    {
        if (IsKeyPressed(KEY_SPACE)) // Toggle between the languages
        {
            if (GetLocalizationLanguage() == "DE")
                SetLocalizationLanguage("EN");
            else
                SetLocalizationLanguage("DE");
        }
    }

    void drawGame(GameState gameState, Camera2D& camera2D) override
    {
        const auto* msg = Localize("greeting");
        const auto text = TextFormat("Current Language: %s", GetLocalizationLanguage().c_str());
        DrawText(text, 50, 50, 25, BLACK);
        DrawText(msg, 50, 100, 25, BLACK);
    }
};

#endif //MAGIQUE_LOCALIZATION_DEMO_H