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

    Anchor ScrollPane::getContentAnchor() const { return contentAnchor; }

    Point ScrollPane::getContentInset() const { return contentInset; }

    void ScrollPane::setContentAnchor(const Anchor anchor, const Point inset)
    {
        contentAnchor = anchor;
        contentInset = inset;
    }

    void ScrollPane::setInvertVertScroll(bool invert) { vertical.invertScroll = invert; }

    bool ScrollPane::getInvertVertScroll() const { return vertical.invertScroll; }

    void ScrollPane::drawContent() const
    {
        beginBoundsScissor();
        if (getContent() != nullptr)
        {
            getContent()->align(getContentAnchor(), *this, getContentInset() + getScrollOffset());
            getContent()->draw();
        }
        EndScissorMode();
    }

    void ScrollPane::drawDefault(const Rectangle& bounds)
    {
        auto& theme = global::ENGINE_CONFIG.theme;
        DrawRectangleLinesEx(bounds, 1, theme.backDark);
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
        content->align(contentAnchor, *this, contentInset + getScrollOffset());
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

        if (contentSize <= paneSize || paneSize == 0) // No need
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
        const auto moveWay = std::ceil(paneSize - scrollerSize);
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
            if (invertScroll)
            {
                scroller = {pane.x + pane.width - width, pane.y + pane.height - offset - scrollerSize, width,
                            scrollerSize};
            }
            else
            {
                scroller = {pane.x + pane.width - width, pane.y + offset, width, scrollerSize};
            }
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

        const auto scroll = GetMouseWheelMove() * (invertScroll ? -2.0F : 2.0F);
        if (CheckCollisionMouseRect(pane) && scroll != 0 && scroller.width != 0)
        {
            isDragging = false;
            if (!isHorizontal && !IsKeyDown(KEY_LEFT_CONTROL))
            {
                offset -= scroll;
                offset = std::floor(std::min(offset, pane.height - scroller.height));
            }
            else if (isHorizontal && IsKeyDown(KEY_LEFT_CONTROL))
            {
                offset -= scroll;
                offset = std::floor(std::min(offset, pane.width - scroller.width));
            }
        }

        if (isDragging)
        {
            const Point diff = (GetMousePos() - dragStart) * (invertScroll ? -1.0F : 1.0F);
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
            LayeredInput::ConsumeMouse();
        }
        else
        {
            isDragging = false;
            dragStartOffset = offset;
            dragStart = GetMousePos();
        }

        offset = std::max(offset, 0.0F);
    }

    float ScrollPane::Scroller::getScaledOffset() const
    {
        const auto val = std::ceil(offset * moveFactor);
        if (invertScroll)
        {
            return val;
        }
        else
        {
            return -val;
        }
    }


} // namespace magique
