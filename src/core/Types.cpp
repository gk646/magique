// SPDX-License-Identifier: zlib-acknowledgement
#define _CRT_SECURE_NO_WARNINGS
#include "magique/core/Types.h"


#include <cstdio>
#include <utility>
#include <cstring>
#include <cctype>
#include <string_view>
#include <charconv>
#include <algorithm>

#include <raylib/raylib.h>

#include <magique/core/Types.h>
#include <magique/util/Logging.h>
#include <magique/util/Math.h>
#include <magique/internal/Macros.h>

#include "internal/utils/CollisionPrimitives.h"
#include "magique/ui/UI.h"
#include "magique/util/RayUtils.h"

namespace magique
{
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

    bool Point::operator<(const Point& p) const { return x < p.x && y < p.y; }

    Point& Point::operator/=(const Point& p)
    {
        x /= p.x;
        y /= p.y;
        return *this;
    }

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
        return D * (dx + dy) + (D2 - 2 * D) * std::min(dx, dy);
    }

    float Point::octile(const Point& p) const
    {
        constexpr auto D = 1.0F;
        constexpr auto D2 = 1.41421356237F;
        const auto dx = abs(x - p.x);
        const auto dy = abs(y - p.y);
        return D * (dx + dy) + (D2 - 2 * D) * std::min(dx, dy);
    }

    Vector2 Point::v() const { return Vector2{x, y}; }

    int Point::intx() const { return (int)x; }

    int Point::inty() const { return (int)y; }

    float Point::dot(const Point& p) const { return (x * p.x) + (y * p.y); }

    float Point::cross(const Point& p) const { return (x * p.x) - (y * p.y); }

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

    Point Point::floor() const
    {
        Point p = *this;
        return p.floor();
    }

    Point& Point::clamp(const float min, const float max)
    {
        x = std::clamp(x, min, max);
        y = std::clamp(y, min, max);
        return *this;
    }

    Point& Point::decreaseMagnitude(const float f)
    {
        const float mag = magnitude();
        if (mag <= f)
        {
            x = 0.0F;
            y = 0.0F;
            return *this;
        }

        const float newMagnitude = mag - f;
        const float scaleFactor = newMagnitude / mag;
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

    //----------------- RECT -----------------//

    Rect::Rect(const Rectangle& rect) : x(rect.x), y(rect.y), w(rect.width), h(rect.height) {}

    Rect::Rect(const Point& topLeft, const Point& size) : x(topLeft.x), y(topLeft.y), w(size.x), h(size.y) {}

    Rect::Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

    Rect Rect::FromSpanPoints(const Point& p1, const Point& p2)
    {
        const Point smallest = {std::min(p1.x, p2.x), std::min(p1.y, p2.y)};
        const Point biggest = {std::max(p1.x, p2.x), std::max(p1.y, p2.y)};
        Rect rect;
        rect.x = smallest.x;
        rect.y = smallest.y;
        rect.w = biggest.x - smallest.x;
        rect.h = biggest.y - smallest.y;
        return rect;
    }

    Rect& Rect::operator+=(const Point& p)
    {
        x += p.x;
        y += p.y;
        return *this;
    }

    Rect& Rect::operator=(const Point& p)
    {
        x = p.x;
        y = p.y;
        return *this;
    }

    bool Rect::operator==(const float num) const { return x == num && y == num && w == num && h == num; }

    Rectangle Rect::v() const { return Rectangle{x, y, w, h}; }

    Rect& Rect::floor()
    {
        x = std::floor(x);
        y = std::floor(y);
        w = std::floor(w);
        h = std::floor(h);
        return *this;
    }
    Rect& Rect::round()
    {
        x = std::round(x);
        y = std::round(y);
        w = std::round(w);
        h = std::round(h);
        return *this;
    }
    Rect& Rect::zero()
    {
        x = 0.0F;
        y = 0.0F;
        w = 0.0F;
        h = 0.0F;
        return *this;
    }

    Point Rect::random() const { return Point{x + GetRandomFloat(0, w), y + GetRandomFloat(0, h)}; }

    bool Rect::contains(const Point& p) const { return PointToRect(p.x, p.y, x, y, w, h); }

    bool Rect::intersects(const Rect& r) const { return RectToRect(x, y, w, h, r.x, r.y, r.w, r.h); }

    float Rect::area() const { return w * h; }

    Point Rect::pos() const { return Point{x, y}; }

    Point Rect::size() const { return Point{w, h}; }

    Point Rect::mid() const { return Point{x + w / 2, y + h / 2}; }

    Point Rect::closestInside(const Point& p) const
    {
        Point closest{};
        closest.x = std::clamp(p.x, x, x + w);
        closest.y = std::clamp(p.y, y, y + h);
        return closest;
    }

    Point Rect::closestOutline(const Point& p) const
    {
        auto inside = closestInside(p);

        const auto distUp = std::abs(y - inside.y);
        const auto distDown = std::abs((y + h) - inside.y);
        const auto distLeft = std::abs(x - inside.x);
        const auto distRight = std::abs((x + w) - inside.x);

        const auto minDist = std::min({distUp, distDown, distLeft, distRight});
        if (minDist == distUp)
        {
            return {inside.x, y};
        }
        else if (minDist == distDown)
        {
            return {inside.x, y + h};
        }
        else if (minDist == distLeft)
        {
            return {x, inside.y};
        }
        else
        {
            return {x + w, inside.y};
        }
    }

    Rect Rect::scale(const float factor) const { return {x * factor, y * factor, w * factor, h * factor}; }

    Rect Rect::enlarge(const float size) const
    {
        const auto change = size / 2;
        Rect rect;
        rect.x = x - change;
        rect.y = y - change;
        rect.w = w + size;
        rect.h = h + size;
        return rect;
    }

    Rect Rect::CenteredOn(const Point& p, const Point& size) { return {p - size / 2, size}; }

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

    TextureRegion SpriteAnimation::getCurrentFrame(const float millis) const
    {
        MAGIQUE_ASSERT(maxDuration > 0 && sheet.frames > 0, "Empty Animation");
        const int count = (int)millis % maxDuration;
        int frame = 0;
        uint16_t millisCount = 0;
        for (const auto duration : durations)
        {
            millisCount += duration;
            if (millisCount > count)
            {
                break;
            }
            frame++;
        }
        return sheet.getRegion(frame);
    }

    int SpriteAnimation::getDurationUntil(int frame) const
    {
        int duration = 0;
        for (int i = 0; i < frame; i++)
        {
            duration += durations[i];
        }
        return static_cast<int>((float)duration / 1000.0F * MAGIQUE_LOGIC_TICKS);
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

    Param::Param(const ParamInfo& info) : name(info.name), type(info.allowedTypes[0])
    {
        switch (type)
        {
        case ParamType::BOOL:
            boolean = info.boolean;
            break;
        case ParamType::NUMBER:
            number = info.number;
            break;
        case ParamType::STRING:
            string = info.string.c_str();
            break;
        }
    }

    Param::Param(const std::string_view& name, const std::string_view& str) : name(name)
    {
        std::string_view view{str};

        // Bool cases
        if (view == "False" || view == "false" || view == "FALSE" || view == "OFF" || view == "off")
        {
            boolean = false;
            type = ParamType::BOOL;
        }
        else if (view == "True" || view == "true" || view == "TRUE" || view == "ON" || view == "on")
        {
            boolean = true;
            type = ParamType::BOOL;
        }
        else // Either number or string
        {
            auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), number);
            if (ec == std::errc())
            {
                type = ParamType::NUMBER;
            }
            else
            {
                string = str;
                type = ParamType::STRING;
            }
        }
    }

    const char* Param::getName() const { return name.c_str(); }

    const char* Param::getString() const
    {
        MAGIQUE_ASSERT(type == ParamType::STRING, "Accessing wrong type");
        return string.c_str();
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


#define KEY_MACRO(var, func, key)                                                                                       \
    if (layered)                                                                                                        \
        var = LayeredInput::func(key);                                                                                  \
    else                                                                                                                \
        var = func(key);

#define KEY_MACRO_GAMEPAD(var, func, key)                                                                               \
    if (layered)                                                                                                        \
        var = LayeredInput::func(gamepad, key);                                                                         \
    else                                                                                                                \
        var = func(gamepad, key);

#define KEY_MACRO_MODIFIER()


    Keybind::Keybind(KeyboardKey key, bool layered, bool shift, bool ctrl, bool alt) :
        key(key), type(Keyboard), layered(layered), shift(shift), ctrl(ctrl), alt(alt)
    {
    }
    Keybind::Keybind(MouseButton mouse, bool layered, bool shift, bool ctrl, bool alt) :
        key(mouse), type(Mouse), layered(layered), shift(shift), ctrl(ctrl), alt(alt)
    {
    }

    Keybind::Keybind(GamepadButton key, bool layered) : key(key), type(Controller), layered(layered) {}

    bool Keybind::isPressed(int gamepad) const
    {
        bool keyPressed = false;
        switch (type)
        {
        case Mouse:
            KEY_MACRO(keyPressed, IsMouseButtonPressed, key);
            break;
        case Keyboard:
            KEY_MACRO(keyPressed, IsKeyPressed, key);
            break;
        case Controller:
            KEY_MACRO_GAMEPAD(keyPressed, IsGamepadButtonPressed, key);
            break;
        }
        return keyPressed && isModifierDown();
    }

    bool Keybind::isDown(int gamepad) const
    {
        bool keyPressed = false;
        switch (type)
        {
        case Mouse:
            KEY_MACRO(keyPressed, IsMouseButtonDown, key);
            break;
        case Keyboard:
            KEY_MACRO(keyPressed, IsKeyDown, key);
            break;
        case Controller:
            KEY_MACRO_GAMEPAD(keyPressed, IsGamepadButtonDown, key);
            break;
        }
        return keyPressed && isModifierDown();
    }

    bool Keybind::isReleased(int gamepad) const
    {
        bool keyPressed = false;
        switch (type)
        {
        case Mouse:
            KEY_MACRO(keyPressed, IsMouseButtonReleased, key);
            break;
        case Keyboard:
            KEY_MACRO(keyPressed, IsKeyReleased, key);
            break;
        case Controller:
            KEY_MACRO_GAMEPAD(keyPressed, IsGamepadButtonReleased, key);
            break;
        }

        auto anyModifierReleased = [&]()
        {
            bool value = false;
            if (hasShift())
            {
                bool leftShift = false;
                KEY_MACRO(leftShift, IsKeyReleased, KEY_LEFT_SHIFT);
                value = value || leftShift;
            }
            if (hasCtrl())
            {
                bool ctrlPressed = false;
                KEY_MACRO(ctrlPressed, IsKeyReleased, KEY_LEFT_CONTROL);
                value = value || ctrlPressed;
            }
            if (hasAlt())
            {
                bool altPressed = false;
                KEY_MACRO(altPressed, IsKeyReleased, KEY_LEFT_ALT);
                value = value || altPressed;
            }
            return value;
        };

        return keyPressed && anyModifierReleased();
    }

    int Keybind::getKey() const { return key; }

    auto Keybind::hasShift() const -> bool { return shift; }

    auto Keybind::hasCtrl() const -> bool { return ctrl; }

    auto Keybind::hasAlt() const -> bool { return alt; }

    bool Keybind::isLayered() const { return layered; }

    KeyBindType Keybind::getType() const { return type; }

    bool Keybind::isModifierDown() const
    {
        bool value = true;
        if (hasShift())
        {
            bool leftShift = false;
            KEY_MACRO(leftShift, IsKeyDown, KEY_LEFT_SHIFT);
            value = value && leftShift;
        }
        if (hasCtrl())
        {
            bool ctrlPressed = false;
            KEY_MACRO(ctrlPressed, IsKeyDown, KEY_LEFT_CONTROL);
            value = value && ctrlPressed;
        }
        if (hasAlt())
        {
            bool altPressed = false;
            KEY_MACRO(altPressed, IsKeyDown, KEY_LEFT_ALT);
            value = value && altPressed;
        }
        return value;
    }

    //----------------- PARTICLE -----------------//

} // namespace magique
