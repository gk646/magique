`magique` is a 2D engine putting the programming first! It focused on supplying a stable teck stack of libraries that
are curated together to create complex games.

Most notable features:

- Supports visual scripting with the node editor `raynodes` which is run blazingly fast by LuaJIT
- Global memory allocator for performance and control
- Encrypted asset packing for simple distribution
- Powerful and fast ECS integration with `entt`
- Multiplayer support with steamworks api
- Many custom wrappers for Audio and UI

The techstack and feature list of `magique`:

- raylib
    - Drawing, shaders
    - File loading: .mp3, .wav, .png, .gif, models and shaders...
    - Random Numbers: GetRandomInt()
    - Audio
- cute headers
    - 2D collision detection
- entt
    - Entity component system
    - Views, groups, signals...
- tsl
    - Optimized hashmap and hashset
- LuaJIT
    - Lua scripts
- cxstructs
    - helper datastructures and algorithms

Paradigms:

- 2 Thread model
    - Main thread for drawing
    - Update thread for updates
    - Helper threads for additional work

- Fixed update tick rate
    - Default is 60 update per second

#### What `magique` is NOT:

- a 3D engine
- a AAA engine with all possible bells and whistles