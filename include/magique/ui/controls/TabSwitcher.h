#ifndef STARFIGHTER_TABSWITCHER_H
#define STARFIGHTER_TABSWITCHER_H

#include <magique/ui/UIContainer.h>

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
        void onDraw(const Rect& bounds) override { drawDefault(bounds); }

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateInputs();
        }

        void updateInputs();
        void drawDefault(const Rect& bounds);

    private:
        int active = 0;
    };

} // namespace magique

#endif // STARFIGHTER_TABSWITCHER_H
