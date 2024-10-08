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
// Default controls are:
//      - Arrow keys    : move cursor
//      - CTRL + C      : copy selection
//      - CTRL + V      : paste selection
//      - BACKSPACE/DEL : delete infront/behind
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
        TextField(float w, float h, ScalingMode scaling = ScalingMode::FULL);
        TextField(AnchorPosition anchor, float w, float h, ScalingMode scaling = ScalingMode::FULL);

        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        void onUpdate(const Rectangle& bounds, bool wasDrawn) override { updateText(); }

        // Returns the current text
        [[nodiscard]] const char* getCText() const;
        [[nodiscard]] const std::string& getText() const;

        // Returns true if the text has changed since last time this method was called - resets the changed status
        bool getHasTextChanged();

        // Sets if the cursor is shown or not
        void setShowCursor(bool show);

        // Sets the blink delay for the cursor
        void setBlinkDelay(int delay);

    protected:
        // Updates the text with the current inputs for this tick using GetKeyCodes()
        void updateText();

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