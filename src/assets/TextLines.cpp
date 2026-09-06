#include <algorithm>

#include <magique/assets/types/TextLines.h>
#include <magique/util/Strings.h>

namespace magique
{
    TextLines::TextLines(std::string_view buff, const char delimiter) : lines(StringSplit(buff, delimiter)), full(buff)
    {
    }

    std::optional<std::string_view> TextLines::getRandomLine() const
    {
        if (lines.empty())
            return {};
        return lines[GetRandomValue(0, lines.size() - 1)];
    }

    const std::vector<std::string>& TextLines::getLines() const { return lines; }

    std::vector<std::string>& TextLines::getLines() { return lines; }

    void TextLines::addLine(std::string_view line) { lines.push_back(std::string{line}); }

    bool TextLines::hasLine(std::string_view line) const { return std::ranges::find(lines, line) != lines.end(); }

} // namespace magique
