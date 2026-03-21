// SPDX-License-Identifier: zlib-acknowledgement
#include <enchantum/enchantum.hpp>

#include <magique/gamedev/Localization.h>
#include <magique/util/Logging.h>
#include <magique/util/Datastructures.h>

namespace magique
{
    struct LocalizationData final
    {
        HashMap<Language, LocalizedLanguage> languageMapping; // Maps language to (keywords, translation)
        Language current = Language::EN;                      // Current language
    };

    inline LocalizationData DATA{};

    const char* Localize(const char* keyword)
    {
        if (keyword == nullptr) [[unlikely]]
        {
            return "";
        }
        const auto it = DATA.languageMapping.find(DATA.current);
        if (it != DATA.languageMapping.end()) [[likely]]
        {
            const auto& table = it->second.translations;
            const auto tableIt = table.find(keyword);
            if (tableIt != table.end())
            {
                return tableIt->second.c_str();
            }
        }
        return keyword;
    }

    std::string_view Localize(const std::string_view& key) { return Localize(key.data()); }

    void LocalizationAdd(const LocalizedLanguage& lang)
    {
        if (lang.language != Language::None)
        {
            LOG_WARNING("Cannot add localization language: Invalid language");
            return;
        }
        DATA.languageMapping[lang.language] = lang;
    }

    void LocalizationAdd(LocalizedLanguage&& lang)
    {
        if (lang.language == Language::None)
        {
            LOG_WARNING("Cannot add localization language: Invalid language");
            return;
        }
        DATA.languageMapping[lang.language] = std::move(lang);
    }

    void LocalizationSetLanguage(Language language)
    {
        if (language == Language::None)
        {
            LOG_WARNING("Cannot set localization language to None");
            return;
        }
        DATA.current = language;
    }

    Language LocalizationGetLanguage() { return DATA.current; }

    Language LocalizationParseLanguage(std::string_view langCode)
    {
        if (langCode.size() != 2)
            return Language::None;

        for (auto val : enchantum::values<Language>)
        {
            auto string = enchantum::to_string(val);
            if (std::tolower(langCode[0]) == std::tolower(string[0]) &&
                std::tolower(langCode[1]) == std::tolower(string[1]))
            {
                return val;
            }
        }
        return Language::None;
    }

    void LocalizationValidate(Language base) {}

    // {
    //     struct LanguageInfo final
    //     {
    //         int emptyKeywords = 0;
    //         int missingKeywords = 0;
    //         int newKeywords = 0;
    //     };
    //
    //     auto asdf = DATA.languageMapping;
    //     HashMap<const char*, LanguageInfo> languageInfos;
    //     if (!DATA.languageMapping.contains(referenceLanguage))
    //     {
    //         LOG_ERROR("Validation reference language %s does not exist!", referenceLanguage);
    //         return;
    //     }
    //
    //     StringHashMap<std::string> initialTable; // Table the others are compared with
    //     const auto& referenceTable = DATA.languageMapping[referenceLanguage];
    //
    //     for (const auto& languagePair : DATA.languageMapping)
    //     {
    //         auto& info = languageInfos[languagePair.first.c_str()];
    //         const auto& table = languagePair.second;
    //         for (const auto& keywordPair : table)
    //         {
    //             if (keywordPair.second.empty())
    //             {
    //                 ++info.emptyKeywords;
    //             }
    //         }
    //
    //         if (&languagePair.second == &referenceTable)
    //         {
    //             continue; // Skip - don't need to compare table to itself
    //         }
    //
    //         // Consistency check
    //         for (const auto& referencePair : referenceTable) // Check reference against current
    //         {
    //             if (!table.contains(referencePair.first))
    //             {
    //                 ++info.missingKeywords;
    //             }
    //         }
    //
    //         for (const auto& keywordPair : table) // Check current against reference
    //         {
    //             if (!referenceTable.contains(keywordPair.first))
    //             {
    //                 ++info.newKeywords;
    //             }
    //         }
    //     }
    //
    //     LOG_INFO("Localization Validation against: %s", referenceLanguage);
    //     for (const auto& infoPair : languageInfos)
    //     {
    //         const auto infoData = infoPair.second;
    //         if (strcmp(infoPair.first, referenceLanguage) == 0)
    //         {
    //             const auto* msg = "    Language: %s | Empty Translations: %d | Reference\n";
    //             printf(msg, infoPair.first, infoData.emptyKeywords, infoData.newKeywords, infoData.missingKeywords);
    //         }
    //         else
    //         {
    //             const auto* msg =
    //                 "    Language: %s | Empty Translations: %d | New Keywords: %d | Missing Keywords: %d\n";
    //             printf(msg, infoPair.first, infoData.emptyKeywords, infoData.newKeywords, infoData.missingKeywords);
    //         }
    //     }
    // }

} // namespace magique
