#ifndef MAGIQUE_UIROOT_H_
#define MAGIQUE_UIROOT_H_

#include <memory>
#include <optional>
#include <magique/core/Types.h>
#include <magique/ui/types/UIContainer.h>

//-----------------------------------------------
// UI Root
//-----------------------------------------------
// .....................................................................
// The UI module in magique uses a fixed logical resolution of 1920x1080. This means that you can define all dimensions,
// offsets and gaps in absolute numbers. These values are then automatically scaled to fit to the current resolution.
// To fit all screen rations (16:9, 4:3) you can use anchor points.
// .....................................................................

namespace magique
{
    struct UIRoot final : UIContainer
    {
        // Currently hovered object
        std::optional<UIObject*> hoveredObject;

        // Current object attached to the cursor
        std::optional<std::shared_ptr<CursorAttachment>> cursorAttachment;

        // Returns true if the mouse is not hovered over any UI element
        [[nodiscard]] bool hasMouse() const { return _hasMouse; }

    private:
        UIRoot() noexcept;
        bool _hasMouse = false; // If the mouse is hovering any UI element
        friend struct UIData;
    };

} // namespace magique
#endif //MAGIQUE_UIROOT_H_