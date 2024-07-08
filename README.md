`magique` is a beginner-friendly 2D engine for programmers (or those yet to be!). It's meant answer all game development needs in a simple but powerful way both in performance and usability. `magique` achieves this by providing core functionalities around a stack of proven libraries to create a productive 2D engine. You like raylib? You like entt? You love `magique`!

Most notable features:

- Every method and header is fully documented including return values, parameters, defaults or failure
- Optimized for maximum usability following a simple code-style and clean headers
- Every engine facility is implemented in the most memory and performance efficient way possible


- Encrypted asset packing and loading into a single asset image
- Built in loading interface handling load-priority and background loading of assets
- Custom wrapper for ECS integration with `entt`
- Particle system following the style of Godot 4
- Persistence interface for game saves and config including a custom database emulator
- Lighting system featuring a hard shadow and raytracing mode through built in ECS components
- Fast and extendable C++ scripting system
- Built in facilities for loading and drawing sprites and tilemaps (supports [Tiled](https://www.mapeditor.org/))

The techstack of `magique`:

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