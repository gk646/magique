// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_TEXTFIELD_H
#define MAGIQUE_TEXTFIELD_H

#include <string>
#include <magique/core/Core.h>
#include <magique/ui/UIObject.h>
M_IGNORE_WARNING(4100)

//===============================================
// TextField
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
        TextField(float x, float y, float w, float h, ScalingMode scaling = ScalingMode::FULL);
        TextField(float w, float h, Anchor anchor = Anchor::NONE, ScalingMode scaling = ScalingMode::FULL);

    protected:
        // Same as ui/UIObject.h
        // Note: Text needs to be drawn manually
        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Same as ui/UIObject.h
        void onUpdate(const Rectangle& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateInputs();
        }

        // Updates the text with the current inputs for this tick and updates the focused state
        // Note: This should be called each update tick
        // Returns: true if any event OR text input has been made
        bool updateInputs(bool allowInput = true);

        // Draws the text and updates the selection
        void drawText(float size, Color txt, Color cursor, const Font& font = GetEngineFont(), float spacing = 1,
                      bool centered = true);

        // Draws the default graphical representation of this textfield
        void drawDefault(const Rectangle& bounds);

    public:
        // Called everytime the textfield is focused and enter is pressed
        virtual void wireOnEnter(const EnterFunc& func);

        // Returns the current text of the textfield
        const std::string& getText() const;
        // Sets the textChanged flag to true
        std::string& getTextToModify();

        // Sets the hint - drawn on the field if empty
        void setHint(const char* hint);

        // Returns true if the text has changed since last time this method was called - resets the changed status
        bool pollTextHasChanged();

        // Sets if the cursor is shown or not and the blink delay in ticks
        void setCursorStatus(bool shown, int delay);

        // Returns true if the textfield is focused
        [[nodiscard]] bool getIsFocused() const;
        void setFocused(bool val);

        // Returns the total amount of lines (count of newlines '\n' + 1 (for first line))
        [[nodiscard]] int getLineCount() const;

        // Adjust the bounds such that the text fits inside
        // Keeps the top left the same
        void fitBoundsToText(float size = 14, const Font& font = GetEngineFont(), float spacing = 1,
                             bool heightOnly = false);

    private:
        bool pollControls();
        bool updateControls();
        void updateSelection(float fSize, const Font& font, float spacing);
        void updateLineState();
        Point getCenteredTextPos(const Rectangle& bounds, float fontSize) const;

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

} // namespace magique

M_UNIGNORE_WARNING()

#endif //MAGIQUE_TEXTFIELD_H
