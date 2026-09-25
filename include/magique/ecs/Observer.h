// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_OBSERVER_H
#define MAGIQUE_OBSERVER_H

#include <magique/internal/glaze/beve/read.hpp>
#include <magique/internal/glaze/json/patch.hpp>
#include <magique/internal/glaze/beve/write.hpp>
#include <magique/util/Reflection.h>
#include <magique/util/Data.h>
#include <magique/ecs/ECS.h>

//===============================================
// Observer
//===============================================
// ................................................................................
// The Observer API allows to react to arbitrary changes made to components in ANY way
// It works by saving a copy of the component data and using MetaEquals() (from util/Meta.h) to check for changes
// This is primarily intended for multiplayer syncing, but there are probably many other applications
//
// Note: You can annotate members variables with [[="no_reflect"]] to disable syncing e.g:  [[="no_reflect"]] int member;
// Intended Workflow: EntityOnChange() -> ObserverDiff() -> EntityPatch()
// ................................................................................

namespace magique
{
    // Checks if the specified component of the given entity changed since the last time this function was called
    //      - func: executed with (entity, const oldState, const newState) ONLY if the state changed
    // Note: Specified component MUST exists on the entity
    template <typename Component, typename Func>
    void EntityOnChange(Entity entity, Func&& func);

    // Serializes the difference from old to target
    //      - tryCompress: tries compression, only used if it actually decreases size
    // Note: Does NOT work for structs that have unions - should not be used for pointers
    template <typename Component>
    Diff ObserverDiff(const Component& old, const Component& target, bool tryCompress = true);

    // Patches the given component of the given entity using the diff to transform it into the observed target state
    template <typename T>
    bool EntityPatch(Entity entity, Diff diff);

    // Resets the saved state such that any new observation causes an update
    // Note: Should be called when a new game starts
    void ChangeStateClear();

    // Abstraction class that allows to manage changes for many entities across multiple components
    struct ChangeManager
    {
        template <typename... Components>
        void trackComponents();

        // For each given component, iterates all entities with that component and returns the change set
        // Note: The returned changeset is a view and only valid until this method is called again
        template <typename... Components>
        ChangeSet generateChangeSet(FilterFunc filter = nullptr);

        // Applies the changeset if contained changes can be matched to a given component
        // Note: the components specified to generate and apply should match
        void applyChangeSet(ChangeSet set);

    private:
        std::string_view numFromString(std::string_view component);
        std::string_view stringFromNum(std::string_view components);
        std::string changeBuffer;
        std::string compressionBuffer;
        StringHashMap<uint8_t> nameToNum;        // Maps components to numbers to save space
        HashMap<uint8_t, std::string> numToName; // Numbers to component names
        uint16_t componentNum = 0;               // Increasing counter for each unique component
        StringHashMap<std::function<void(Entity, std::string_view)>> applyMap; // Saves apply functions - type erasure
    };

} // namespace magique


// IMPLEMENTATION


namespace magique
{
    namespace internal
    {
        struct Observer final
        {
            entt::registry registry;

            template <typename T, typename Func>
            void onUpdate(Entity e, Func&& func)
            {
                const auto& newState = ComponentGet<T>(e);
                if (!registry.valid(e)) [[unlikely]]
                {
                    if (registry.create(e) != e) [[unlikely]]
                        LOG_FATAL("ID mismatch between cache and main registry");
                }

                if (!registry.all_of<T>(e)) [[unlikely]]
                {
                    // Call the func as this is a new change
                    auto& oldState = registry.emplace<T>(e, newState);
                    func(e, *(const T*)&oldState, newState);
                    return;
                }

                auto& oldState = registry.get<T>(e);
                if (!ReflectEquals(oldState, newState))
                {
                    // Force oldstate to be const
                    func(e, *(const T*)&oldState, newState);
                    oldState = newState;
                }
            }

            void clear() { registry.clear(); }
        };

        inline Observer GLOBAL_OBSERVER{};

        struct change_opts
        {
            uint32_t format = glz::JSON;
            bool bools_as_numbers = true;
            bool reflect_enums = false;
            bool null_terminated = GLZ_NULL_TERMINATED;
            bool comments = false;
            bool error_on_unknown_keys = true;
            bool skip_null_members = true;
            bool prettify = false;
            bool minified = true;
            bool error_on_missing_keys = false;
            bool partial_read = false;
            uint32_t internal{};
        };
    } // namespace internal

    template <typename T, typename Func>
    void EntityOnChange(Entity entity, Func&& func)
    {
        internal::GLOBAL_OBSERVER.onUpdate<T>(entity, func);
    }

