#include <raylib/raylib.h>

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    bool Point::operator==(const Point other) const { return x == other.x && y == other.y; }

    bool Point::operator!=(const Point other) const { return x != other.x || y != other.y; }

    Point Point::operator+(const Point other) const { return {x + other.x, y + other.y}; }

    Point& Point::operator+=(const Point other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Point Point::operator*(const Point other) const { return {x * other.x, y * other.y}; }

    float Point::manhattan(const Point p) const { return abs(x - p.x) + abs(y - p.y); }

    float Point::euclidean(const Point p) const
    {
        float distSqr = (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
        SquareRoot(distSqr);
        return distSqr;
    }

    float Point::chebyshev(Point p) const
    {
        constexpr auto D = 1.0F;
        constexpr auto D2 = 1.0F;
        const auto dx = abs(x - p.x);
        const auto dy = abs(y - p.y);
        return D * (dx + dy) + (D2 - 2 * D) * std::min(dx, dy);
    }

    float Point::octile(Point p) const
    {
        constexpr auto D = 1.0F;
        constexpr auto D2 = 1.41421356237F;
        const auto dx = abs(x - p.x);
        const auto dy = abs(y - p.y);
        return D * (dx + dy) + (D2 - 2 * D) * std::min(dx, dy);
    }

    Point Point::operator*(const float i) const { return {x * i, y * i}; }

    //----------------- SPRITE SHEET -----------------//

    TextureRegion SpriteSheet::getRegion(const int frame) const
    {
        TextureRegion region;
        region.height = height;
        region.width = width;
        region.id = id;
        region.offX = static_cast<uint16_t>(offX + frame * width);
        region.offY = offY; // Same as only continuous pictures are supported
        return region;
    }

    TextureRegion SpriteAnimation::getCurrentFrame(const uint16_t spriteCount) const
    {
        MAGIQUE_ASSERT(duration != UINT16_MAX, "Invalid sprite animation");
        const int frame = (spriteCount / duration) % sheet.frames;
        return sheet.getRegion(frame);
    }

    Point SpriteAnimation::getAnchor() const { return {static_cast<float>(rotX), static_cast<float>(rotY)}; }

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

    //----------------- TILE INFO -----------------//

    int TileInfo::getClass() const { return clazz; }

    Rectangle TileInfo::getCollisionRect(const float scale) const
    {
        return {static_cast<float>(hitBoxX) * scale, static_cast<float>(hitBoxY) * scale,
                static_cast<float>(hitBoxW) * scale, static_cast<float>(hitBoxH) * scale};
    }

    //----------------- COLLIDER INFO -----------------//

    int ColliderInfo::getColliderClass() const
    {
        if (type != ColliderType::TILEMAP_OBJECT) [[unlikely]]
        {
            LOG_WARNING("Using the wrong getter. Type has to be TILEMAP_OBJECT");
            return INT32_MAX;
        }
        return data;
    }

    int ColliderInfo::getManualGroup() const
    {
        if (type != ColliderType::MANUAL_COLLIDER) [[unlikely]]
        {
            LOG_WARNING("Using the wrong getter. Type has to be MANUAL_COLLIDER");
            return INT32_MAX;
        }
        return data;
    }

    int ColliderInfo::getTileNum() const
    {
        if (type != ColliderType::TILESET_TILE) [[unlikely]]
        {
            LOG_WARNING("Using the wrong getter. Type has to be TILESET_TILE");
            return INT32_MAX;
        }
        return data;
    }

    ColliderInfo::ColliderInfo(const int data, const ColliderType type) : type(type), data(data) {}

    //----------------- COLLISION INFO -----------------//

    bool CollisionInfo::isColliding() const { return penDepth != 0.0F; }

    bool CollisionInfo::getIsAccumulated() const { return isAccumulated; }

    //----------------- KEYBIND -----------------//

    Keybind::Keybind(const int keyCode, const bool shiftDown, const bool CTRLDown, const bool altDown)
    {
        data = static_cast<uint16_t>(keyCode & 0x1FFF);
        if (shiftDown)
            data |= (1 << 13);
        if (CTRLDown)
            data |= (1 << 14);
        if (altDown)
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