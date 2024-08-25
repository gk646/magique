#include <raylib/raylib.h>

#include <magique/core/Types.h>

namespace magique
{
    int Animation::getCurrentTexture(const uint16_t spriteCount) const
    {
        return spriteCount % sheet.frames * duration / duration;
    }

    //----------------- TILE OBJECT -----------------//

    const char* TileObject::getName() const
    {
        if (name == nullptr)
            return "";
        return name;
    }

    int TileObject::getClass() const { return type; }

    int TileObject::getID() const { return id; }

    Rectangle TileObject::getRect() const { return {x, y, width, height}; }

    //----------------- COLLISION INFO -----------------//

    bool CollisionInfo::isColliding() const { return penDepth != 0.0F; }

    //----------------- KEYBIND -----------------//

    Keybind::Keybind(const int key, const bool shift, const bool ctrl, const bool alt)
    {
        data = static_cast<uint16_t>(key & 0x1FFF);
        if (shift)
            data |= (1 << 13);
        if (ctrl)
            data |= (1 << 14);
        if (alt)
            data |= (1 << 15);
    }

    bool Keybind::isPressed() const
    {
        if (IsKeyPressed(getKey()))
        {
            if (hasShift())
            {
                return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
            }
            if (hasCtrl())
            {
                return IsKeyDown(KEY_LEFT_CONTROL);
            }
            if (hasAlt())
            {
                return IsKeyDown(KEY_LEFT_ALT);
            }
            return true;
        }
        return false;
    }

    bool Keybind::isDown() const
    {
        if (IsKeyDown(getKey()))
        {
            if (hasShift())
            {
                return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
            }
            if (hasCtrl())
            {
                return IsKeyDown(KEY_LEFT_CONTROL);
            }
            if (hasAlt())
            {
                return IsKeyDown(KEY_LEFT_ALT);
            }
            return true;
        }
        return false;
    }

    bool Keybind::isReleased() const { return IsKeyReleased(getKey()); }

    int Keybind::getKey() const { return data & 0x1FFF; }

    auto Keybind::hasShift() const -> bool { return data & 1 << 13; }

    auto Keybind::hasCtrl() const -> bool { return data & 1 << 14; }

    auto Keybind::hasAlt() const -> bool { return data & 1 << 15; }

    //----------------- PARTICLE -----------------//

    Color ScreenParticle::getColor() const { return {r, g, b, a}; }

    void ScreenParticle::setColor(const Color& color)
    {
        r = color.r;
        g = color.g;
        b = color.b;
        a = color.a;
    }

} // namespace magique