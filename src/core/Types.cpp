// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <utility>
#include <cstring>

#include <raylib/raylib.h>

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/internal/Macros.h>

#include "internal/utils/CollisionPrimitives.h"
#include "internal/utils/STLUtil.h"


namespace magique
{
    bool Point::operator==(const Point& other) const { return x == other.x && y == other.y; }

    bool Point::operator!=(const Point& other) const { return x != other.x || y != other.y; }

    Point Point::operator+(const Point& other) const { return {x + other.x, y + other.y}; }

    Point Point::operator/(const float divisor) const { return {x / divisor, y / divisor}; }

    Point& Point::operator+=(const Point& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Point& Point::operator*=(const float f)
    {
        x *= f;
        y *= f;
        return *this;
    }

    Point& Point::operator-=(const Point& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Point Point::operator*(const Point& other) const { return {x * other.x, y * other.y}; }

    float Point::manhattan(const Point& p) const { return std::abs(x - p.x) + std::abs(y - p.y); }

    float Point::euclidean(const Point& p) const
    {
        float distSqr = (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y);
        SquareRoot(distSqr);
        return distSqr;
    }

    float Point::chebyshev(const Point& p) const
    {
        constexpr auto D = 1.0F;
        constexpr auto D2 = 1.0F;
        const auto dx = std::abs(x - p.x);
        const auto dy = std::abs(y - p.y);
        return D * (dx + dy) + (D2 - 2 * D) * minValue(dx, dy);
    }

    float Point::octile(const Point& p) const
    {
        constexpr auto D = 1.0F;
        constexpr auto D2 = 1.41421356237F;
        const auto dx = abs(x - p.x);
        const auto dy = abs(y - p.y);
        return D * (dx + dy) + (D2 - 2 * D) * minValue(dx, dy);
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

    //----------------- TILE OBJECT PROPERTY -----------------//

    //TODO is leaking memory with name and property value / is it bad?

    bool TileObjectCustomProperty::getBool() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::BOOL, "Property does not contain a boolean!");
        return boolean;
    }

    int TileObjectCustomProperty::getInt() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::INT, "Property does not contain a integer!");
        return integer;
    }

    float TileObjectCustomProperty::getFloat() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::FLOAT, "Property does not contain a float!");
        return floating;
    }

    const char* TileObjectCustomProperty::getString() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::STRING, "Property does not contain a string!");
        return string;
    }

    Color TileObjectCustomProperty::getColor() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::COLOR, "Property does not contain a color!");
        return GetColor(integer);
    }

    TileObjectPropertyType TileObjectCustomProperty::getType() const { return type; }

    const char* TileObjectCustomProperty::getName() const { return name; }

    //----------------- TILE OBJECT -----------------//
    //TODO is leaking memory with name / isnt too bad

    const char* TileObject::getName() const
    {
        if (name == nullptr)
            return "";
        return name;
    }

    int TileObject::getClass() const { return type; }

    int TileObject::getID() const { return id; }

    //----------------- TILE INFO -----------------//

    int TileInfo::getClass() const { return tileClass; }

    static uint32_t ReverseBytes(const uint32_t value)
    {
        return ((value & 0x000000FFU) << 24) | ((value & 0x0000FF00U) << 8) | ((value & 0x00FF0000U) >> 8) |
            ((value & 0xFF000000U) >> 24);
    }

    Checksum::Checksum(const char* hexadecimalHash)
    {
        if (hexadecimalHash == nullptr || strlen(hexadecimalHash) != 32)
        {
            LOG_WARNING("Invalid hexadecimal hash string");
            first = second = third = fourth = 0;
            return;
        }
        auto parseHex = [](const char* str) -> uint32_t
        {
            char buffer[9];
            strncpy(buffer, str, 8);
            buffer[8] = '\0';
            return static_cast<uint32_t>(strtoul(buffer, nullptr, 16));
        };

        first = parseHex(hexadecimalHash);
        second = parseHex(hexadecimalHash + 8);
        third = parseHex(hexadecimalHash + 16);
        fourth = parseHex(hexadecimalHash + 24);

        first = ReverseBytes(first);
        second = ReverseBytes(second);
        third = ReverseBytes(third);
        fourth = ReverseBytes(fourth);
    }

    bool Checksum::operator==(const Checksum& o) const
    {
        return first == o.first && second == o.second && third == o.third && fourth == o.fourth;
    }

    void Checksum::print() const
    {
        printf("Checksum: %08x%08x%08x%08x\n", ReverseBytes(first), ReverseBytes(second), ReverseBytes(third),
               ReverseBytes(fourth));
    }

    void Checksum::format(char* buffer, const int size) const
    {
        if (size < 33)
        {
            LOG_WARNING("Need 32 bytes for checksum");
            return;
        }
        snprintf(buffer, size, "%08x%08x%08x%08x", ReverseBytes(first), ReverseBytes(second), ReverseBytes(third),
                 ReverseBytes(fourth));
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

    const char* Param::getName() const { return name; }

    const char* Param::getString() const
    {
        MAGIQUE_ASSERT(type == ParamType::STRING, "Accessing wrong type");
        return string;
    }

    bool Param::getBool() const
    {
        MAGIQUE_ASSERT(type == ParamType::BOOL, "Accessing wrong type");
        return boolean;
    }

    float Param::getFloat() const
    {
        MAGIQUE_ASSERT(type == ParamType::NUMBER, "Accessing wrong type");
        return number;
    }

    int Param::getInt() const
    {
        MAGIQUE_ASSERT(type == ParamType::NUMBER, "Accessing wrong type");
        return (int)number;
    }

    //----------------- GAMEDEV -----------------/

    ParamType Param::getType() const { return type; }

    //----------------- KEYBIND -----------------//

    Keybind::Keybind(const int keyCode, bool UIinput, const bool shift, const bool ctrl, const bool alt)
    {
        key = static_cast<uint16_t>(keyCode);

        if (shift)
        {
            modifier.shift = 1;
        }
        if (ctrl)
        {
            modifier.ctrl = 1;
        }
        if (alt)
        {
            modifier.alt = 1;
        }
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

    int Keybind::getKey() const { return key; }

    auto Keybind::hasShift() const -> bool { return modifier.shift == 1; }

    auto Keybind::hasCtrl() const -> bool { return modifier.ctrl == 1; }

    auto Keybind::hasAlt() const -> bool { return modifier.alt == 1; }

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