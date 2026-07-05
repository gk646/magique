#include <cmath>

#include <magique/gamedev/TextDrawer.h>
#include <magique/util/Strings.h>
#include <magique/core/Draw.h>

namespace magique
{

    TextDrawer::TextDrawer(const Font& font, const Rect& bounds, const Point off, Point gap) :
        bounds(bounds), gapp(gap), offf(off), cursor(off), font(font)
    {
        resetMods();
    }

    TextDrawer& TextDrawer::left(const std::string_view& txt, const Color tint, bool moveCursor)
    {
        const auto width = textWidth(txt);
        const auto pos = getCursor();
        drawText(pos, txt, tint);
        if (moveCursor)
            cursor.x += width + gapp.x;
        return *this;
    }
#define MAX_TEXT_BUFFER_LENGTH 2048

    inline char BUFF[MAX_TEXT_BUFFER_LENGTH];

#define FMT_TEXT()                                                                                                      \
    va_list args;                                                                                                       \
    va_start(args, fmt);                                                                                                \
    int requiredByteCount = vsnprintf(BUFF, MAX_TEXT_BUFFER_LENGTH, fmt, args);                                         \
    if (requiredByteCount >= MAX_TEXT_BUFFER_LENGTH)                                                                    \
    {                                                                                                                   \
        char* truncBuffer = BUFF + MAX_TEXT_BUFFER_LENGTH - 4;                                                          \
        sprintf(truncBuffer, "...");                                                                                    \
    }                                                                                                                   \
    va_end(args);

    TextDrawer& TextDrawer::left(Color tint, const char* fmt, ...)
    {
        FMT_TEXT();
        left(BUFF, tint);
        return *this;
    }

    TextDrawer& TextDrawer::center(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto pos = Point{bounds.x + (bounds.width - width) / 2.0F, bounds.y + cursor.y} + modOfffset;
        drawText(pos, txt, tint);
        return *this;
    }

    TextDrawer& TextDrawer::center(Color tint, const char* fmt, ...)
    {
        FMT_TEXT();
        center(BUFF, tint);
        return *this;
    }

    TextDrawer& TextDrawer::right(const std::string_view& txt, const Color tint)
    {
        const auto width = textWidth(txt);
        const auto lineEnd = bounds.x + bounds.width - offf.x;
        const auto pos = Point{lineEnd - (width + cursorEndX), bounds.y + cursor.y} + modOfffset;
        drawText(pos, txt, tint);
        cursorEndX += width + gapp.x;
        return *this;
    }

    TextDrawer& TextDrawer::right(Color tint, const char* fmt, ...)
    {
        FMT_TEXT();
        right(BUFF, tint);
        return *this;
    }

    TextDrawer& TextDrawer::img(const TextureRegion& img, bool centeredOnCursor, bool moveCursor, Color tint)
    {
        auto pos = getCursor();
        auto size = img.getSize() * modSizeMult;
        if (centeredOnCursor)
        {
            pos.y -= (size.y - font.baseSize) / 2.0F;
        }
        pos.floor();
        if (img.isValid())
            DrawRegionPro(img, {pos, size}, 0, {}, tint);
        if (moveCursor)
            cursor.x += size.x + gapp.x;
        resetMods();
        return *this;
    }

    TextDrawer& TextDrawer::imgRight(const TextureRegion& img, bool centeredOnCursor, bool moveCursor, Color tint)
    {
        const auto width = img.getSize().x;
        const auto lineEnd = bounds.x + bounds.width - offf.x;
        auto pos = Point{lineEnd - (width + cursorEndX), bounds.y + cursor.y} + modOfffset;
        const auto size = img.getSize() * modSizeMult;
        if (centeredOnCursor)
        {
            pos.y -= (img.height - font.baseSize) / 2.0F;
        }
        pos.floor();
        if (img.isValid())
            DrawRegionPro(img, {pos, size}, 0, {}, tint);
        if (moveCursor)
            cursorEndX += img.width + gapp.x;
        resetMods();
        return *this;
    }

    TextDrawer& TextDrawer::line(float length, Color tint)
    {
        const auto len = length * bounds.width;
        const float posX = bounds.x + (bounds.width - len) / 2.0F;
        const auto cursY = getCursor().y;
        DrawLineV({posX, cursY}, {len + posX, cursY}, tint);
        return *this;
    }

    TextDrawer& TextDrawer::linebreak(const float amount)
    {
        cursor.x = offf.x;
        cursor.y += amount * (2 + (float)font.baseSize);
        resetMods();
        cursorEndX = 0;
        return *this;
    }

    TextDrawer& TextDrawer::indent(float x)
    {
        bounds.x += x;
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

    TextDrawer& TextDrawer::move(Point pos)
    {
        cursor += pos;
        return *this;
    }

    TextDrawer& TextDrawer::set(Point pos)
    {
        cursor = pos - bounds.pos();
        return *this;
    }

    TextDrawer& TextDrawer::jumpLast()
    {
        cursor.x = offf.x;
        cursorEndX = 0;
        cursor.y = bounds.height - (offf.y + font.baseSize * modSizeMult);
        resetMods();
        return *this;
    }

    TextDrawer& TextDrawer::modSize(int fsm)
    {
        modSizeMult = fsm;
        return *this;
    }

    TextDrawer& TextDrawer::modOffset(Point offset)
    {
        modOfffset = offset;
        return *this;
    }

    TextDrawer& TextDrawer::modHighlight(Color numberHighlight)
    {
        modHighlightColor = numberHighlight;
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

    TextDrawer& TextDrawer::modBackground(Color background)
    {
        backgroundColor = background;
        return *this;
    }

    float TextDrawer::textWidth(const std::string_view& txt) const
    {
        return std::floor(
            MeasureTextEx(font, txt.data(), (float)font.baseSize * (float)modSizeMult, (float)modSizeMult).x);
    }

    int TextDrawer::withNewLines(char* str) const
    {
        return StringSetNewlines(str, MAX_TEXT_BUFFER_LENGTH, bounds.width - offf.x, font, font.baseSize);
    }

    Point TextDrawer::getCursor() const { return bounds.pos() + cursor + modOfffset; }

    Point TextDrawer::getCursorOffset() const { return cursor; }

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

        if (backgroundColor.a > 0)
        {
            DrawTextHighlight(font, txt, pos, fntSize, modSizeMult, backgroundColor);
        }

        if (shadeColor.a > 0)
        {
            DrawPixelText(font, txt, pos + Point{(float)modSizeMult, 0}, modSizeMult, shadeColor);
        }

        if (modHighlightColor.a > 0)
        {
            DrawPixelTextWithNumberHighlight(font, txt.data(), pos, modSizeMult, tint, modHighlightColor);
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
        shadeColor = BLANK;
        backgroundColor = BLANK;
        modCenterVert = false;
    }

} // namespace magique
