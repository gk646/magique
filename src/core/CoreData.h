#pragma once
#ifndef COREDATA_H
#define COREDATA_H

#include <thread>
#include <raylib/raylib.h>
#include <cxstructs/StackVector.h>

#include <magique/util/TaskExecutor.h>
#include <entt/entity/registry.hpp>
#include <magique/util/DataStructures.h>

#include "InternalTypes.h"
#include "perf/Perf.h"

#include <magique/assets/AssetManager.h>


// All the core data that is shared between internal implementations
// We carry a slog of includes around but dont really care as its statically compiled
// Library compile time isnt a user cost

namespace magique
{
    // Logic thread
    inline std::thread LOGIC_THREAD;

    // Internal registry
    inline entt::registry ENTT_REGISTRY;

    // Type map
    inline HashMap<EntityType, std::function<void(entt::registry&, entt::entity)>> ENT_TYPE_MAP;

    // Current logic data
    inline LogicTickData LOGIC_TICK_DATA;

    // Current draw data
    inline DrawTickData DRAW_TICK_DATA;

    // Active game loader if any
    inline IExecutor* CURRENT_GAME_LOADER = nullptr;

    // Performance metrics
    inline PerfData PERF_DATA;

    // Engine configuration flags
    inline Configuration CONFIGURATION;

    // Asset manager
    inline AssetManager<Sound, Music, Shader, TextureRegion> ASSET_MANAGER;

    // Texture atlases
    inline cxstructs::StackVector<TextureAtlas, CUSTOM_2 + 1> TEXTURE_ATLASES;

} // namespace magique
#endif //COREDATA_H