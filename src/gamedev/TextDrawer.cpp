#include <magique/gamedev/TextDrawer.h>
#include <magique/util/Strings.h>
#include <magique/core/Draw.h>
#include <raylib/config.h>

namespace magique
{

    TextDrawer::TextDrawer(const Font& font, const Rect& bounds, const Point off) :
        bounds(bounds), off(off), cursor(off), font(font)
    {
        resetMods();
    }

    void TextDrawer::left(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto pos = bounds.pos() + cursor + modOffset;
        drawText(pos, txt, tint);
        cursor.x += width + off.x;
    }

    void TextDrawer::center(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto pos = Point{bounds.x + (bounds.w - width) / 2.0F, bounds.y + cursor.y} + modOffset;
        drawText(pos, txt, tint);
    }

    void TextDrawer::right(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto lineEnd = bounds.x + bounds.w - off.x;
        const auto pos = Point{lineEnd - (width + cursorEndX), bounds.y + cursor.y} + modOffset;
        drawText(pos, txt, tint);
        cursorEndX += width + off.x;
    }

    void TextDrawer::custom(const CustomTextFunc& func) { cursor += func(*this); }

    void TextDrawer::icon(const TextureRegion& img, bool centeredOnText, bool moveCursor)
    {
        auto pos = bounds.pos() + cursor + modOffset;
        if (centeredOnText)
        {
            pos.y -= img.height / 2 - font.baseSize / 2;
        }
        pos.floor();
        DrawRegion(img, pos);
        if (moveCursor)
            cursor.x += img.width + off.x;
    }

    void TextDrawer::gap(bool vertical, float mult)
    {
        if (vertical)
        {
            cursor.y += mult * off.y;
        }
        else
        {
            if (mult > 0)
            {
                cursor.x += mult * off.x;
            }
            else
            {
                cursorEndX += -mult * off.x;
            }
        }
    }

    void TextDrawer::linebreak(const float amount)
    {
        cursor.x = off.x;
        cursor.y += amount * (off.y + (float)font.baseSize);
        resetMods();
    }

    void TextDrawer::move(Point pos) { cursor += pos; }

    int TextDrawer::withNewLines(char* str) const
    {
        return StringSetNewlines(str, MAX_TEXT_BUFFER_LENGTH, bounds.w - off.x, font, font.baseSize);
    }

    void TextDrawer::setMod(const int fsm, Color highlight, Point offset)
    {
        modOffset = offset;
        modSizeMult = fsm;
        modHighlightColor = highlight;
    }

    float TextDrawer::textWidth(const std::string_view& txt) const
    {
        return MeasureTextEx(font, txt.data(), (float)font.baseSize * (float)modSizeMult, (float)modSizeMult).x;
    }

    Point TextDrawer::getCursor() const { return cursor; }

    Rect& TextDrawer::getBounds() { return bounds; }

    Rect TextDrawer::getBounds() const { return bounds; }

    void TextDrawer::drawText(Point pos, std::string_view txt, Color tint)
    {
        const auto fntSize = (float)font.baseSize * (float)modSizeMult;
        const auto width = textWidth(txt);

        pos.floor();

        int newLines = 0;
        if (width >= bounds.w)
        {
            txt = TextFormat("%s", txt.data());
            const auto lineEnd = bounds.w - off.x - cursor.x - modOffset.x;
            newLines = StringSetNewlines((char*)txt.data(), MAX_TEXT_BUFFER_LENGTH, lineEnd, font, fntSize);
        }

        if (modHighlightColor.a > 0)
        {
            DrawPixelTextNumbers(font, txt.data(), pos, modSizeMult, tint, modHighlightColor);
        }
        else
        {
            DrawPixelText(font, txt, pos, modSizeMult, tint);
        }

        if (newLines > 0)
        {
            linebreak((float)newLines);
        }
        resetMods();
    }

    void TextDrawer::resetMods()
    {
        modOffset = 0;
        modHighlightColor = BLANK;
        modSizeMult = 1;
        cursorEndX = 0;
    }

} // namespace magique
