#ifndef MAGIQUE_META_H
#define MAGIQUE_META_H

#include <inplace_vector>
#include <meta>

//===============================================
// Meta
//===============================================
// ................................................................................
// Helpers to interact more easily with the new C++26 reflection API
// ................................................................................

namespace magique
{
    // Returns a vector of meta information of all reflected members (including base classes)
    template <typename T>
    consteval auto ReflectAllMembers(std::inplace_vector<const char*, 5> bannedAnnotations = {"no_reflect"});

    // Performs and equal comparison of the two types by iterating all members and comparing those
    // Uses MetaAllMembers()
    template <typename T>
    bool ReflectEquals(const T& first, const T& second);

    // Returns all classes in the given namespace
    // Allows to specify an optional filter function called with each classes info
    template <typename FilterFunc = decltype([](std::meta::info info) consteval { return true; })>
    consteval auto ReflectNamespaceClasses(std::meta::info nameSpace, FilterFunc&& filter = {});

} // namespace magique


// IMPLEMENTATION


namespace magique
{
    namespace internal
    {
        consteval std::vector<std::meta::info> MetaAllMembersImpl(std::meta::info type,
                                                          const std::inplace_vector<const char*, 5>& bannedAnnotations)
        {
            std::vector<std::meta::info> results;
            for (auto val : std::meta::nonstatic_data_members_of(type, std::meta::access_context::unchecked()))
            {
                auto annotations = std::meta::annotations_of(val);
                bool isBanned = false;
                for (auto annotation : annotations)
                {
                    auto id = std::meta::display_string_of(annotation);
                    auto value = id.substr(id.find_first_of('"') + 1, id.find_last_of('"') - id.find_first_of('"') - 1);

                    for (auto badAnnotation : bannedAnnotations)
                        if (value == badAnnotation)
                            isBanned = true;
                }
                if (!isBanned)
                    results.push_back(val);
            }

            for (auto base : std::meta::bases_of(type, std::meta::access_context::unchecked()))
                results.append_range(MetaAllMembersImpl(std::meta::type_of(base), bannedAnnotations));

            return results;
        }
    } // namespace internal

    template <typename T>
    consteval auto ReflectAllMembers(std::inplace_vector<const char*, 5> bannedAnnotations)
    {
        return std::define_static_array(internal::MetaAllMembersImpl(^^T, bannedAnnotations));
    }

    template <typename T>
    bool ReflectEquals(const T& first, const T& second)
    {
        static constexpr auto members = ReflectAllMembers<T>();
        template for (constexpr auto member : members)
        {
            if (!(first.[:member:] == second.[:member:]))
                return false;
        }
        return true;
    }

    template <typename FilterFunc = decltype([](std::meta::info info) consteval { return true; })>
    consteval auto ReflectNamespaceClasses(std::meta::info info, FilterFunc&& filter)
    {
        if (!std::meta::is_namespace(info))
            throw std::meta::exception("Info is not a namespace", info);

        std::vector<std::meta::info> results;
        for (auto member : std::meta::members_of(info, std::meta::access_context::unchecked()))
        {
            if (std::meta::is_type(member) && std::meta::is_class_type(member) && filter(member))
                results.push_back(member);
        }
        return results;
    }

} // namespace magique
#endif // MAGIQUE_META_H
