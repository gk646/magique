#include <magique/gamedev/Localization.h>
#include <magique/internal/Macros.h>
#include <magique/assets/types/Asset.h>
#include <magique/util/Logging.h>

#include "internal/datastructures/StringHashMap.h"

namespace magique
{
    struct LocalizationData final
    {
        StringHashMap<StringHashMap<std::string>> languageMapping; // Maps language to (keywords, translation)
        std::string language;                                      // Current language
        static constexpr char separator = ':';
    };

    static LocalizationData DATA;

    const char* Localize(const char* keyword)
    {
        if (DATA.language.empty())
            return keyword;
        const auto it = DATA.languageMapping.find(DATA.language);
        if (it != DATA.languageMapping.end()) // Contains keywords
        {
            const auto& table = it->second;
            const auto tableIt = table.find(keyword);
            if (tableIt != table.end())
            {
                return tableIt->second.c_str();
            }
            return keyword;
        }
        return keyword;
    }

    void LoadLocalization(const Asset& asset)
    {
        MAGIQUE_ASSERT(strcmp(asset.getExtension(), ".mtf") == 0, "Passed asset needs to be a .mtf file");
        const char* workPtr = asset.data;

        const char* language = nullptr;
        int translations = 0;

        std::string cacheString;
        int i = 0;
        while (workPtr + i != nullptr)
        {
            if (workPtr[i] == LocalizationData::separator) // Parse the language
            {
                const int languageStart = i;
                while (workPtr + i != nullptr && workPtr[i] != '\n')
                {
                    ++i;
                }
                const int languageLength = i - languageStart;
                cacheString.assign(workPtr + languageStart, languageLength); // Contains the language
                if (languageLength == 0)
                {
                    const auto* msg = "Localization file %s has invalid language:%s";
                    LOG_ERROR(msg, asset.getFileName(true), cacheString.c_str());
                    return;
                }
                if (DATA.languageMapping.contains(cacheString))
                {
                    const auto* msg = "Localization for language %s already exists from file:%s";
                    LOG_ERROR(msg, cacheString.c_str(), asset.getFileName(true));
                    return;
                }

                auto& table = DATA.languageMapping[cacheString];



            }
            ++i;
        }


        const auto* msg = "Successfully loaded Localization: Language: %s | keywords: %d";
        LOG_INFO(msg, language, translations);
    }

    void AddLocalization(const char* keyword, const char* language, const char* translation) {}

    void SetLocalizationLanguage(const char* language) {}

    const std::string& GetCurrentLanguage() {}

    void ValidateLocalizations() {}

} // namespace magique