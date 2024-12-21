#define _CRT_SECURE_NO_WARNINGS
#include <string>

#include <magique/assets/FileImports.h>
#include <magique/util/Logging.h>

namespace magique
{
    CSVImport FileImportCSV(Asset asset, char delimiter, bool firstRowNames)
    {
        CSVImport csv{};
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

        auto parseLine = [&](int& pos, char* line)
        {
            int lineColumns = 0;
            while (pos < csv.len)
            {
                if (line[pos] == delimiter || line[pos] == '\n' || line[pos] == '\0')
                {
                    MAGIQUE_ASSERT(lineColumns < MAGIQUE_MAX_CSV_COLUMNS, "Too many cells in CSV file");
                    lineColumns++;
                    if (line[pos] == '\n' || line[pos] == '\0')
                    {
                        line[pos] = '\0'; // 0-terminate the data here so we can return a string from the start
                        pos++;
                        return lineColumns;
                    }
                    line[pos] = '\0'; // 0-terminate the data here so we can return a string from the start
                }
                pos++;
            }
            return -1;
        };

        int pos = 0;
        int rows = 0;
        int columns = 0;

        if (firstRowNames)
        {
            while (pos < csv.len)
            {
                if (content[pos] == delimiter)
                {
                    content[pos] = '\0';
                    columns++;
                }
                else if (content[pos] == '\n' || content[pos] == '\0')
                {
                    content[pos] = '\0';
                    rows++;
                    pos++;
                    columns++;
                    break;
                }
                pos++;
            }
            csv.nameLen = pos;
        }

        while (pos < csv.len)
        {
            const int lineColumns = parseLine(pos, content);
            if (columns != 0 && columns != lineColumns)
            {
                LOG_ERROR("CSV file is not well-formed: Row %d has inconsistent amount of columns", rows);
                return csv;
            }
            columns = lineColumns;
            rows++;
        }

        csv.columns = columns;
        csv.rows = rows;

        return csv;
    }
} // namespace magique