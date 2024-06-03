#ifndef MAGIQUE_UIOBJECT_H
#define MAGIQUE_UIOBJECT_H

#include <magique/fwd.hpp>

namespace magique
{
    struct UIObject
    {
        // Called each tick on the main thread
        virtual void draw() {}
        // Called each tick
        // Thread: On the udpate thread
        virtual void update() {}

        // Attempt to click on this objects - pos is absolute
        virtual bool click(const Vector2& pos) { return false; }

        // Attempt to hover over this object - pos is absolute
        virtual bool hover() { return true; }

        //----------------- EVENTS -----------------//

        // Called when this object is added as children

        virtual void onAdd() {}
        virtual void onRepsition() {}

        //----------------- GETTERS -----------------//



        [[nodiscard]] bool isHovered() const;

        [[nodiscard]] bool isPressed() const;

    };

} // namespace magique
#endif //MAGIQUE_UIOBJECT_H