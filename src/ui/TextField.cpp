#include <raylib/raylib.h>

#include <magique/ui/controls/TextField.h>

namespace magique
{

    const char* TextField::getCText() const { return text.c_str(); }

    const std::string& TextField::getText() const { return text; }

    bool TextField::getHasTextChanged()
    {
        const bool val = textChanged;
        textChanged = false;
        return val;
    }

    void TextField::setShowCursor(const bool show) { showCursor = show; }

    void TextField::setBlinkDelay(const int delay) { blinkDelay = delay; }

    void TextField::updateText()
    {
        int keycode = 0;
        while ((keycode = GetCharPressed()) != 0)
        {
            if (keycode == '\x08') // Backspace
            {

            }
        }
    }

    void TextField::drawDefault(const Rectangle& bounds) {}
} // namespace magique