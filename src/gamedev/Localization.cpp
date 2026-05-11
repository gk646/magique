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

    void LocalizationValidate(Language base)
    {
        struct LanguageInfo final
        {
            int emptyValues = 0;
            int missingKeys = 0;
            int newKeywords = 0;
            bool present = false;
        };

        if (!DATA.languageMapping.contains(base))
        {
            LOG_ERROR("Localization validation base language does not exist: %s", enchantum::to_string(base).data());
            return;
        }

        const auto& referenceTable = DATA.languageMapping[base];
        HashMap<Language, LanguageInfo> languageInfos;

        for (const auto& [_, language] : DATA.languageMapping)
        {
            if (language.language == base || language.translations.empty())
                continue;

            auto& info = languageInfos[language.language];
            info.present = true;

            for (const auto& [key, translation] : language.translations)
            {
                if (translation.empty())
                {
                    ++info.emptyValues;
                }
            }

            // Consistency check - Check reference against current
            for (const auto& [key, translation] : referenceTable.translations)
            {
                if (!language.translations.contains(key))
                {
                    ++info.missingKeys;
                }
            }

            for (const auto& [key, translation] : language.translations)
            {
                if (!referenceTable.translations.contains(key))
                {
                    ++info.newKeywords;
                }
            }
        }

        LOG_INFO("Localization Validation against: %s", enchantum::to_string(base).data());
        for (const auto& [lang, info] : languageInfos)
        {
            if (!info.present) // Not used
                continue;
            const auto* msg = "    Language: %s | Empty Translations: %d | New Keywords: %d | Missing Keywords: %d\n";
            printf(msg, enchantum::to_string(lang).data(), info.emptyValues, info.newKeywords, info.missingKeys);
        }
    }

} // namespace magique
