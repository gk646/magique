#ifndef MAGIQUE_LOCALIZATION_H
#define MAGIQUE_LOCALIZATION_H

#include <string>
#include <magique/fwd.hpp>

//-----------------------------------------------
// Localization Module
//-----------------------------------------------
// .....................................................................
// This module allows to dynamically change language and get the correct strings based on a given keyword
// Each keyword is unique and identifies the translated strings across languages
//      -> keyword: hello  - English: Hello User! - German: Guten Tag Benutzer!
// You can either manually add localization for each language and keyword or load it from a file
// Note: How the different languages are called is up to the user (e.g. country codes (DE, US) or something else)
//
// The magique translation file format (.mtf):
// language:{Your language code}
// {keyword}:{translation}
// {keyword}:{translation}
// ...
// Note: the keywords MUST not contain a semicolon (everything after the first semicolon is translation)
// Example (german.mtf):
// language:DE
// greeting:Herzlich Willkommen!
// goodbye:Auf Wiedersehen!
// .....................................................................

namespace magique
{

    // Returns the localized string for the given keyword
    // Failure: if no language is set or doesnt exist or the keyword is not translated, the keyword itself is returned
    const char* Localize(const char* keyword);

    //-------------- LOAD --------------//

    // Loads the given magique translation file (.mtf) and adds the specified localizations for that language
    void LoadLocalization(const Asset& asset);

    // Manually adds a localization of the keyword in the given language
    void AddLocalization(const char* keyword, const char* language, const char* translation);

    //-------------- UTIL --------------//

    // Sets the localization language - takes effect immediately and Localize() returns the translation for this language
    // Default: empty
    void SetLocalizationLanguage(const char* language);

    // Returns the current language
    // Failure: if no language is set returns empty string
    const std::string& GetCurrentLanguage();

    // Scans all registered localization and reports:
    //      - missing translation for keywords
    //      - missing keywords in some languages
    void ValidateLocalizations();

} // namespace magique

#endif //MAGIQUE_LOCALIZATION_H