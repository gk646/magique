#ifndef MAGEQUEST_SWITCHBUTTON_H
#define MAGEQUEST_SWITCHBUTTON_H

#include <string>
#include <functional>
#include <magique/ui/UIObject.h>

//===============================================
// Checkbox
//===============================================
// .....................................................................
// A control that allows to toggle between a ON and OFF state
// .....................................................................

namespace magique
{
    // Called with the new state after a change
    using ButtonChangeFunc = std::function<void(bool state)>;

    struct CheckBox : LabelledObject
    {
        CheckBox(Rect bounds, std::string_view label = "", Direction labelDir = Direction::LEFT,
                 Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode scaling = ScalingMode::FULL);

        // Sets the function called when the state changes
        void setOnChange(const ButtonChangeFunc& func);

        // Returns the current state of the checkbox
        bool getState() const;
        void setState(bool state, bool triggerCallback = false);


    protected:
        void onDraw(const Rect& bounds) override
        {
            LabelledObject::onDraw(bounds);
            drawDefault(bounds);
        }

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateInputs();
        }
        // Updates the click state
        void updateInputs();

        // Draws a default representation
        void drawDefault(const Rect& bounds) const;

    private:
        ButtonChangeFunc func;
        bool state = false;
    };

} // namespace magique

#endif // MAGEQUEST_SWITCHBUTTON_H
