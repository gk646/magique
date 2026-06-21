// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGEQUEST_SCROLL_PANE_H
#define MAGEQUEST_SCROLL_PANE_H

#include <magique/ui/UIObject.h>
#include <raylib/raylib.h>

//===============================================
// Scroll Pane
//===============================================
// .....................................................................
// The ScrollPane allows to draw content that is moved (and clipped) by scrolling
// It automatically shows scroll bars for both horizontal and vertical scrolling IF the content is bigger than the pane
// Note: The scroller is the rectangle that is draggable to move the contents
// Note: The content is aligned to the top left of the pane with the given scroll offset
//        You should use a Container type UIObject as content that holds your actual content and scales with its size
// Controls:
//      DRAG            : Move scroll bar
//      SCROLL          : Move vertical bar
//      CTRL + SCROLL   : Move horizontal bar
// .....................................................................

namespace magique
{
    struct ScrollPane : UIObject
    {
        ScrollPane(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode mode = ScalingMode::FULL);

        // Sets the width of the scroller
        void setScrollerWidth(float width);
        float getScrollerWidth() const;

        // Returns how much the content is offset horizontally and vertically - always >= 0
        Point getScrollOffset() const;

        // Set the content that will be drawn and where it will be aligned in the scroll pane
        void setContent(UIObject& content, Anchor anchor = Anchor::TOP_LEFT, Point inset = {});
        UIObject* getContent() const;

        // Sets the base anchor position of the content
        // Note: This is useful to position the main content inside the pane
        Anchor getContentAnchor() const;
        Point getContentInset() const;
        void setContentAnchor(Anchor anchor, Point inset = {});

        // If true scroller starts at the bottom to move up (instead of at the top to move down)
        // Useful for e.g. chat
        void invertVerticalScrolling(bool invert = true);
        bool isVerticalScrollInverted() const;

        Rectangle getVerticalScrollBounds();
        Rectangle getHorizontalScrollBounds();
        bool getIsVerticalDragging() const;
        bool getIsHorizontalDragging() const;

    protected:
        void onDraw(const Rect& bounds) override
        {
            updateInputs();
            drawContent();
            drawDefault(bounds);
        }

        // Correctly aligns and draws the content
        void drawContent() const;

        // Draws a default representation of scrollers
        void drawDefault(const Rect& bounds);

        // Updates the dragging for the mouse input and aligns the content
        void updateInputs();

    private:
        struct Scroller final
        {
            bool isHorizontal = false;   // If scroller is horizontal
            float offset = 0.0F;         // Offset in px of the scroller
            float scrollerWidth = 0.03F; // How wide the scroller is
            float moveFactor = 1.0F;     // How much content moves for 1px of scroller
            bool invertScroll = false;

            // Drag state
            Point dragStart{};
            float dragStartOffset = 0;
            bool isDragging = false;

            Rectangle getBounds(const Rectangle& cBounds, const Rectangle& pane);
            void updateInputs(const Rectangle& scroller, const Rectangle& pane);
            float getScaledOffset(Anchor anchor) const;
        };
        Scroller horizontal{true};
        Scroller vertical{false};
        UIObject* content = nullptr;
        Anchor contentAnchor = Anchor::TOP_LEFT;
        Point contentInset = {};
    };
} // namespace magique

#endif // MAGEQUEST_SCROLL_PANE_H
