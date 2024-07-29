#ifndef MAGIQUE_LOCALIZATION_H
#define MAGIQUE_LOCALIZATION_H

#include <magique/fwd.hpp>

//-----------------------------------------------
// Localization Module
//-----------------------------------------------
// .....................................................................
// This module allows to dynamically change language and get the correct strings
// It uses the POEdit tool and the '.PO' format for exporting and importing translations
//
// 1. Installation: Download for you PC: https://github.com/vslavik/poedit -> https://poedit.net/
// 2. Import:       Create a new project
//                  -> Select import from source
//                  -> add the root of your source folder as parse root
//                  -> Under the keywords tab add the custom keyword "Localize" so its finds the function
//                  -> start the import
// 3. Translate:    Now you should see all strings that are wrapped in the Localize function
//                  You can then translate it into the target language. Note that you create a new file for each target language
// 4. Import:       Save the files inside your main resource folder
//                  -> when the game loads iterate the localization directory call "LoadLocalization" for each .pot file
// .....................................................................

namespace magique
{

    //
    const char* Localize(const char*);


    void LoadLocalization(const Asset& asset);

} // namespace magique

#endif //MAGIQUE_LOCALIZATION_H