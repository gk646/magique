#ifndef STARFIGHTER_DROPDOWN_H
#define STARFIGHTER_DROPDOWN_H

#include <magique/ui/controls/ListMenu.h>

//===============================================
// Dropdown
//===============================================
// .....................................................................
//
// .....................................................................

namespace magique
{

    struct Dropdown : UIObject
    {
        Dropdown(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode scaling = ScalingMode::FULL);

        bool getIsOpen() const;

        ListMenu& getList();

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
        ListMenu list;
        bool isOpen = false;
    };


} // namespace magique

#endif // STARFIGHTER_DROPDOWN_H
