#ifndef COREDATA_H
#define COREDATA_H

#include <magique/fwd.hpp>

// This has been optimized for library build size and compile time
// -> All heavy global headers are in the /global folder
// These are commented out

namespace magique::global
{
    // Logic thread
    //inline std::thread LOGIC_THREAD;

    // Current draw data
    //inline DrawTickData DRAW_TICK_DATA;

    // Active game loader if any
    inline IExecutor* CURRENT_GAME_LOADER = nullptr;

    // Performance metrics
    //inline PerfData PERF_DATA;

    // Engine configuration flags
    // inline Configuration CONFIGURATION;

    // Asset manager
    // inline AssetManager<Sound, Music, Shader, TextureRegion, SpriteSheet> ASSET_MANAGER;

    // Shaders
    // inline Shaders SHADERS;

    // Texture atlases
    // inline cxstructs::StackVector<TextureAtlas, CUSTOM_2 + 1> TEXTURE_ATLASES;

    // Type map
    // inline HashMap<EntityID, std::function<void(entt::registry&, entt::entity)>> ENT_TYPE_MAP{50};

    // Current logic data
    // inline LogicTickData LOGIC_TICK_DATA;
} // namespace magique::global
#endif //COREDATA_H