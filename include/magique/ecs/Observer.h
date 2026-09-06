// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_OBSERVER_H
#define MAGIQUE_OBSERVER_H

#include "external/sdefl.h"
#include <magique/internal/entt/entity/registry.hpp>
#include <magique/internal/glaze/json/patch.hpp>

//===============================================
// Observer
//===============================================
// ................................................................................
// The observer API allows to react to arbitrary changes made to components in ANY way
// It works by saving a copy of the component data and using == (equality operator) to check for changes
// This is primarily intended for multiplayer syncing, but there are probably many other applications
//
// Note: Your component needs to implement (or default) == in order to work
// ................................................................................

namespace magique
{
    // Checks if the specified component of the given entity changed since the last time this function was called
    //      - func: executed with (entity, const oldState, const newState) ONLY if the state changed
    // Note: Specified component MUST exists on the entity
    template <typename T, typename Func>
    void EntityObserveChange(Entity entity, Func&& func);

    // Resets the saved state such that any new observation causes an update
    // Note: Should be called when a new game starts
    void EntityResetObserver();

    // Serializes the change from
    // Note: This requires your struct to be serializable by glaze - Look at assets/JSON:h for more info
    template <typename T>
    std::string_view EntitySerializeChange(const T& oldState, const T& newState);

    template <typename T>
    bool EntityApplyChange(Entity entity, std::string_view change);

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

    } // namespace internal

    template <typename T, typename Func>
    void EntityObserveChange(Entity entity, Func&& func)
    {
        internal::GLOBAL_OBSERVER.onUpdate<T>(entity, func);
    }

    template <typename T>
    std::string_view EntitySerializeChange(const T& oldState, const T& newState)
    {
        thread_local std::string patchBuffer;
        thread_local std::string compBuffer;
        compBuffer.clear();

        auto patch = glz::merge_diff(oldState, newState).value();
        (void)glz::write<glz::opts{.minified = true}>(patch, patchBuffer);

        if (patchBuffer.size() > 32)
        {
            struct sdefl* sdefl = (struct sdefl*)RL_CALLOC(1, sizeof(struct sdefl));
            int bounds = sdefl_bound(patchBuffer.size());
            compBuffer.resize(bounds);

            int compSize = sdeflate(sdefl, compBuffer.data(), patchBuffer.data(), patchBuffer.size(), 8);
            compBuffer.resize(compSize);

            return compBuffer;
        }

        return patchBuffer;
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

    inline void EntityResetObserver() { internal::GLOBAL_OBSERVER.clear(); }

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


} // namespace magique


#endif // MAGIQUE_OBSERVER_H
