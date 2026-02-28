#include <magique/gamedev/TextDrawer.h>
#include <magique/util/Strings.h>
#include <magique/core/Draw.h>
#include <raylib/config.h>

namespace magique
{

    TextDrawer::TextDrawer(const Font& font, const Rect& bounds, const Point off, Point gap) :
        bounds(bounds), gapp(gap), offf(off), cursor(off), font(font)
    {
        resetMods();
    }

    void TextDrawer::left(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto pos = bounds.pos() + cursor + modOfffset;
        drawText(pos, txt, tint);
        cursor.x += width + gapp.x;
    }

    void TextDrawer::center(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto pos = Point{bounds.x + (bounds.width - width) / 2.0F, bounds.y + cursor.y} + modOfffset;
        drawText(pos, txt, tint);
    }

    void TextDrawer::right(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto lineEnd = bounds.x + bounds.width - offf.x;
        const auto pos = Point{lineEnd - (width + cursorEndX), bounds.y + cursor.y} + modOfffset;
        drawText(pos, txt, tint);
        cursorEndX += width + gapp.x;
    }

    void TextDrawer::icon(const TextureRegion& img, bool centeredOnText, bool moveCursor)
    {
        auto pos = bounds.pos() + cursor + modOfffset;
        if (centeredOnText)
        {
            pos.y -= img.height / 2 - font.baseSize / 2;
        }
        pos.floor();
        DrawRegion(img, pos);
        if (moveCursor)
            cursor.x += img.width + gapp.x;
    }

    TextDrawer& TextDrawer::linebreak(const float amount)
    {
        cursor.x = offf.x;
        cursor.y += amount * (2 + (float)font.baseSize);
        resetMods();
        return *this;
    }

    TextDrawer& TextDrawer::move(Point pos)
    {
        cursor += pos;
        return *this;
    }

    TextDrawer& TextDrawer::gapH(float mult)
    {
        if (mult > 0)
        {
            cursor.x += mult * gapp.x;
        }
        else
        {
            cursorEndX += -mult * gapp.x;
        }
        return *this;
    }

    TextDrawer& TextDrawer::gapV(float mult)
    {
        cursor.y += mult * gapp.y;
        return *this;
    }
    TextDrawer& TextDrawer::modSize(int fsm)
    {
        modSizeMult = fsm;
        return *this;
    }

    TextDrawer& TextDrawer::modHighlight(Color numberHighlight)
    {
        modHighlightColor = numberHighlight;
        return *this;
    }

    TextDrawer& TextDrawer::modOffset(Point offset)
    {
        modOfffset = offset;
        return *this;
    }

    TextDrawer& TextDrawer::modShade(Color shade)
    {
        shadeColor = shade;
        return *this;
    }

    TextDrawer& TextDrawer::modCenterV()
    {
        modCenterVert = true;
        return *this;
    }

    float TextDrawer::textWidth(const std::string_view& txt) const
    {
        return MeasureTextEx(font, txt.data(), (float)font.baseSize * (float)modSizeMult, (float)modSizeMult).x;
    }

    int TextDrawer::withNewLines(char* str) const
    {
        return StringSetNewlines(str, MAX_TEXT_BUFFER_LENGTH, bounds.width - offf.x, font, font.baseSize);
    }

    Point TextDrawer::getCursor() const { return cursor; }

    Rect  TextDrawer::getBounds() { return bounds; }

    Rect TextDrawer::getBounds() const { return bounds; }

    Rect TextDrawer::getGap() const { return gapp; }

    void TextDrawer::drawText(Point pos, std::string_view txt, Color tint)
    {
        const auto fntSize = (float)font.baseSize * (float)modSizeMult;
        const auto width = textWidth(txt);

        if (modCenterVert)
        {
            pos.y = bounds.mid().y - fntSize / 2.0F;
        }

        pos.floor();

        int newLines = 0;
        if (width >= bounds.width)
        {
            txt = TextFormat("%s", txt.data());
            const auto lineEnd = bounds.width - offf.x - cursor.x - modOfffset.x;
            newLines = StringSetNewlines((char*)txt.data(), MAX_TEXT_BUFFER_LENGTH, lineEnd, font, fntSize);
        }

        if (shadeColor.a > 0)
        {
            DrawPixelText(font, txt, pos + Point{1 * (float)modSizeMult, 0}, modSizeMult, shadeColor);
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
        modOfffset = 0;
        modHighlightColor = BLANK;
        modSizeMult = 1;
        cursorEndX = 0;
        shadeColor = BLANK;
        modCenterVert = false;
    }

} // namespace magique
