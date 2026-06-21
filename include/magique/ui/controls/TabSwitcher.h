#ifndef MAGIQUE_TABSWITCHER_H
#define MAGIQUE_TABSWITCHER_H

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

        // Sets/gets the align offset applied to the alignment - active tab is aligned after: TOP_CENTER + offset
        void setAlignOffset(Point offset);
        Point getAlignOffset() const;

        void setActive(UIObject* tab);
        void setActive(std::string_view tab);
        void setActive(int index);

        UIObject* getActive() const;
        bool isActive(std::string_view tab) const;

        // Switches to the next tab in the given direction
        void switchLeft();
        void switchRight();

    protected:
        // Draws active the switcher and the active - tabs are aligned with the TOP_CENTER align option relative to the switcher
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
        Point alignOffset{};
        int active = 0;
    };

} // namespace magique

#endif // MAGIQUE_TABSWITCHER_H
