#ifndef MAGEQUEST_TEXTLINES_H
#define MAGEQUEST_TEXTLINES_H

#include <string>
#include <vector>
#include <magique/fwd.hpp>

//===============================================
// TextLines
//===============================================
// ................................................................................
// Holds all the lines of a file as separate strings in a vector
// ................................................................................

namespace magique
{
    struct TextLines final
    {
        TextLines() = default;
        TextLines(const std::string& buff, char delimiter = '\n');

        // Returns a random lines or nullptr if empty
        const std::string* getRandomLine() const;

        // Returns the lines vector
        const std::vector<std::string>& getLines() const;
        std::vector<std::string>& getLines();

        // Adds the given line
        void addLine(std::string_view line);

        // Returns true if a line with the given text is present
        bool hasLine(std::string_view line) const;

    private:
        std::vector<std::string> lines;
        std::string full;
    };
} // namespace magique


#endif //MAGEQUEST_TEXTLINES_H
