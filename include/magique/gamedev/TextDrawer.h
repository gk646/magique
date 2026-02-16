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

    using CustomTextFunc = std::function<Point(TextDrawer& drawer)>;

    struct TextDrawer
    {
        explicit TextDrawer(const Font& font, const Rect& bounds, Point off = Point{2});

        // Draw text and resets mods
        // move cursor to the right of the text
        void left(const std::string_view& txt, Color tint);
        void center(const std::string_view& txt, Color tint);
        // moves cursor to the left of the text
        void right(const std::string_view& txt, Color tint);

        // Executes the custom function and moves the cursor for the returned size
        void custom(const CustomTextFunc& func);

        // Images
        void icon(const TextureRegion& img, bool centeredOnText = true, bool moveCursor = true);

        // Move cursor
        void gap(bool vertical = true, float mult = 1.0F);
        // moves cursor to the beginning of the next line and resets mods
        void linebreak(float amount = 1.0F);
        // Moves the cursor
        void move(Point pos);

        // Helpers
        int withNewLines(char* str) const;
        void setMod(int fsm = 1, Color highlight = WHITE, Point offset = {});
        float textWidth(const std::string_view& txt) const;

        Point getCursor() const;
        Rect& getBounds();
        Rect getBounds() const;

    private:
        void drawText(Point pos, std::string_view txt, Color tint);
        void resetMods();

        Rect bounds;
        Point off;            // Initial offset from bounds
        Point cursor;         // Where next action happens
        float cursorEndX = 0; // offset from line end

        // Mods that can be set
        Color modHighlightColor = BLANK;
        Point modOffset{};
        int modSizeMult = 1;
        Font font;
    };

} // namespace magique

#endif // MAGIQUE_TEXT_DRAWER_HPP
