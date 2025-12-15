// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <utility>
#include <cstring>

#include <raylib/raylib.h>

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/util/Math.h>
#include <magique/internal/Macros.h>

#include "internal/utils/CollisionPrimitives.h"
#include "internal/utils/STLUtil.h"
#include "magique/ui/UI.h"

namespace magique
{
    Point::Point(const char* string)
    {
        x = TextToFloat(string);
        while (*string != '\0')
        {
            if (*string == ',')
            {
                string++;
                break;
            }
            string++;
        }
        y = TextToFloat(string);
    }

    Point::Point(const Vector2& vec) : x(vec.x), y(vec.y) {}

    bool Point::operator==(const Point& other) const { return x == other.x && y == other.y; }

    bool Point::operator!=(const Point& other) const { return x != other.x || y != other.y; }

    Point Point::operator+(const Point& other) const { return {x + other.x, y + other.y}; }

    Point Point::operator-(const Point& point) const { return {x - point.x, y - point.y}; }

    Point& Point::operator+=(const Point& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Point& Point::operator-=(const Point& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Point Point::operator*(const Point& other) const { return {x * other.x, y * other.y}; }

    Point& Point::operator*=(const Point& p)
    {
        x *= p.x;
        y *= p.y;
        return *this;
    }

    Point Point::operator/(const Point& p) const { return {x / p.x, y / p.y}; }

    Point Point::operator*(const float i) const { return {x * i, y * i}; }

    bool Point::operator<(float num) const { return x < num && y < num; }

    bool Point::operator<=(float num) const { return x <= num && y <= num; }

    bool Point::operator==(float num) const { return x == num && y == num; }

    Point Point::operator/(const float divisor) const { return {x / divisor, y / divisor}; }

    Point Point::operator-(float f) const { return {x - f, y - f}; }

    Point Point::operator+(float f) const { return {x + f, y + f}; }

    Point& Point::operator+=(float f)
    {
        x += f;
        y += f;
        return *this;
    }

    Point& Point::operator*=(const float f)
    {
        x *= f;
        y *= f;
        return *this;
    }

    Point& Point::operator=(const float f)
    {
        x = f;
        y = f;
        return *this;
    }

    Point& Point::operator-=(float f)
    {
        x -= f;
        y -= f;
        return *this;
    }

    Point& Point::operator/=(float f)
    {
        x /= f;
        y /= f;
        return *this;
    }

    float Point::manhattan(const Point& p) const { return std::abs(x - p.x) + std::abs(y - p.y); }

    float Point::euclidean(const Point& p) const
    {
        float distSqr = (p.x - x) * (p.x - x) + (p.y - y) * (p.y - y);
        return std::sqrt(distSqr);
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

    Vector2 Point::v() const { return Vector2{x, y}; }

    float Point::dot(const Point& p) const { return x * p.x + y * p.y; }

    float Point::cross(const Point& p) const { return x * p.x - y * p.y; }

    Point& Point::invert()
    {
        x = -x;
        y = -y;
        return *this;
    }

    Point& Point::normalize()
    {
        auto magnitude = x * x + y * y;
        if (magnitude != 0) [[likely]]
        {
            SquareRoot(magnitude);
            x /= magnitude;
            y /= magnitude;
        }
        return *this;
    }

    Point& Point::normalizeManhattan()
    {
        const auto magnitude = std::abs(x) + std::abs(y);
        if (magnitude != 0) [[likely]]
        {
            x /= magnitude;
            y /= magnitude;
        }
        return *this;
    }

    float Point::magnitude() const { return std::sqrt(x * x + y * y); }

    Point& Point::round()
    {
        x = std::round(x);
        y = std::round(y);
        return *this;
    }

    Point& Point::floor()
    {
        x = std::floor(x);
        y = std::floor(y);
        return *this;
    }

    Point& Point::ceil()
    {
        x = std::ceil(x);
        y = std::ceil(y);
        return *this;
    }

    Point& Point::clamp(const float min, const float max)
    {
        x = magique::clamp(x, min, max);
        y = magique::clamp(y, min, max);
        return *this;
    }

    Point& Point::decreaseMagnitude(const float f)
    {
        const float magnitude = std::sqrt(x * x + y * y);
        if (magnitude <= f)
        {
            x = 0.0F;
            y = 0.0F;
            return *this;
        }

        const float newMagnitude = magnitude - f;
        const float scaleFactor = newMagnitude / magnitude;
        x *= scaleFactor;
        y *= scaleFactor;

        return *this;
    }

    Point Point::perpendicular(const bool left) const
    {
        // 0/1 goes down (in raylib coordinates)
        // left would be -1/0
        if (left)
        {
            return {-y, x};
        }
        else // right would be 1/0
        {
            return {y, -x};
        }
    }

    void Point::max(const Point& other)
    {
        x = std::max(x, other.x);
        y = std::max(y, other.y);
    }

    void Point::min(const Point& other)
    {
        x = std::min(x, other.x);
        y = std::min(y, other.y);
    }

    Point Point::PerpendicularTowardsPoint(const Point& startPoint, const Point& direction, const Point& target)
    {
        const auto diff = target - startPoint;
        const float crossProduct = direction.cross(diff);
        if (crossProduct > 0)
        {
            return direction.perpendicular(true);
        }
        else
        {
            return direction.perpendicular(false);
        }
    }

    //----------------- SPRITE SHEET -----------------//

    TextureRegion SpriteSheet::getRegion(const int frame) const
    {
        TextureRegion region;
        region.height = height;
        region.width = width;
        region.id = id;
        region.offX = static_cast<uint16_t>(offX + (frame * width));
        region.offY = offY; // Same as only continuous pictures are supported
        return region;
    }

    TextureRegion SpriteAnimation::getCurrentFrame(const uint16_t spriteCount) const
    {
        MAGIQUE_ASSERT(maxDuration > 0 && sheet.frames > 0, "Empty Animation");
        const int count = spriteCount % maxDuration;
        int frame = 0;
        uint16_t durationCount = 0;
        for (const auto duration : durations)
        {
            durationCount += duration;
            if (durationCount > count)
            {
                break;
            }
            frame++;
        }
        return sheet.getRegion(frame);
    }

    bool SpriteAnimation::isValid() const { return sheet.id != 0 && sheet.frames > 0; }


    //----------------- TILE OBJECT PROPERTY -----------------//

    bool TiledProperty::getBool() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::BOOL, "Property does not contain a boolean!");
        return boolean;
    }

    int TiledProperty::getInt() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::INT, "Property does not contain a integer!");
        return integer;
    }

    float TiledProperty::getFloat() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::FLOAT, "Property does not contain a float!");
        return floating;
    }

    const char* TiledProperty::getString() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::STRING, "Property does not contain a string!");
        return string;
    }

    Color TiledProperty::getColor() const
    {
        MAGIQUE_ASSERT(type == TileObjectPropertyType::COLOR, "Property does not contain a color!");
        return GetColor(integer);
    }

    TileObjectPropertyType TiledProperty::getType() const { return type; }

    const char* TiledProperty::getName() const { return name; }

    //----------------- TILE OBJECT -----------------//

    const char* TileObject::getName() const { return name; }

    const TiledProperty* TileObject::getProperty(const char* propertyName) const
    {
        for (const auto& property : customProperties)
        {
            if (property.getName() == nullptr)
            {
                continue;
            }
            if (strcmp(property.getName(), propertyName) == 0)
            {
                return &property;
            }
        }
        return nullptr;
    }

    int TileObject::getID() const { return id; }

    int TileObject::getTileClass() const { return tileClass; }

    int TileObject::getTileID() const { return tileId; }

    //----------------- TILE INFO -----------------//

    static uint32_t ReverseBytes(const uint32_t value)
    {
        return ((value & 0x000000FFU) << 24) | ((value & 0x0000FF00U) << 8) | ((value & 0x00FF0000U) >> 8) |
            ((value & 0xFF000000U) >> 24);
    }

    const TiledProperty* TileInfo::getProperty(const char* name) const
    {
        for (const auto& property : customProperties)
        {
            if (property.getName() == nullptr)
            {
                continue;
            }
            if (strcmp(property.getName(), name) == 0)
            {
                return &property;
            }
        }
        return nullptr;
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

    TileClass ColliderInfo::getTileClass() const
    {
        if (type != ColliderType::TILESET_TILE) [[unlikely]]
        {
            LOG_WARNING("Using the wrong getter. Type has to be TILESET_TILE");
            return {};
        }
        return static_cast<TileClass>(data);
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
        return static_cast<int>(number);
    }

    //----------------- GAMEDEV -----------------/

    ParamType Param::getType() const { return type; }

    //----------------- KEYBIND -----------------//


#define KEY_MACRO(var, func, key, layered)                                                                              \
    if (layered)                                                                                                        \
        var = LayeredInput::func(key);                                                                                  \
    else                                                                                                                \
        var = func(key);


    Keybind::Keybind(int key, bool layered, bool shift, bool ctrl, bool alt) :
        key(key), layered(layered), shift(shift), alt(alt)
    {
    }

    bool Keybind::operator==(const Keybind& other) const
    {
        return key == other.key && layered == other.layered && shift == other.shift && alt == other.alt;
    }

    bool Keybind::isPressed() const
    {
        bool keyPressed = false;
        if (key < 3)
        {
            KEY_MACRO(keyPressed, IsMouseButtonPressed, key, layered);
        }
        else
        {
            KEY_MACRO(keyPressed, IsKeyPressed, key, layered);
        }
        if (keyPressed)
        {
            if (hasShift())
            {
                bool shiftPressed = false;
                KEY_MACRO(shiftPressed, IsKeyPressed, KEY_LEFT_SHIFT, layered);
                if (!shiftPressed)
                {
                    KEY_MACRO(shiftPressed, IsKeyPressed, KEY_RIGHT_SHIFT, layered);
                }
                return shiftPressed;
            }
            if (hasCtrl())
            {
                bool ctrlPressed = false;
                KEY_MACRO(ctrlPressed, IsKeyPressed, KEY_LEFT_CONTROL, layered);
                return ctrlPressed;
            }
            if (hasAlt())
            {
                bool altPressed = false;
                KEY_MACRO(altPressed, IsKeyPressed, KEY_LEFT_ALT, layered);
                return altPressed;
            }
            return true;
        }
        return false;
    }

    bool Keybind::isDown() const
    {
        bool keyPressed = false;
        if (key < 3)
        {
            KEY_MACRO(keyPressed, IsMouseButtonDown, key, layered);
        }
        else
        {
            KEY_MACRO(keyPressed, IsKeyDown, key, layered);
        }
        if (keyPressed)
        {
            if (hasShift())
            {
                bool shiftPressed = false;
                KEY_MACRO(shiftPressed, IsKeyDown, KEY_LEFT_SHIFT, layered);
                if (!shiftPressed)
                {
                    KEY_MACRO(shiftPressed, IsKeyDown, KEY_RIGHT_SHIFT, layered);
                }
                return shiftPressed;
            }
            if (hasCtrl())
            {
                bool ctrlPressed = false;
                KEY_MACRO(ctrlPressed, IsKeyDown, KEY_LEFT_CONTROL, layered);
                return ctrlPressed;
            }
            if (hasAlt())
            {
                bool altPressed = false;
                KEY_MACRO(altPressed, IsKeyDown, KEY_LEFT_ALT, layered);
                return altPressed;
            }
            return true;
        }
        return false;
    }

    bool Keybind::isReleased() const
    {
        bool keyPressed = false;
        if (key < 3)
        {
            KEY_MACRO(keyPressed, IsMouseButtonReleased, key, layered);
        }
        else
        {
            KEY_MACRO(keyPressed, IsKeyReleased, key, layered);
        }
        if (keyPressed)
        {
            if (hasShift())
            {
                bool shiftPressed = false;
                KEY_MACRO(shiftPressed, IsKeyReleased, KEY_LEFT_SHIFT, layered);
                if (!shiftPressed)
                {
                    KEY_MACRO(shiftPressed, IsKeyReleased, KEY_RIGHT_SHIFT, layered);
                }
                return shiftPressed;
            }
            if (hasCtrl())
            {
                bool ctrlPressed = false;
                KEY_MACRO(ctrlPressed, IsKeyReleased, KEY_LEFT_CONTROL, layered);
                return ctrlPressed;
            }
            if (hasAlt())
            {
                bool altPressed = false;
                KEY_MACRO(altPressed, IsKeyReleased, KEY_LEFT_ALT, layered);
                return altPressed;
            }
            return true;
        }
        return false;
    }

    int Keybind::getKey() const { return key; }

    auto Keybind::hasShift() const -> bool { return shift; }

    auto Keybind::hasCtrl() const -> bool { return ctrl; }

    auto Keybind::hasAlt() const -> bool { return alt; }

    bool Keybind::isUIInput() const { return layered; }

    bool Keybind::isMouse() const { return key < 3; }

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
