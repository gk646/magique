#pragma once
#ifndef MAGIQUE_UI_UIROOT_H_
#define MAGIQUE_UI_UIROOT_H_

#include <memory>
#include <optional>

#include <magique/fwd.hpp>
#include <magique/ui/UIObject.h>
#include <magique/ui/types/CursorAttachment.h>

enum AnchorPos : uint8_t
{
    LEFT_TOP,
    LEFT_CENTER,
    LEFT_BOTTOM,
    CENTER_TOP,
    CENTER_MID,
    CENTER_BOTTOM,
    RIGHT_TOP,
    RIGHT_MID,
    RIGHT_BOTTOM
};

enum Size : uint8_t
{
    MINI,
    SMALL,
    MID,
    BIG
};

namespace magique
{
    struct UIRoot final
    {
        // Currently hovered object
        std::optional<UIObject*> hoveredObject;

        // Current object attached to the cursor
        std::optional<std::shared_ptr<CursorAttachment>> cursorAttachment;

        [[nodiscard]] bool hasMouse() const { return _hasMouse; }

        uint64_t getNextID() { return nextID++; }

    private:
        bool _hasMouse = false; // If the mouse is hovering any UI element
        uint64_t nextID = 1;
    };
} // namespace magique
#endif //MAGIQUE_UI_UIROOT_H_