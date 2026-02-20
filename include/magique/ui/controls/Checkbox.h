#ifndef MAGEQUEST_SWITCHBUTTON_H
#define MAGEQUEST_SWITCHBUTTON_H

#include <string>
#include <magique/ui/UIObject.h>
#include <functional>

//===============================================
// Checkbox
//===============================================
// .....................................................................
// A control that allows to toggle between a ON and OFF state
// .....................................................................

namespace magique
{

    // Called with the new state after a change
    using SwitchFunc = std::function<void(bool state)>;

    struct CheckBox : UIObject
    {
        CheckBox(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode mode = ScalingMode::FULL);

        void onDraw(const Rect& bounds) override { drawDefault(bounds); }

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateInputs();
        }

        // Sets the function called when the state changes
        void setOnClick(const SwitchFunc& func);

        // Returns the current state of the checkbox
        bool getState() const;
        void setState(bool state);

        const std::string& getInfoText() const;
        void setInfoText(const std::string& text);

    protected:
        // Updates the click state
        void updateInputs();

        // Draws a default representation
        void drawDefault(const Rectangle& bounds) const;

    private:
        SwitchFunc func;
        std::string infoText;
        bool state = false;
    };

} // namespace magique

#endif // MAGEQUEST_SWITCHBUTTON_H
