// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_OBSERVER_H
#define MAGIQUE_OBSERVER_H

#include "external/sdefl.h"
#include <magique/ecs/ECS.h>
#include <magique/internal/glaze/json/patch.hpp>

//===============================================
// Change
//===============================================
// ................................................................................
// The change API allows to react to arbitrary changes made to components in ANY way
// It works by saving a copy of the component data and using operator== to check for changes
// This is primarily intended for multiplayer syncing, but there are probably many other applications
//
// Note: Your component needs to implement (or default) operator== in order to work
// ................................................................................

namespace magique
{
    // Checks if the specified component of the given entity changed since the last time this function was called
    //      - func: executed with (entity, const oldState, const newState) ONLY if the state changed
    // Note: Specified component MUST exists on the entity
    template <typename Component, typename Func>
    void EntityOnChange(Entity entity, Func&& func);

    // Applies the given change to the given entity
    template <typename T>
    bool EntityApplyChange(Entity entity, std::string_view change);

    // Serializes the change from old to new
    // Note: Does NOT work for structs that have unions - should not be used for pointers
    template <typename Component>
    std::string_view ChangeSerialize(const Component& old, const Component& target);

    // Returns [name, change] where name is the name of the component
    // Note: This is useful to correctly handle the change on the receiver side
    template <typename Component>
    std::pair<std::string_view, std::string_view> ChangeSerializeEx(const Component& old, const Component& target);

    // Resets the saved state such that any new observation causes an update
    // Note: Should be called when a new game starts
    void ChangeStateClear();

    // Abstraction class that allows to manage changes for many entities across multiple components
    struct ChangeManager
    {
        // For each given component, iterates all entities with that component and returns the change set
        // Note: The returned changeset is a view and only valid until this method is called again
        template <typename... Components>
        ChangeSet generateChangeSet(FilterFunc filter = nullptr);

        // Applies the changeset if contained changes can be matched to a given component
        // Note: the components specified to generate and apply should match
        template <typename... Components>
        void applyChangeSet(ChangeSet set);

    private:
        std::string changeBuffer;
        std::string compressionBuffer;
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
            void onUpdate(Entity e, Func&& func);

            void clear() { registry.clear(); }
        };

        inline Observer GLOBAL_OBSERVER{};

        template <typename T>
        void TrackComponent()
        {
        }
    } // namespace internal

    template <typename T, typename Func>
    void EntityOnChange(Entity entity, Func&& func)
    {
        internal::GLOBAL_OBSERVER.onUpdate<T>(entity, func);
    }

    template <typename T>
    std::string_view ChangeSerialize(const T& oldState, const T& newState)
    {
        thread_local std::string patchBuffer;
        auto patch = glz::merge_diff(oldState, newState).value();
        struct custom_opts
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
        (void)glz::write<custom_opts{}>(patch, patchBuffer);
        return patchBuffer;
    }

    template <typename Component>
    std::pair<std::string_view, std::string_view> ChangeSerializeEx(const Component& old, const Component& target)
    {
        return {std::meta::identifier_of(^^Component), ChangeSerialize(old, target)};
    }

    template <typename T>
    bool EntityApplyChange(Entity entity, std::string_view change)
    {
        auto& oldState = ComponentGet<T>(entity);
        auto err = glz::read<glz::opts{
                                 .minified = true,
                             },
                             T>(oldState, change);
        return !err;
    }

    inline void ChangeStateClear() { internal::GLOBAL_OBSERVER.clear(); }

    template <typename T, typename Func>
    void internal::Observer::onUpdate(Entity e, Func&& func)
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
        if (!(oldState == newState))
        {
            // Force oldstate to be const
            func(e, *(const T*)&oldState, newState);
            oldState = newState;
        }
    }

    template <typename... Components>
    ChangeSet ChangeManager::generateChangeSet(FilterFunc filter)
    {
        changeBuffer.reserve(1024);
        changeBuffer.clear();

        auto serializeChange = [&](Entity entity, const auto& old, const auto& target)
        {
            auto [name, change] = ChangeSerializeEx(old, target);
            changeBuffer.append(TextFormat("%d", (int)entity));
            changeBuffer.push_back('\x1F');
            changeBuffer.append(name);
            changeBuffer.push_back('\x1F');
            changeBuffer.append(change);
            changeBuffer.push_back('\x1D');
        };

        auto checkComponentChanges = [&]<typename T>()
        {
            for (auto entity : ComponentGetView<T>())
            {
                EntityOnChange<T>(entity, serializeChange);
            }
        };

        (checkComponentChanges.template operator()<Components>(), ...);

        if (changeBuffer.size() > 500)
        {
            struct sdefl* sdefl = (struct sdefl*)RL_CALLOC(1, sizeof(struct sdefl));
            int bounds = sdefl_bound(compressionBuffer.size());
            compressionBuffer.resize(bounds);

            int compSize =
                sdeflate(sdefl, compressionBuffer.data(), compressionBuffer.data(), compressionBuffer.size(), 8);
            compressionBuffer.resize(compSize);

            return ChangeSet{compressionBuffer};
        }

        return ChangeSet{changeBuffer};
    }

    template <typename... Components>
    void ChangeManager::applyChangeSet(ChangeSet set)
    {
    }

} // namespace magique


#endif // MAGIQUE_OBSERVER_H
