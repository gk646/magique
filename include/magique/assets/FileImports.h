// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_CSVREADER_H
#define MAGIQUE_CSVREADER_H

#include <magique/assets/types/Asset.h>
#include <magique/assets/container/CSVImport.h>

//===============================================
// File Imports
//===============================================
// ................................................................................
// This module allows to import various files types to a representation that can easily be accessed.
// ................................................................................

namespace magique
{

    // Imports the given asset as CSV file - if specified the first row will be loaded as column names
    // Supported filetypes: ".csv"
    // Failure: Returns an empty import
    CSVImport FileImportCSV(Asset asset, char delimiter = ';', bool firstRowNames = true);

} // namespace magique



#endif //MAGIQUE_CSVREADER_H