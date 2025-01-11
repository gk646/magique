#define _CRT_SECURE_NO_WARNINGS
#include <cstring>

#include <magique/assets/FileImports.h>
#include <magique/util/Logging.h>

namespace magique
{
    CSVImport FileImportCSV(Asset asset, const char delimiter, const bool firstRowNames)
    {
        CSVImport csv{};
        if (asset.getSize() == 0)
        {
            LOG_ERROR("Passed empty asset:%s", asset.getFileName());
            return csv;
        }

        if (strcmp(".csv", asset.getExtension()) != 0)
        {
            LOG_ERROR("Invalid extension for a .csv file: %s", asset.getExtension());
            return csv;
        }

        // The strategy here is to only make one allocation
        // This normally doesn't work as each string needs a 0-termination
        // But in the csv format each cell is terminated by the delimiter (or a newline)

        char* content = new char[asset.getSize() + 1];
        memcpy(content, asset.getData(), asset.getSize());
        content[asset.getSize()] = '\0';

        csv.data = content;
        csv.len = asset.getSize() + 1;

        auto parseLine = [&](int& pos, char* line, int row)
        {
            int lineColumns = 0;
            while (pos < csv.len)
            {
                if (line[pos] == delimiter)
                {
                    MAGIQUE_ASSERT(lineColumns < MAGIQUE_MAX_CSV_COLUMNS, "Too many cells in CSV file");
                    line[pos] = '\0'; // 0-terminate the data here so we can return a string from the start
                    lineColumns++;
                    if (content[pos + 1] == '\n' || content[pos + 1] == '\0')
                    {
                        LOG_WARNING("Detected trailing delimiter:%s | Row:%d", asset.getFileName(), row);
                        pos+=2; // Skip both the terminator and the end marker and
                        return lineColumns;
                    }
                }
                else if (line[pos] == '\n' || line[pos] == '\0')
                {
                    MAGIQUE_ASSERT(lineColumns < MAGIQUE_MAX_CSV_COLUMNS, "Too many cells in CSV file");
                    lineColumns++;
                    line[pos] = '\0';
                    pos++;
                    if (firstRowNames && row == 0)
                    {
                        csv.nameLen = pos;
                    }
                    return lineColumns;
                }
                pos++;
            }
            LOG_FATAL("Failed to parse CSV file: %s", asset.getFileName());
            return -1;
        };

        int pos = 0;
        int rows = 0;
        int columns = 0;

        while (pos < csv.len)
        {
            const int lineColumns = parseLine(pos, content, rows);
            if (columns != 0 && columns != lineColumns)
            {
                LOG_ERROR("CSV file is not well-formed: Row %d  amount of columns", rows);
                return csv;
            }
            columns = lineColumns;
            rows++;
        }

        csv.columns = columns;
        csv.rows = rows;
        if (static_cast<int>(strlen(csv.data)) + 1 == csv.len)
        {
            LOG_WARNING("Mismatch between used and specified delimiter detected, Used \"%c\"", delimiter);
        }
        return csv;
    }
} // namespace magique