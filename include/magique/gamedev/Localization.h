// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_LOCALIZATION_H
#define MAGIQUE_LOCALIZATION_H

#include <magique/util/Datastructures.h>

//===============================================
// Localization Module
//===============================================
// .....................................................................
// This module allows to dynamically change language and get the correct strings based on a given keyword
// Each keyword is unique and identifies the translated strings across languages
//      -> keyword: hello  - English: Hello User! - German: Guten Tag Benutzer!
//      -> keyword: Good Morning  - English: Good Morning - German: Guten Morgen!
//
// You can either manually add localization for each language and keyword or load it from a file in assets/AssetImport.h
// Supported Formats: .po (GNU gettext format)
// Note: There is also a macro localize() that can be used
// .....................................................................

namespace magique
{
    struct LocalizedLanguage final
    {
        Language language = Language::None;
        StringHashMap<std::string> translations;
    };

    // Returns the localized string for the given keyword - uses the currently set language
    // Failure: if no language is set or doesn't exist or the keyword is not translated, the keyword itself is returned
    const char* Localize(const char* key);
    std::string_view Localize(const std::string_view& key);

    //============== LOAD ==============//

    // Adds the given language
    void LocalizationAdd(const LocalizedLanguage& lang);
    void LocalizationAdd(LocalizedLanguage&& lang);

    // Manually adds a translation of the keyword in the given language
    void LocalizationAdd(std::string_view key, std::string_view translation, Language lang);

    //============== UTIL ==============//

    // Sets the localization language - takes effect immediately and Localize() returns the translation for this language
    // Default: EN
    void LocalizationSetLanguage(Language lang);
    Language LocalizationGetLanguage();

    // Scans all registered languages and compares them to base language:
    //      - missing translation for keywords
    //      - missing keywords in other languages
    void LocalizationValidate(Language base = Language::EN);

    // Parses the language from a given ISO 639 code (e.g. en, es, de, it) - NOT case sensitive
    // Failure: Returns Language::None
    Language LocalizationParseLanguage(std::string_view langCode);

} // namespace magique

#endif // MAGIQUE_LOCALIZATION_H
