#ifndef MAGIQUE_UI__POPUP_H
#define MAGIQUE_UI__POPUP_H

#include <functional>
#include <magique/ui/UIObject.h>
#include <magique/ui/controls/Button.h>

//===============================================
// Popup
//===============================================
// .....................................................................
// A popup is usually used to confirm users action or to display immediate information to the user
// It can be closed or some action can be executed (e.g. closing the game)
// It should be drawn as the last thing (using UIDrawPopups()) as it should overlap the rest
//
// Note: Use the UIDrawPopup() / UIAddPopup() API from UI.h to display and draw them
// .....................................................................

namespace magique
{
    using ActionFunction = std::function<void()>;

    struct Popup : UIObject
    {
        Popup(Rect size, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode scaling = ScalingMode::FULL);

        // Closes the popup with no action
        void close();

        // Executes the popup action
        void action() const;

        // Sets the action if the popup resolves no with just closing
        void setOnAction(const ActionFunction& func);

    protected:
        void onUpdate(const Rect& bounds, bool wasDrawn) override;

    private:
        ActionFunction actionFunc;
    };

    // Shows a simple modal with the info text and two buttons - darkens the rest of the screen
    struct ConfirmPopup : Popup
    {
        ConfirmPopup(std::string_view infoText);

        void onDraw(const Rect& bounds) override;

    private:
        std::string infoText;
        TextButton confirm{"Confirm"};
        TextButton back{"Back"};
    };

} // namespace magique


#endif // MAGIQUE_UI__POPUP_H
