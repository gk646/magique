#ifndef MAGEQUEST_STL_H
#define MAGEQUEST_STL_H

#include <meta>
#include <span>
#include <algorithm>
#include <raylib/raylib.h>

//===============================================
// STL (Standard Templates Library)
//===============================================
// ................................................................................
// Useful additions to the STL
// ................................................................................

namespace magique
{
    // Returns the value type of the container
    template <typename Container>
    using value_type_of = std::iterator_traits<decltype(std::cbegin(std::declval<Container>()))>::value_type;

    template <typename T>
    using optional_ref = std::optional<std::reference_wrapper<T>>;

    // Randomly picks n unique elements from the given range and returns them
    // Note: vec MUST not contain duplicates
    template <typename Container>
    std::optional<std::vector<value_type_of<Container>>> PickRandomSequence(const Container& c, int n);

    template <typename Container>
    std::optional<value_type_of<Container>> PickRandom(const Container& c);

    // Returns the enum value as string
    // Note: This requires the whole enum definition to be visible when used
    template <class E>
    constexpr std::string_view EnumToString(E val);

    // Returns the matching enum value if it exists
    template <class E>
    constexpr std::optional<E> EnumFromString(std::string_view value);

    // Returns a iterable view of all values of this enum
    template <class E>
    constexpr std::span<const E> EnumValues();

} // namespace magique

// IMPLEMENTATION

namespace magique
{

    template <typename T>
   std::optional<T> TiledPropertyHolder::getEnumProperty(std::string_view name) const
    {
        auto property = getStringProperty(name);
        if (!property.has_value())
            return {};
        return EnumFromString<T>(property.value());
    }

    namespace internal
    {
         template <typename T>
         consteval auto MakeEnumTable() noexcept {
            static constexpr auto members = std::define_static_array(std::meta::enumerators_of(^^T));

            std::size_t index{};
            std::array<std::pair<T, std::string_view>, members.size()> table{};

            template for (constexpr auto item : members) {
                table[index].first  = [:item:];
                table[index].second = std::meta::identifier_of(item);
                ++index;
            }
            return table;
        }

        template <typename T>
         consteval auto MakeEnumList() noexcept {
            static constexpr auto members = std::define_static_array(std::meta::enumerators_of(^^T));

            std::size_t index{};
            std::array<T, members.size()> table{};

            template for (constexpr auto item : members) {
                table[index] = [:item:];
                ++index;
            }
            return table;
        }
    } // namespace internal


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
    constexpr std::string_view EnumToString(E val)
    {
        static constexpr auto table = internal::MakeEnumTable<E>();
        template for (constexpr auto item : table)
        {
            if (item.first == val)
            {
                return item.second;
            }
        }
        return {};
    }

    template <class E>
    constexpr std::optional<E> EnumFromString(std::string_view input)
    {
        static constexpr auto table = internal::MakeEnumTable<E>();
        for ( auto [val, string] : table)
        {
            if (string == input)
            {
                return val;
            }
        }
        return {};
    }

    template <class E>
    constexpr std::span<const E> EnumValues()
    {
        static constexpr auto table = internal::MakeEnumList<E>();
        return std::span{table};
    }

} // namespace magique

#endif // MAGEQUEST_STL_H
