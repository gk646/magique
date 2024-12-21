#ifndef MAGIQUE_CSVIMPORT_H
#define MAGIQUE_CSVIMPORT_H

#include <magique/internal/InternalTypes.h>

//===============================================
// CSV Import
//===============================================
// ................................................................................
// This containers efficiently holds all data related to a .csv file.
// Internally it only uses a single copy of the file with one allocation.
// ................................................................................

namespace magique
{
    struct CSVImport final
    {
        // Returns the string value of the given cell
        const char* getCell(int row, int column) const;

        // Returns the column index idenfified by the given name
        // Failure: returns -1
        int getColumn(const char* column) const;

        // Returns the amount of columns this CSV file has (determined by the first line)
        int getColumns() const;

        // Returns the amount of rows this CSV file has
        int getRows() const;

        // Import is automatically cleaned up
        ~CSVImport();

    private:
        CSVImport() = default;
        const char* getCellImpl(int row, int column) const;
        const char* data = nullptr;
        int len = 0;
        int nameLen = -1;
        int columns = 0;
        int rows = 0;
        befriend(CSVImport FileImportCSV(Asset, char, bool));
    };
} // namespace magique

#endif //MAGIQUE_CSVIMPORT_H