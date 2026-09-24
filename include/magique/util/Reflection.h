#ifndef MAGIQUE_META_H
#define MAGIQUE_META_H

#include <meta>

//===============================================
// Meta
//===============================================
// ................................................................................
// Helpers to interact more easily with the new C++26 reflection API
// ................................................................................

namespace magique
{
    // Returns all reflected members of the given class that do NOT have any banned annotations
    template <class T>
    consteval auto ReflectClassMembers(std::vector<std::string_view> bannedAnnotations = {"no_reflect"});

    // Performs an equal comparison of the two types by iterating all members and comparing those
    template <class T>
    bool ReflectEquals(const T& first, const T& second);

    // Returns all things in the given namespace that match the filter
    template <typename FilterFunc = decltype([](std::meta::info info) consteval { return true; })>
    consteval auto ReflectNamespace(std::meta::info nameSpace, FilterFunc&& filter = {});

    // Returns true if the given info has the specified annotations
    // e.g. [[="bench"]] void myFastFunction();
    consteval bool ReflectHasAnnotation(std::meta::info info, std::string_view annotations);

    // Returns the name of the given reflected function
    consteval std::string_view ReflectGetFuncName(std::meta::info info);

} // namespace magique


// IMPLEMENTATION


namespace magique
{
    namespace internal
    {
        consteval std::vector<std::meta::info> ReflectMembers(std::meta::info type,
                                                          const std::vector<std::string_view>& bannedAnnotations)
        {
            std::vector<std::meta::info> results;
            for (auto val : std::meta::nonstatic_data_members_of(type, std::meta::access_context::unchecked()))
            {
                bool isBanned = false;
                for (auto banned : bannedAnnotations)
                    if (ReflectHasAnnotation(val, banned))
                        isBanned = true;

                if (!isBanned)
                    results.push_back(val);
            }

            for (auto base : std::meta::bases_of(type, std::meta::access_context::unchecked()))
                results.append_range(ReflectMembers(std::meta::type_of(base), bannedAnnotations));

            return results;
        }
    } // namespace internal

    template <class T>
    consteval auto ReflectClassMembers(std::vector<std::string_view> bannedAnnotations)
    {
        return std::define_static_array(internal::ReflectMembers(^^T, bannedAnnotations));
    }

    template <class T>
    bool ReflectEquals(const T& first, const T& second)
    {
        static constexpr auto members = ReflectClassMembers<T>();
        template for (constexpr auto member : members)
        {
            if (!(first.[:member:] == second.[:member:]))
                return false;
        }
        return true;
    }

    template <typename FilterFunc>
    consteval auto ReflectNamespace(std::meta::info info, FilterFunc&& filter)
    {
        if (!std::meta::is_namespace(info))
            throw std::meta::exception("Info is not a namespace", info);

        std::vector<std::meta::info> results;
        for (auto member : std::meta::members_of(info, std::meta::access_context::unchecked()))
        {
            if (filter(member))
                results.push_back(member);
        }
        return results;
    }

    consteval bool ReflectHasAnnotation(std::meta::info info, std::string_view name)
    {
        for (auto annotation : std::meta::annotations_of(info))
        {
            auto id = std::meta::display_string_of(annotation);
            auto value = id.substr(id.find_first_of('"') + 1, id.find_last_of('"') - id.find_first_of('"') - 1);

            if (value == name)
                return true;
        }
        return false;
    }

    consteval std::string_view ReflectGetFuncName(std::meta::info info)
    {
        if (!std::meta::is_function(info))
            throw std::meta::exception("Info is not a function", info);

        auto display = std::meta::display_string_of(info);
        auto paramStart = display.find_first_of('(');
        auto nameStart =  display.subview(0, paramStart).rfind(' ');
        // Some type use () e.g. __vector(8) char __builtin_ia32_vec_init_v8qi()
        if (nameStart == std::string::npos)
            return display;
        return display.subview(nameStart, display.size() - paramStart);
    }


} // namespace magique
#endif // MAGIQUE_META_H
