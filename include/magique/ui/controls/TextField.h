#ifndef MAGIQUE_TEXTFIELD_H
#define MAGIQUE_TEXTFIELD_H

#include <string>
#include <magique/ui/UIObject.h>
IGNORE_WARNING(4100)

//-----------------------------------------------
// TextField
//-----------------------------------------------
// .....................................................................
// The textfield is useful to capture text input and allows selecting and editing text inside the field
// Controls are:
//      - Arrow keys    : move cursor
//      - CTRL + C      : copy selection
//      - CTRL + V      : paste selection
//      - BACKSPACE/DEL : delete in front/behind
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

        // Same as ui/UIObject.h
        // Note: Text needs to be drawn manually
        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Same as ui/UIObject.h
        void onUpdate(const Rectangle& bounds, bool wasDrawn) override { updateInputs(); }

        // Returns the current text of the textfield
        [[nodiscard]] const char* getCText() const;
        [[nodiscard]] const std::string& getText() const;

        // Returns true if the text has changed since last time this method was called - resets the changed status
        bool getHasTextChanged();

        // Sets if the cursor is shown or not and the blink delay in ticks
        void setCursorStatus(bool shown, int delay);

    protected:
        // Updates the text with the current inputs for this tick - also updates text selection
        void updateInputs();

        // Scales the dimensions of the textfield based on the current text and given font size
        // If set, adjusts the width to the length of the longest line and the height to the amount of lines
        void scaleSizeWithText(float fontSize, bool scaleHorizontal = true, bool scaleVertical = true);

    private:
        void drawDefault(const Rectangle& bounds);

        std::string text;
        int cursorPos = 0;
        int selectionStart = 0;
        int selectionEnd = 0;
        int blinkDelay = 30; // 30 ticks are 0.5 seconds at 60 ticks/s
        bool isFocused = false;
        bool textChanged = false;
        bool showCursor = true;
    };

} // namespace magique

UNIGNORE_WARNING()


#endif //MAGIQUE_TEXTFIELD_H