#include <magique/ui/UI.h>
#include <magique/ui/controls/ScrollPane.h>

#include "internal/globals/UIData.h"
#include "magique/util/Math.h"
#include "magique/util/RayUtils.h"
#include "internal/globals/EngineConfig.h"

namespace magique
{
    ScrollPane::ScrollPane(float w, float h, Anchor anchor, Point inset) : UIObject(w, h, anchor, inset)
    {
        // Registered as container so its updated before content despite being drawn behind
        global::UI_DATA.registerObject(this, true);
        setScrollerWidth(w * 0.05F);
    }

    ScrollPane::ScrollPane(float x, float y, float w, float h) : UIObject(x, y, w, h)
    {
        global::UI_DATA.registerObject(this, true);
        setScrollerWidth(w * 0.05F);
    }

    void ScrollPane::setScrollerWidth(float width)
    {
        width = std::max(width, 5.0F);
        vertical.scrollerWidth = width;
        horizontal.scrollerWidth = width;
    }

    float ScrollPane::getScrollerWidth() const
    {
        // At least 5 pixels
        return std::max(GetScaled(vertical.scrollerWidth), 5.0F);
    }

    Point ScrollPane::getScrollOffset() const { return {horizontal.getScaledOffset(), vertical.getScaledOffset()}; }

    void ScrollPane::setContent(UIObject* newContent) { content = newContent; }

    UIObject* ScrollPane::getContent() const { return content; }

    void ScrollPane::drawDefault(const Rectangle& bounds)
    {
        auto& theme = global::ENGINE_CONFIG.theme;
        DrawRectangleLinesEx(bounds, 1, theme.backDark);

        beginBoundsScissor();
        if (content != nullptr)
        {
            content->draw();
        }
        EndScissorMode();

        Color color;
        {
            const auto scroller = getVerticalScrollBounds();
            color = (CheckCollisionMouseRect(scroller) || vertical.isDragging) ? theme.backSelected : theme.backLight;
            DrawRectangleRec(scroller, color);
        }
        {
            const auto scroller = getHorizontalScrollBounds();
            color = (CheckCollisionMouseRect(scroller) || horizontal.isDragging) ? theme.backSelected : theme.backLight;
            DrawRectangleRec(scroller, color);
        }
    }

    void ScrollPane::updateInputs()
    {
        if (content == nullptr)
        {
            return;
        }
        const auto& bounds = getBounds();
        if (!horizontal.isDragging)
        {
            vertical.updateInputs(getVerticalScrollBounds(), bounds);
        }
        if (!vertical.isDragging)
        {
            horizontal.updateInputs(getHorizontalScrollBounds(), bounds);
        }

        const auto offset = getScrollOffset();
        content->align(Direction::LEFT, *this, Point{offset.x, -offset.y} / GetUIScaling());
    }

    Rectangle ScrollPane::getVerticalScrollBounds()
    {
        if (content == nullptr)
        {
            return {};
        }
        const auto& cBounds = content->getBounds();
        const auto& bounds = getBounds();
        return vertical.getBounds(cBounds, bounds);
    }

    Rectangle ScrollPane::getHorizontalScrollBounds()
    {
        if (content == nullptr)
        {
            return {};
        }
        const auto& cBounds = content->getBounds();
        const auto& bounds = getBounds();
        return horizontal.getBounds(cBounds, bounds);
    }

    bool ScrollPane::getIsVerticalDragging() const { return vertical.isDragging; }

    bool ScrollPane::getIsHorizontalDragging() const { return horizontal.isDragging; }

    Rectangle ScrollPane::Scroller::getBounds(const Rectangle& cBounds, const Rectangle& pane)
    {
        Rectangle scroller{};
        float contentSize{};
        float paneSize{};

        if (isHorizontal)
        {
            contentSize = cBounds.width;
            paneSize = pane.width;
        }
        else
        {
            contentSize = cBounds.height;
            paneSize = pane.height;
        }

        if (contentSize <= paneSize) // No need
        {
            offset = 0;
            return scroller;
        }

        // The factor of the scroll size relative to the pane size
        // Gets smaller the bigger the difference
        const auto sizeFactor = ExponentialDecay(contentSize, paneSize, 0.1F, 0.005);
        const float scrollerSize = std::floor(paneSize * sizeFactor);

        // How much we actually have to scroll - only what comes on top of the pane size
        const auto maxScrollOff = contentSize - paneSize;
        // How far we can move the slider - before hitting the bottom with the end of the scroller
        const auto moveWay = std::floor(paneSize - scrollerSize);
        // How much content moves based on 1px scroller
        moveFactor = maxScrollOff / moveWay;

        const auto width = GetScaled(scrollerWidth);
        if (isHorizontal)
        {
            offset = std::min(offset, pane.width - scrollerSize);
            scroller = {pane.x + offset, pane.y + pane.height - width, scrollerSize, width};
        }
        else
        {
            offset = std::min(offset, pane.height - scrollerSize);
            scroller = {pane.x + pane.width - width, pane.y + offset, width, scrollerSize};
        }
        return scroller;
    }

    void ScrollPane::Scroller::updateInputs(const Rectangle& scroller, const Rectangle& pane)
    {
        if (LayeredInput::IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionMouseRect(scroller))
        {
            isDragging = true;
        }

        if (!LayeredInput::IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            isDragging = false;
        }

        if (isDragging)
        {
            const Point diff = GetMousePos() - dragStart;
            if (isHorizontal)
            {
                offset = dragStartOffset + diff.x;
                offset = std::min(offset, pane.width - scroller.width);
            }
            else
            {
                offset = dragStartOffset + diff.y;
                offset = std::min(offset, pane.height - scroller.height);
            }
            offset = std::max(offset, 0.0F);
            LayeredInput::ConsumeMouse();
        }
        else
        {
            isDragging = false;
            dragStartOffset = offset;
            dragStart = GetMousePos();
        }
    }

    float ScrollPane::Scroller::getScaledOffset() const { return std::ceil(offset * moveFactor); }


} // namespace magique
