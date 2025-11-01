#include <algorithm>

#include <raylib/raylib.h>
#include <magique/assets/types/TextLines.h>
#include <magique/util/Logging.h>
#include <magique/assets/types/Asset.h>

namespace magique
{
    const std::string& TextLines::getRandomLine() const
    {
        if (lines.empty())
        {
            LOG_ERROR("TextLines is empty");
        }
        return lines[GetRandomValue(0, (int)lines.size() - 1)];
    }

    std::string& TextLines::getRandomLine()
    {
        if (lines.empty())
        {
            LOG_ERROR("TextLines is empty");
        }
        return lines[GetRandomValue(0, (int)lines.size() - 1)];
    }

    const std::vector<std::string>& TextLines::getTextLines() const { return lines; }

    std::vector<std::string>& TextLines::getTextLines() { return lines; }

    void TextLines::addLine(std::string line) { lines.push_back(std::move(line)); }

    void TextLines::addLine(const char* line) { lines.push_back(line); }

    bool TextLines::hasLine(const char* line) const { return std::ranges::find(lines, line) != lines.end(); }

    TextLines::TextLines(const Asset& asset, char delimiter)
    {
        const char* start = asset.getData();

        uint64_t cursor = 0;
        uint64_t lastLine = 0;
        uint64_t end = asset.getSize();

        lines.reserve(32);
        while (cursor < end)
        {
            if (start[cursor] == delimiter)
            {
                lines.emplace_back(&start[lastLine], cursor - lastLine);
                lastLine = cursor + 1;
            }
            cursor++;
        }

        if (lastLine < end)
        {
            lines.emplace_back(&start[lastLine], end - lastLine);
        }
    }

} // namespace magique
