#ifndef STARFIGHTER_TABSWITCHER_H
#define STARFIGHTER_TABSWITCHER_H

#include <magique/ui/UIContainer.h>

//===============================================
// TabSwitcher
//===============================================
// .....................................................................
// TabSwitcher is a control that draws the name of its children as clickable tabs and allows switching between them
// The active child is then aligned below the switcher and drawn
// .....................................................................

namespace magique
{

    struct TabSwitcher : UIContainer
    {
        TabSwitcher(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode mode = ScalingMode::FULL);

        void setActive(UIObject* tab);
        void setActive(std::string_view tab);
        void setActive(int index);

        UIObject* getActive() const;

        // Switches to the next tab in the given direction
        void switchLeft();
        void switchRight();

    protected:
        // Draws the tabs
        void onDraw(const Rect& bounds) override;

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateInputs();
        }


        // Called for each tab
        virtual Point drawTab(Point pos, const ContainerChild& child, bool isActive);

        // Make the tabs clickable
        void updateInputs();

    private:
        int active = 0;
    };

} // namespace magique

#endif // STARFIGHTER_TABSWITCHER_H
