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
//      - Arrow keys + CTRL : move cursor one word
//      - BACKSPACE/DEL     : delete in front/behind
//
// The default behavior is to require the user to focus it by clicking it once - once focused it registers the input
// .....................................................................

namespace magique
{
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
        void onUpdate(const Rectangle& bounds, bool wasDrawn) override { updateInputs(); }

        // Updates the text with the current inputs for this tick and updates the focused state
        // Note: This should be called each update tick
        // Returns: true if any event OR text input has been made
        bool updateInputs();

        // Draws the text
        void drawText(float size, Color txt, Color cursor, const Font& font = GetEngineFont(), float spacing = 1) const;

        // Draws the default graphical representation of this textfield
        void drawDefault(const Rectangle& bounds) const;

        // Called everytime the textfield is focused and enter is pressed
        // Return true: signals enter action was consumed and should NOT be treated as newline
        // Return false: signals that enter action was not used and should be treated as newline
        virtual bool onEnterPressed(bool ctrl, bool shift, bool alt) { return false; }

    public:
        // Returns the current text of the textfield
        [[nodiscard]] const std::string& getText() const;

        // Sets the field text to the provided string
        void setText(const char* newText = "");

        // Sets the hint - drawn on the field if empty
        void setHint(const char* hint);

        // Returns true if the text has changed since last time this method was called - resets the changed status
        bool pollTextHasChanged();

        // Sets if the cursor is shown or not and the blink delay in ticks
        void setCursorStatus(bool shown, int delay);

        // Returns true if the textfield is focused
        [[nodiscard]] bool getIsFocused() const;

        // Returns the total amount of lines (count of newlines '\n')
        [[nodiscard]] int getLineCount() const;


    private:
        bool updateControls();

        std::string text;
        const char* hint = nullptr;
        float longestLineLen = 0; // Length of the longest line
        int cursorPos = 0;        // Current text index the cursor is at
        int cursorLine = 0;       // Current line the cursor is in
        int currLineStart = 0;    // Text index where the current line starts
        int blinkDelay = 30;      // Blink delay of the cursor - 30 ticks are 0.5 seconds at 60 ticks/s
        int lineCount = 0;        // Amount of lines (separated by newlines)
        bool isFocused = false;   // If the textfield is focused
        bool textChanged = false; // If the text was changed
        bool showCursor = true;   // If the cursor is shown
        uint8_t blinkCounter = 0; // Blink counter
    };

} // namespace magique

M_UNIGNORE_WARNING()

#endif //MAGIQUE_TEXTFIELD_H