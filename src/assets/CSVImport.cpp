#include <cstring>
#include <magique/assets/container/CSVImport.h>
#include <magique/util/Logging.h>

namespace magique
{

    int CSVImport::getColumn(const char* column) const
    {
        MAGIQUE_ASSERT(column != nullptr, "Passed nullptr");
        if (nameLen == -1)
        {
            LOG_ERROR("No cell names found");
            return -1;
        }

        if (strcmp(column, data) == 0)
            return 0;

        int pos = 0;
        int count = 0;
        while (pos < nameLen)
        {
            if (data[pos] == '\0')
            {
                pos++;
                count++;
                if (strcmp(column, data + pos) == 0)
                    return count;
                continue;
            }
            pos++;
        }
        return -1;
    }

    const char* CSVImport::getCell(const int row, const int column) const
    {
        return getCellImpl(row, column);
    }

    int CSVImport::getColumns() const { return columns; }

    int CSVImport::getRows() const { return rows; }

    bool CSVImport::hasColumnNames() const { return nameLen != -1; }

    CSVImport::~CSVImport()
    {
        delete[] data;
        data = nullptr;
    }

    const char* CSVImport::getCellImpl(const int row, const int column) const
    {
        MAGIQUE_ASSERT(row < rows && row >= 0, "Invalid row index");
        MAGIQUE_ASSERT(column < columns && column >= 0, "Invalid column index");
        if (row == 0 && column == 0)
            return data;
        int pos = 0;
        int parsedRow = 0;
        int parsedColumn = 0;
        while (pos < len)
        {
            const auto c = data[pos];
            if (c == '\0')
            {
                parsedColumn++;
            }
            if (parsedColumn == columns)
            {
                parsedRow++;
                parsedColumn = 0;
            }
            if (parsedRow == row && parsedColumn == column)
                return data + pos + 1;
            ++pos;
        }
        return nullptr;
    }


} // namespace magique