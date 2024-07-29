[![Discord](https://img.shields.io/badge/Discord-%235865F2.svg?&logo=discord&logoColor=white)](https://discord.gg/YAsvnxAmX7)

STILL IN ALPHA STATE - NOT PUBLICLY RELEASED

`magique` is a beginner-friendly 2D engine for programmers (or those yet to be!). It's meant answer all game development
needs in a simple but definitive way both in performance and usability. `magique` achieves this by providing core
functionalities around a stack of proven libraries to create a productive 2D engine. You like raylib? You like C++? You
will love `magique`!

Most notable features:

- Every method and header is fully documented including return values, parameters, defaults or failure
- Optimized for maximum usability following a simple code-style and clean headers
- Every engine facility is implemented in the most memory and performance efficient way possible
- Encrypted asset packing and loading into a single asset image
- Automatic texture stitching into a different texture atlases
- Built in loading interface handling load-priority and background loading of assets
- Particle system similar to Godot 4
- Persistence interface for game saves and config including a custom database emulator
- Lighting system featuring a hard shadow and raytracing mode through built in ECS components
- Unique C++ scripting system
- New types and methods to load and draw sprites and tilemaps (supports [Tiled](https://www.mapeditor.org/) exports)

#### What magique IS:

- A free, fast and easy-to-use open-source 2D engine
- An engine with solutions other engines usually don't have: asset-packing, asset-loading, creating and saving
  game saves/config
- A C++ library requiring some initial understanding of C++

#### What `magique` is NOT:

- A 3D engine
- An AAA engine with all possible bells and whistles
- An engine for complete C++ beginners

### 1. Installation

### 2. The Wiki

### 2. The tech-stack of `magique`:

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
- Entity Component System