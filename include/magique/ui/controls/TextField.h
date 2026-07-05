// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_TEXTFIELD_H
#define MAGIQUE_TEXTFIELD_H

#include <magique/core/Engine.h>
#include <magique/ui/UIObject.h>
#include <magique/ui/UI.h>

//===============================================
// TextField + TextDisplay
//===============================================
// .....................................................................
// The textfield is useful to capture text input and allows editing text inside the field
// Controls are:
//      - Arrow keys        : move cursor
//      - Arrow keys + CTRL : move cursor one word (non whitespace)
//      - BACKSPACE/DEL     : delete in front/behind or selection if any
//      - CTRL + V          : Paste clipboard content at cursor
//      - CTRL + C          : Copy selection
//      - CTRL + X          : Copy selection then erase selection
//      - CTRL + A          : Select the whole text
//      - Mouse Drag        : Select text area - also works multiline
//
// The default behavior is to require the user to focus it by clicking it once - once focused it registers the input
// .....................................................................

namespace magique
{
    // Return true: signals enter action was consumed and should NOT be treated as newline
    // Return false: signals that enter action was not used and should be treated as newline
    using EnterFunc = std::function<bool(bool ctrl, bool shift, bool alt)>;

    struct TextField : UIObject
    {
        // Creates the textfield from absolute dimensions in the logical UI resolution
        // Optionally specify an anchor point the object is anchored to and a scaling mode
        TextField(Rect bounds, Anchor anchor = Anchor::NONE, Point insert = {}, ScalingMode scaling = ScalingMode::FULL);

        // Called everytime the textfield is focused and enter is pressed
        void setOnEnter(const EnterFunc& func);

        // Returns the current text of the textfield
        const std::string& getText() const;

        // Sets the textChanged flag to true
        std::string& getTextToModify();

        // Adds text with a newline appended
        void addLine(std::string_view line);

        // Removes text from the front until the first newline is found "\n"
        // Note: Useful for implementing chats or logs
        void popFirstLine();

        // Sets the hint - drawn on the field if empty
        void setHint(const char* hint);

        // Returns true if the text has changed since last time this method was called - resets the changed status
        bool pollTextHasChanged();

        // Sets if the cursor is shown or not and the blink delay in ticks
        void setCursorStatus(bool shown, int delay);

        // Returns true if the textfield is focused
        [[nodiscard]] bool getIsFocused() const;
        void setFocused(bool val = true);

        // Returns the total amount of lines (count of newlines '\n' + 1 (for first line))
        [[nodiscard]] int getLineCount() const;

        // Adjust the bounds such that the text fits inside
        // Keeps the top left the same
        void fitToText(float size = 8, const Font& font = EngineGetFont(), float spacing = 1, bool heightOnly = false);

    protected:
        // Same as ui/UIObject.h
        // Note: Text needs to be drawn manually
        void onDraw(const Rect& bounds) override { drawDefault(bounds); }

        // Same as ui/UIObject.h
        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                if (updateInputs() || getIsFocused())
                    LayeredInput::ConsumeKey();
        }

        // Updates the text with the current inputs for this tick and updates the focused state
        // Note: This should be called each update tick
        // Returns: true if any event OR text input has been made
        bool updateInputs(bool allowInput = true);

        // Draws the text and updates the selection
        void drawText(float size, Color txt, Color cursor, const Font& font = EngineGetFont(), float spacing = 1,
                      bool centered = true);

        // Draws the default graphical representation of this textfield
        void drawDefault(const Rect& bounds, float fontSize = 8);


    private:
        bool pollControls();
        bool updateControls();
        void updateSelection(float fSize, const Font& font, float spacing);
        void updateLineState();
        Point getCenteredTextPos(const Rect& bounds, float fontSize) const;

        // Returns if change occurred
        bool removeSelectionContent();
        void resetSelection();
        bool hasSelection() const;

        EnterFunc enterFunc;
        std::string text;
        const char* hint = nullptr;
        float longestLineLen = 0; // Length of the longest line
        int cursorPos = 0;        // Current text index the cursor is at
        int cursorLine = 0;       // Current line the cursor is in
        int currLineStart = 0;    // Text index where the current line starts
        int blinkDelay = 30;      // Blink delay of the cursor - 30 ticks are 0.5 seconds at 60 ticks/s
        int lineCount = 1;        // Amount of lines (separated by newlines)
        int selectionStart = -1;
        int selectionEnd = 0;
        bool isFocused = false;   // If the textfield is focused
        bool textChanged = false; // If the text was changed
        bool showCursor = true;   // If the cursor is shown
        uint8_t blinkCounter = 0; // Blink counter
    };

    // TextDisplay a simpler than TextField and allows NO editing - its meant for only displaying text
    // However it allows more customization on how text is drawn such as individual colors per line
    struct TextDisplay : UIObject
    {
        TextDisplay(Rect bounds, Anchor anchor = Anchor::NONE, Point insert = {},
                    ScalingMode scaling = ScalingMode::FULL);

        // A section is a piece everything from a single letter to a whole text or line
        void addSection(std::string_view section, Color color = WHITE);

        // Adds text with the given color - just a section with a newline
        void addLine(std::string_view line, Color color = WHITE);

        // Sets the inset for the given side
        void setInset(Direction dir, float val);

        // Gets the ALL text that was rendered last tick
        std::string_view getText() const;

        // Returns the amount of newlines
        int getLineCount() const;

        // Removes sections from the front until a new lines is reached
        void removeLine();

        // Removes the first section
        void removeSection();

        // Removes all sections
        void clear();

        // Sets/gets the line limit - if the rendered text surpasses this limit lines will be popped from the start
        // Default: 0 - no limit
        int getLineLimit() const;
        void setLineLimit(int limit);

    protected:
        void onDraw(const Rect& bounds) override;

        // Specify the font and size to draw the text in - automatically fits the height so it contains the text
        // Text is drawn with linebreaks such that each word is inside the bounds
        void drawText(const Font& f, float fSize);

        // Called for every section - needed as
        // Note: Allows to convert a section to a different string for rendering
        //       e.g. WoW chat when linking items or color codes |#ff0000 Red Text| or item codes |#001 Sword of Khaine|
        // Default: Passthrough: result = section;
        virtual void convertSection(std::string_view section, std::string& result);

        // Called for every section - changed is text after convertSection(), original before
        // Note: This allows for complex effects like different colors for each letter (rainbow effects)
        // Default: DrawTextEx(args);
        virtual void drawSection(const Font& f, float fSize, Point pos, std::string_view converted,
                                 std::string_view original, Color color);

    private:
        struct Section final
        {
            std::string text;
            Color color = WHITE;
        };
        std::vector<Section> sections; // Individual sections
        std::string temp;              // Temporary used for assignment
        std::string rendered;          // Complete rendered text of last tick
        Rect inset = {2, 2, 2, 2};     // Inset in the different directions
        int lines = 0;                 // Amount of newlines
        int limit = 0;
    };

} // namespace magique

#endif // MAGIQUE_TEXTFIELD_H