    template <typename T>
    Diff ObserverDiff(const T& oldState, const T& newState, bool tryCompress)
    {
        thread_local std::string patchBuffer{};

        auto result = glz::merge_diff<T>(oldState, newState);
        if (result) [[likely]]
        {
            const auto& value = result.value();
            if (value.empty()) [[unlikely]]
                return {};
            (void)glz::write_beve(value, patchBuffer);

            if (!tryCompress)
                return {patchBuffer, false};

            auto& compressed = internal::DataCompressImpl(patchBuffer);
            if (compressed.size() < patchBuffer.size())
            {
                // Move here for more understandable API
                // This data is only valid until next compress call which is not obvious to user
                // Like this its valid until next diff call which is default behavior
                patchBuffer = std::move(compressed);
                return {patchBuffer, true};
            }
            return {patchBuffer, false};
        }
        LOG_ERROR("Failed to generate diff: %s", glz::format_error(result).c_str());
        return {};
    }

    template <typename T>
    bool EntityPatch(Entity entity, Diff diff)
    {
        auto& oldState = ComponentGet<T>(entity);
        if (diff.compressed)
        {
            diff.data = DataDecompress(diff.data).value();
            diff.compressed = false;
        }
        auto err = glz::read_beve(oldState, diff.data);
        return !err;
    }

    inline void ChangeStateClear() { internal::GLOBAL_OBSERVER.clear(); }

    template <typename... Components>
    ChangeSet ChangeManager::generateChangeSet(FilterFunc filter)
    {
        changeBuffer.reserve(1024);
        changeBuffer.clear();

        auto serializeChange = [&]<typename T>(Entity entity, const T& old, const T& target)
        {
            auto diff = ObserverDiff(old, target, false);
            if (diff.data.empty())
                return;
            // Append as raw bytes
            changeBuffer.append(std::string_view{(const char*)&entity, sizeof(Entity)});
            changeBuffer.push_back('\x1F');
            changeBuffer.append(numFromString(std::meta::identifier_of(^^T)));
            changeBuffer.push_back('\x1F');
            changeBuffer.append(diff);
            changeBuffer.push_back('\x1D');
        };

        auto checkComponentChanges = [&]<typename T>()
        {
            for (auto entity : ComponentGetView<T>())
            {
                if (!filter || filter(entity))
                    EntityOnChange<T>(entity, serializeChange);
            }
        };

        (checkComponentChanges.template operator()<Components>(), ...);

        return ChangeSet{changeBuffer};
    }

    template <typename... Components>
    void ChangeManager::trackComponents()
    {
        constexpr std::array<std::string_view, sizeof...(Components)> names = {
            std::meta::identifier_of(^^Components)...};

        auto checkComponentChanges = [&]<typename T>()
        {
            if (applyMap.contains(std::meta::identifier_of(^^T)))
                return;
            applyMap[std::meta::identifier_of(^^T)] = [](Entity entity, std::string_view diff)
            {
                EntityPatch<T>(entity, {diff, false});
            };
        };

        (checkComponentChanges.template operator()<Components>(), ...);

        for (auto name : names)
        {
            if (!nameToNum.contains(name))
            {
                numToName[componentNum] = name;
                nameToNum[name] = componentNum;
                componentNum++;
            }
        }
    }

    inline void ChangeManager::applyChangeSet(ChangeSet set)
    {
        auto packets = StringSplitCopy(set, '\x1D');
        for (auto packet : packets)
        {
            auto parts = StringSplit(packet, '\x1F');
            MAGIQUE_ASSERT(parts.size() == 3, "entity, component, data");
            MAGIQUE_ASSERT(parts[0].size() == sizeof(Entity), "Entity id encoded as bytes");

            const auto entity = *(Entity*)parts[0].data();
            const auto component = stringFromNum(parts[1]);
            const auto data = parts[2];

            const auto it = applyMap.find(component);
            if (it != applyMap.end()) [[likely]]
                it->second(entity, data);
            else
                LOG_WARNING("Component is not tracked: %s", std::string{component}.c_str());
        }
    }

    inline std::string_view ChangeManager::numFromString(std::string_view component)
    {
        return {(const char*)&nameToNum[component], sizeof(uint8_t)};
    }

    inline std::string_view ChangeManager::stringFromNum(std::string_view components)
    {
        return numToName[*(uint8_t*)components.data()];
    }

} // namespace magique


#endif // MAGIQUE_OBSERVER_H
