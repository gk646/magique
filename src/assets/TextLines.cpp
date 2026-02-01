#include <algorithm>

#include <magique/assets/types/TextLines.h>

#include <magique/util/Math.h>
#include <magique/util/Strings.h>

namespace magique
{
    TextLines::TextLines(const std::string& buff, const char delimiter) : lines(SplitString(buff, delimiter)), full(buff)
    {
    }

    const std::string* TextLines::getRandomLine() const { return PickRandomElement(lines); }

    const std::vector<std::string>& TextLines::getLines() const { return lines; }

    std::vector<std::string>& TextLines::getLines() { return lines; }

    void TextLines::addLine(std::string_view line) { lines.push_back(std::string{line}); }

    bool TextLines::hasLine(std::string_view line) const { return std::ranges::find(lines, line) != lines.end(); }

} // namespace magique
