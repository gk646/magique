#ifndef MAGEQUEST_STL_H
#define MAGEQUEST_STL_H

#include <span>
#include <raylib/raylib.h>
#include <magique/internal/enchantum/enchantum.hpp>

//===============================================
// STL (Standard Templates Library)
//===============================================
// ................................................................................
// These are the simple and public types/enums used and exposed by magique
// Note: Some enum use an explicit type to save memory when used in the ECS or networking
// ................................................................................

namespace magique
{
    // Returns the value type of the container
    template <typename Container>
    using value_type_of = std::iterator_traits<decltype(std::cbegin(std::declval<Container>()))>::value_type;

    // Randomly picks n unique elements from the given range and returns them
    // Note: vec MUST not contain duplicates
    template <typename Container>
    std::optional<std::vector<value_type_of<Container>>> PickRandomSequence(const Container& c, int n);

    template <typename Container>
    std::optional<value_type_of<Container>> PickRandom(const Container& c);

    // Returns the enum value as string
    // Note: This requires the whole enum definition to be visible when used
    template <class E>
    std::string_view EnumToString(E val);

    // Returns an optional that contains the matching enum value if it exists
    template <class E>
    std::optional<E> EnumFromString(std::string_view value);

    // Returns the amount of values part of the given enum
    template <class E>
    size_t EnumSize();

    // Returns a iterable view of all values of this enum
    template <class E>
    std::span<const E> EnumValues();

    // Minimal read-only view type that allows implicit conversion from any container
    template <typename Container>
    class View
    {
        using iterator_type = decltype(std::cbegin(std::declval<Container>()));
        iterator_type begin_;
        iterator_type end_;

    public:
        View(const Container& c) : begin_(std::cbegin(c)), end_(std::cend(c)) {}
        auto operator[](size_t index) const -> decltype(*begin_) { return begin_[index]; }
        iterator_type begin() const { return begin_; }
        iterator_type end() const { return end_; }
        size_t size() const { return std::distance(begin_, end_); }
        bool empty() const { return begin_ == end_; }
    };


} // namespace magique

// IMPLEMENTATION

namespace magique
{
    template <typename Container>
    std::optional<std::vector<value_type_of<Container>>> PickRandomSequence(const Container& view, int n)
    {
        using T = value_type_of<Container>;
        if ((int)view.size() < n)
            return {};

        if ((int)view.size() == n)
            return {std::vector<T>{view.begin(), view.end()}};


        std::vector<T> result;
        while ((int)result.size() < n)
        {
            const auto& rand = view[GetRandomValue(0, view.size() - 1)];
            if (!std::ranges::contains(result, rand))
                result.push_back(rand);
        }
        return result;
    }

    template <typename Container>
    std::optional<value_type_of<Container>> PickRandom(const Container& c)
    {
        if (c.empty())
            return {};
        return {c[GetRandomValue(0, c.size() - 1)]};
    }


    template <class E>
    std::string_view EnumToString(E val)
    {
        return enchantum::to_string(val);
    }

    template <class E>
    std::optional<E> EnumFromString(std::string_view input)
    {
        return enchantum::cast<E>(input);
    }

    template <class E>
    size_t EnumSize()
    {
        return enchantum::count<E>;
    }

    template <class E>
    std::span<const E> EnumValues()
    {
        return enchantum::values<E>;
    }

} // namespace magique

#endif // MAGEQUEST_STL_H
