#ifndef STARFIGHTER_DROPDOWN_H
#define STARFIGHTER_DROPDOWN_H

#include <magique/ui/controls/ListChooser.h>

//===============================================
// Dropdown
//===============================================
// .....................................................................
// Dropdown is a composite UIObject that when pressed shows a list of selectable values and displays the current value
// Uses a ListChooser to display and select an entry
// .....................................................................

namespace magique
{

    struct Dropdown : UIObject
    {
        Dropdown(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode scaling = ScalingMode::FULL);

        // Returns true if the list is visible
        bool getIsOpen() const;

        // Returns the list
        ListChooser& getList();

    protected:
        void onDraw(const Rect& bounds) override { drawDefault(bounds); }

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
            {
                updateInputs();
                if (getIsOpen())
                {
                    getList().updateState();
                }
            }
        }

        // Updates the inputs to extend menu when body is clicked
        void updateInputs();

        // Draws the default representation
        void drawDefault(const Rect& bounds);

    private:
        ListChooser list;
        bool isOpen = false;
    };


} // namespace magique

#endif // STARFIGHTER_DROPDOWN_H
