// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/gamedev/Localization.h>

#include <magique/assets/types/Asset.h>
#include <magique/util/Logging.h>
#include <magique/util/Datastructures.h>

namespace magique
{
    struct LocalizationData final
    {
        StringHashMap<StringHashMap<std::string>> languageMapping; // Maps language to (keywords, translation)
        std::string language;                                      // Current language
        static constexpr char separator = ':';
        static constexpr char lineComment = '#';

        static bool SkipAfter(const int fileSize, int& index, const char* data, const char end)
        {
            while (data[index] != end)
            {
                ++index;
                if (index >= fileSize)
                    return true;
            }
            ++index;
            return false;
        }
    };

    inline LocalizationData DATA{};

    const char* Localize(const char* keyword)
    {
        if (DATA.language.empty())
            return keyword;
        const auto it = DATA.languageMapping.find(DATA.language);
        if (it != DATA.languageMapping.end()) // Language is found
        {
            const auto& table = it->second;
            const auto tableIt = table.find(keyword);
            if (tableIt != table.end()) // Keyword is found
            {
                return tableIt->second.c_str();
            }
            return keyword;
        }
        LOG_WARNING("No localization for the given language: %s", DATA.language.c_str());
        return keyword;
    }

    void LoadLocalization(const Asset& asset)
    {
        MAGIQUE_ASSERT(strcmp(asset.getExtension(), ".mtf") == 0, "Passed asset needs to be a .mtf file");
        MAGIQUE_ASSERT(asset.data != nullptr || asset.size == 0, "Asset contains no data!");

        const int fileSize = asset.size;
        const char* workPtr = asset.data;
        const char* language = nullptr;
        int keywordPairs = 0;

        std::string cacheString(20, '\n');
        int i = 0;

        if (strncmp(workPtr, "language", 8) != 0)
        {
            LOG_ERROR("Localization file has invalid format: see Localization.h");
            return;
        }

        // First line comment
        if (workPtr[0] == LocalizationData::lineComment)
        {
            const auto isEnd = LocalizationData::SkipAfter(fileSize, i, workPtr, '\n');
            if (isEnd) // If file only contains a single comment
            {
                LOG_ERROR("Localization file contains no keywords:%s", asset.getFileName(true));
                return;
            }
        }

        // Find language keyword
        if (LocalizationData::SkipAfter(fileSize, i, workPtr, ':')) // Reach end of file
        {
            LOG_ERROR("Unable to parse language for localization file:%s", asset.getFileName(true));
            return;
        }

        // Find language code
        const int languageStart = i;
        if (LocalizationData::SkipAfter(fileSize, i, workPtr, '\n')) // Reach end of file
        {
            LOG_ERROR("Unable to parse language for localization file:%s", asset.getFileName(true));
            return;
        }
        const int languageLength = -1 + i - languageStart;
        if (languageLength == 0)
        {
            const auto* msg = "Localization file %s has invalid language:%s";
            LOG_ERROR(msg, asset.getFileName(true), cacheString.c_str());
            return;
        }
        cacheString.assign(workPtr + languageStart, languageLength); // Contains the language

        if (cacheString.empty()) // No language
        {
            LOG_ERROR("Unable to parse language for localization file:%s", asset.getFileName(true));
            return;
        }

        if (DATA.languageMapping.contains(cacheString))
        {
            const auto* msg = "Localization for language %s already exists from file:%s";
            LOG_ERROR(msg, cacheString.c_str(), asset.getFileName(true));
            return;
        }

        auto& table = DATA.languageMapping[cacheString]; // The language specific table mapping keyword-translation
        language = DATA.languageMapping.find(cacheString)->first.c_str();

        // Parsing of the keyword-translation pairs
        while (i < fileSize)
        {
            if (workPtr[i] == LocalizationData::lineComment) // Line comment can only be at start
            {
                const auto isEnd = LocalizationData::SkipAfter(fileSize, i, workPtr, '\n');
                if (isEnd) // If file only contains a single comment
                    break;
            }

            // Parse the keyword
            const int keywordStart = i;
            if (LocalizationData::SkipAfter(fileSize, i, workPtr, LocalizationData::separator)) // Reached end
            {
                LOG_ERROR("Reached end while parsing a keyword in localization file:%s", asset.getFileName(true));
                return;
            }

            const int keywordLen = -1 + i - keywordStart;
            if (keywordLen == 0)
            {
                LOG_WARNING("Localization file %s contains empty keyword!", asset.getFileName(true));
                continue;
            }
            cacheString.assign(workPtr + keywordStart, keywordLen); // Now contains the keyword

            // Parse the translation
            auto& translation = table[cacheString];

            const int translationStart = i;
            const auto reachedEnd = LocalizationData::SkipAfter(fileSize, i, workPtr, '\n');
            const int translationLen = -1 + i - translationStart;

            if (translationLen == 0) // empty
            {
                LOG_WARNING("Localization file %s contains empty translation!", asset.getFileName(true));
                translation.clear();
                continue;
            }
            cacheString.assign(workPtr + translationStart, translationLen); // Now contains the translation
            translation = cacheString;
            ++keywordPairs;
            if (reachedEnd)
            {
                break;
            }
        }

        const auto* msg = "Loaded Localization: Language: %s | keywords: %d";
        LOG_INFO(msg, language, keywordPairs);
    }

