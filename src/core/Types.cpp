#include <cmath>
#include <type_traits>

#include <magique/core/Types.h>
#include <raylib/raylib.h>

namespace magique
{
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

    bool Keybind::isKeyPressed() const
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

    bool Keybind::isKeyDown() const
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

    bool Keybind::isKeyReleased() const { return IsKeyReleased(getKey()); }

    int Keybind::getKey() const { return data & 0x1FFF; }

    auto Keybind::hasShift() const -> bool { return data & 1 << 13; }

    auto Keybind::hasCtrl() const -> bool { return data & 1 << 14; }

    auto Keybind::hasAlt() const -> bool { return data & 1 << 15; }

    Setting::Setting(Vector2& val) { save(val); }
    Setting::Setting(int val) { save(val); }
    Setting::Setting(bool val) { save(val); }
    Setting::Setting(float val) { save(val); }

    template <typename T>
    T Setting::get() const
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            return (data & 1) != 0;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            return *reinterpret_cast<const float*>(&data);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            return static_cast<int>(data);
        }
        else if constexpr (std::is_same_v<T, Vector2>)
        {
            Vector2 vec;
            auto* intPtr = reinterpret_cast<int32_t*>(&vec);
            intPtr[0] = static_cast<int32_t>(data >> 32);
            intPtr[1] = static_cast<int32_t>(data & 0xFFFFFFFF);
            return vec;
        }
        else
        {
            static_assert(std::is_same_v<T, Vector2>, "This type is not supported!");
        }
    }

    template <typename T>
    void Setting::save(const T& value)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            data = (data & ~1UL) | (value ? 1 : 0);
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            data = *reinterpret_cast<const int64_t*>(&value);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            data = value;
        }
        else if constexpr (std::is_same_v<T, Vector2>)
        {
            const auto* intPtr = reinterpret_cast<int32_t*>(&const_cast<Vector2&>(value));
            data = static_cast<int64_t>(intPtr[0]) << 32 | static_cast<uint32_t>(intPtr[1]) & 0xFFFFFFFF;
        }
        else
        {
            static_assert(std::is_same_v<T, Vector2>, "This type is not supported!");
        }
    }

} // namespace magique