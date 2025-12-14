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
        ScrollPane(float w, float h, Anchor anchor = Anchor::NONE, Point inset = {});
        ScrollPane(float x, float y, float w, float h);

        // Sets the width of the scroller
        void setScrollerWidth(float width);
        float getScrollerWidth() const;

        // Returns how much the content is offset horizontally and vertically - always >= 0
        Point getScrollOffset() const;

        // Adds content that will be drawn
        void setContent(UIObject* content);
        UIObject* getContent() const;

        // Sets the base anchor position of the content
        // Note: This is useful to position the main content inside the pane
        Anchor getContentAnchor() const;
        Point getContentInset() const;
        void setContentAnchor(Anchor anchor, Point inset = {});

        // If true scroller starts at the bottom to move up (instead of at the top to move down)
        // Useful for e.g. chat
        void setInvertVertScroll(bool invert);
        bool getInvertVertScroll() const;

    protected:
        void onDrawUpdate(const Rectangle& bounds, bool wasDrawn) override { updateInputs(); }
        void onDraw(const Rectangle& bounds) override { drawDefault(bounds); }

        // Draws a default representation
        void drawDefault(const Rectangle& bounds);

        // Updates the dragging for the mouse input and aligns the content
        void updateInputs();

        Rectangle getVerticalScrollBounds();
        Rectangle getHorizontalScrollBounds();
        bool getIsVerticalDragging() const;
        bool getIsHorizontalDragging() const;

    private:
        struct Scroller final
        {
            bool isHorizontal = false;  // If scroller is horizontal
            float offset = 0.0F;        // Offset in px of the scroller
            float scrollerWidth = 0.05; // How wide the scroller is
            float moveFactor = 1.0F;    // How much content moves for 1px of scroller
            bool invertScroll = false;

            // Drag state
            Point dragStart{};
            float dragStartOffset = 0;
            bool isDragging = false;

            Rectangle getBounds(const Rectangle& cBounds, const Rectangle& pane);
            void updateInputs(const Rectangle& scroller, const Rectangle& pane);
            float getScaledOffset() const;
        };
        Scroller horizontal{true};
        Scroller vertical{false};
        UIObject* content = nullptr;
        Anchor contentAnchor = Anchor::TOP_LEFT;
        Point contentInset = {};
    };
} // namespace magique

#endif //MAGEQUEST_SCROLL_PANE_H