    void AddLocalization(const char* keyword, const char* language, const char* translation)
    {
        auto& table = DATA.languageMapping[language];
        table[keyword] = translation;
    }

    void SetLocalizationLanguage(const char* language) { DATA.language = language; }

    const std::string& GetLocalizationLanguage() { return DATA.language; }

    void ValidateLocalizations(const char* referenceLanguage)
    {
        struct LanguageInfo final
        {
            int emptyKeywords = 0;
            int missingKeywords = 0;
            int newKeywords = 0;
        };

        auto asdf = DATA.languageMapping;
        HashMap<const char*, LanguageInfo> languageInfos;
        if (!DATA.languageMapping.contains(referenceLanguage))
        {
            LOG_ERROR("Validation reference language %s does not exist!", referenceLanguage);
            return;
        }

        StringHashMap<std::string> initialTable; // Table the others are compared with
        const auto& referenceTable = DATA.languageMapping[referenceLanguage];

        for (const auto& languagePair : DATA.languageMapping)
        {
            auto& info = languageInfos[languagePair.first.c_str()];
            const auto& table = languagePair.second;
            for (const auto& keywordPair : table)
            {
                if (keywordPair.second.empty())
                {
                    ++info.emptyKeywords;
                }
            }

            if (&languagePair.second == &referenceTable)
            {
                continue; // Skip - don't need to compare table to itself
            }

            // Consistency check
            for (const auto& referencePair : referenceTable) // Check reference against current
            {
                if (!table.contains(referencePair.first))
                {
                    ++info.missingKeywords;
                }
            }

            for (const auto& keywordPair : table) // Check current against reference
            {
                if (!referenceTable.contains(keywordPair.first))
                {
                    ++info.newKeywords;
                }
            }
        }

        LOG_INFO("Localization Validation against: %s", referenceLanguage);
        for (const auto& infoPair : languageInfos)
        {
            const auto infoData = infoPair.second;
            if (strcmp(infoPair.first, referenceLanguage) == 0)
            {
                const auto* msg = "    Language: %s | Empty Translations: %d | Reference\n";
                printf(msg, infoPair.first, infoData.emptyKeywords, infoData.newKeywords, infoData.missingKeywords);
            }
            else
            {
                const auto* msg =
                    "    Language: %s | Empty Translations: %d | New Keywords: %d | Missing Keywords: %d\n";
                printf(msg, infoPair.first, infoData.emptyKeywords, infoData.newKeywords, infoData.missingKeywords);
            }
        }
    }

} // namespace magique