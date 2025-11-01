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
        const std::string& getRandomLine() const;
        std::string& getRandomLine();

        // Returns the lines vector
        const std::vector<std::string>& getTextLines() const;
        std::vector<std::string>& getTextLines();

        // Adds the given line
        // Moves the string
        void addLine(std::string line);
        void addLine(const char* line);

        // Returns true if a line with the given text is present
        bool hasLine(const char* line) const;

    private:
        TextLines(const Asset& asset, char delimiter);
        std::vector<std::string> lines;
        friend TextLines ImportText(Asset, char);
    };
} // namespace magique


#endif //MAGEQUEST_TEXTLINES_H
