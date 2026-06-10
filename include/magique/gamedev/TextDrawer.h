#ifndef MAGIQUE_TEXT_DRAWER_HPP
#define MAGIQUE_TEXT_DRAWER_HPP

#include <magique/core/Types.h>
#include <raylib/raylib.h>

//===============================================
// TextDrawer
//===============================================
// ................................................................................
// TextDrawer is a very helpful class to draw text in a procedural way
// The whole layout of the text is made with function calls. A cursor is kept internally where the next action happens
// Each action can affect the cursor in a different way (moving it to the logical next position)
// But can also be moved artificially with gap() or linebreak()
// Modifiers affect only the next action (then reset)
// Text is wrapped automatically should it hit the end
//
// Note: Only accepts null terminated string_views!
// ................................................................................

namespace magique
{
    struct TextDrawer
    {
        // off is offset from the bounds - gap is horizontal and vertical spacing between actions
        explicit TextDrawer(const Font& font, const Rect& bounds, Point off = Point{2}, Point gap = {2});

        // Draw text and resets mods
        // move cursor to the right of the text
        TextDrawer& left(const std::string_view& txt, Color tint = WHITE);
        TextDrawer& left(Color tint, const char* fmt, ...);

        // Draw text centered horizontally - does not move cursor
        TextDrawer& center(const std::string_view& txt, Color tint = WHITE);
        TextDrawer& center(Color tint, const char* fmt, ...);

        // moves right end of line to the left of the text
        // following calls will align to the left of the previous (only for right() calls) (right-mode)
        TextDrawer& right(const std::string_view& txt, Color tint = WHITE);
        TextDrawer& right(Color tint, const char* fmt, ...);

        // Images - works with modSize()
        TextDrawer& img(const TextureRegion& img, bool centeredOnCursor = true, bool moveCursor = true);

        // Draws the image using the right-mode and moves the cursor to the left
        TextDrawer& imgRight(const TextureRegion& img, bool centeredOnCursor = true, bool moveCursor = true);

        // Must be implemented by the project manually
        TextDrawer& keybind(Keybind keybind, bool centered = true);

        // ===== CURSOR ======//

        // Moves cursor to the beginning of the next line and resets mods
        TextDrawer& linebreak(float amount = 1.0F);

        // Moves the line start - persistent across linebreak()
        TextDrawer& indent(float x);

        // moves the cursor either vertical or horizontal by the initial gap multiplied with a factor
        // Negative gap for horizontal moves the right end of line cursor
        TextDrawer& gapH(float mult = 1.0F);
        TextDrawer& gapV(float mult = 1.0F);

        // Jumps to the last possible line given the bounds and gap (vertically) - respects modSize()
        TextDrawer& jumpLast();

        // ===== MODS ======//
        // Temporary mod only for the next action

        // Multiplies the fontsize by the given factor
        TextDrawer& modSize(int fsm);

        // Applies an offset to the cursor
        TextDrawer& modOffset(Point offset);

        // Highlights numbers in a different color
        TextDrawer& modHighlight(Color numberHighlight);

        // shades the next drawn text the given color
        TextDrawer& modShade(Color shade);

        // Centers the text action vertically inside bounds
        TextDrawer& modCenterV();

        // Applies a background color to the text
        TextDrawer& modBackground(Color background = ColorAlpha(BLACK, 0.25F));

        // ===== Helpers ======//

        float textWidth(const std::string_view& txt) const;

        // Returns the new lines added inplace to the given string
        int withNewLines(char* str) const;

        // Returns the position of the cursor where the next action happens
        Point getCursor() const;

        // Returns the offset of the cursor from the top left
        Point getCursorOffset() const;

        Rect getBounds() const;
        Rect getGap() const;

    private:
        void drawText(Point pos, std::string_view txt, Color tint);
        void resetMods();

        Rect bounds;
        Point gapp;
        Point offf;
        Point cursor;         // Where next action happens
        float cursorEndX = 0; // offset from line end

        // Mods that can be set
        Color modHighlightColor = BLANK;
        Point modOfffset{};
        Color shadeColor{};
        Color backgroundColor{};
        int modSizeMult = 1;
        bool modCenterVert = false;

        Font font;
    };

} // namespace magique

#endif // MAGIQUE_TEXT_DRAWER_HPP
