`magique` is a beginner-friendly 2D engine for programmers (or those yet to be!). It's meant answer all game development needs in a simple but powerful way both in performance and usability. magique achieves this  while providing core functionalities around p to create a productive 2D engine.
are curated together to create complex games.

Most notable features:

- Every method and header is fully documented including return values, parameters, defaults or failure
- Optimized for usability following a simple code-style
- Low-level implementation
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
- ankerl
    - Optimized hashmap and hashset

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