#ifndef MAGIQUE_LOCALIZATION_DEMO_H
#define MAGIQUE_LOCALIZATION_DEMO_H

#include <magique/magique.hpp>

struct LocalizationDemo final : Game
{
    void onStartup(AssetLoader& loader) override
    {
        const auto loadLocalization = [](AssetPack& assets)
        {
            // Import the base file - contains the keywords
            LocalizationAdd(ImportGettextBase(assets["base.pot"]));

            assets.forEachIn("",
                             [](Asset asset)
                             {
                                 // Only import ".po"
                                 if (asset.getExtension() != ".po")
                                     return;
                                 // Import any translations
                                 LocalizationAdd(ImportGettext(asset));
                             });
        };

        loader.registerTask(loadLocalization);
        LocalizationSetLanguage(Language::EN);
    }

    void onLoadingFinished() override
    {
        LocalizationAdd(Language::DE, "jam", "Marmelade");
        LocalizationValidate(Language::DE);
    }

    void onUpdateGame(GameState gameState) override
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
        std::string_view msg = Localize("greeting");
        std::string_view text = TextFormat("Current Language: %s", EnumToString(LocalizationGetLanguage()).data());
        DrawText(text.data(), 50, 50, 25, BLACK);
        DrawText(msg.data(), 50, 100, 25, BLACK);
    }
};

#endif //MAGIQUE_LOCALIZATION_DEMO_H
