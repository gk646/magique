#ifndef MAGIQUE_TEXT_DRAWER_HPP
#define MAGIQUE_TEXT_DRAWER_HPP

#include <magique/core/Types.h>
#include <raylib/raylib.h>
#include <functional>

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
        void left(const std::string_view& txt, Color tint);
        void center(const std::string_view& txt, Color tint);
        // moves right end of line to the left of the text
        // following calls will align to the left of the previous (only for right() calls)
        void right(const std::string_view& txt, Color tint);

        // Images
        void icon(const TextureRegion& img, bool centeredOnText = true, bool moveCursor = true);

        // Move cursor
        // moves cursor to the beginning of the next line and resets mods
        TextDrawer& linebreak(float amount = 1.0F);

        // Moves the cursor arbitrarily
        TextDrawer& move(Point pos);

        // moves the cursor either vertical or horizontal by the initial gap multiplied with a factor
        // Negative gap for horizontal moves the right end of line cursor
        TextDrawer& gapH(float mult = 1.0F);
        TextDrawer& gapV(float mult = 1.0F);

        // ===== MODS ======//
        // Temporary mod only for the next action

        TextDrawer& modSize(int fsm);

        TextDrawer& modOffset(Point offset);

        // Highlights numbers in a different color
        TextDrawer& modHighlight(Color numberHighlight);

        // shades the next drawn text the given color
        TextDrawer& modShade(Color shade);

        // Centers the text action vertically inside bounds
        TextDrawer& modCenterV();

        // ===== Helpers ======//

        float textWidth(const std::string_view& txt) const;

        // Returns the new lines added inplace to the given string
        int withNewLines(char* str) const;

        // getters
        Point getCursor() const;
        Rect getBounds();
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
        int modSizeMult = 1;
        bool modCenterVert = false;

        Font font;
    };

} // namespace magique

#endif // MAGIQUE_TEXT_DRAWER_HPP
