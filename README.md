[![Discord](https://img.shields.io/badge/Discord-%235865F2.svg?&logo=discord&logoColor=white)](https://discord.gg/YAsvnxAmX7)

STILL IN BETA STATE - NOT PUBLICLY RELEASED

`magique` is a beginner-friendly 2D engine for programmers (or those yet to be!). It's meant to answer all game
development
needs in a simple but definitive way both in performance and usability. `magique` achieves this by providing core
functionalities around a stack of proven libraries to create a productive 2D engine in C++ 20.

Most notable features:

- Every method and header is fully documented often including return values, parameters, defaults or failure
- Optimized for maximum usability following a simple code-style and clean headers
- Every engine facility is implemented in the most memory and performance efficient way possible
- Encrypted and compressed asset packing and loading into a single asset image
- Automatic texture stitching into a different texture atlases
- Built in loading interface handling load-priority and background loading of assets
- Particle system similar to Godot 4
- Persistence interface for game saves and config including a custom database emulator
- Lighting system featuring a hard shadow and raytracing mode through built in ECS components
- Unique C++ scripting system
- New types and methods to load and draw sprites and tile maps (supports [Tiled](https://www.mapeditor.org/) exports)

#### What magique IS:

- A free, fast and easy-to-use open-source 2D engine
- An engine with solutions similar engines usually don't have: asset-packing, asset-loading, creating and saving
  game saves/configs
- A C++ library requiring some initial understanding of C++

#### What `magique` is NOT:

- A 3D engine
- A AAA engine with all possible bells and whistles
- An engine for complete C++ beginners

#### Minimal Example

```cpp
using namespace magique:
struct MyGame : Game{

    

};


int main(){

MyGame game{};
return game.run();

}
```

### 1. Installation

There are 2 supported ways of installing `magique`:

#### CMake (recommended)

When using CMake just download or clone this repository and add the directory to your project with:

```cmake
add_subdirectory(path/to/magique)
# Your project add_executable(MyGame main.cpp)
target_link_libraries(MyGame PRIVATE magique)
```

This will automatically build magique with your project and set up the include path. Don't forget to link your project
against magique! This approach will automatically build for the platform your using.
Check the wiki for optional CMake flags you can set before adding magique.

#### Prebuilt binaries / Custom

The other option is to download the prebuilt binaries for your platform and link your program against them.

Feel free to copy single headers, implementations or modules in any combination, as the license allows. A lot of the
modules can easily be
made into
standalone units.

### 2. Documentation

There are 2 main ways magique is documented:

- **In-Header Documentation**
    - Each public function or struct comes with a comment or description
    - Tags are used to provide more insight:
        - `Note:` A helpful sidenote about usage, behavior, relation to other methods or common errors
        - `Default:` The default value or behavior for that method or struct
        - `Failure:` The behavior or return value if the method fails its purpose
        - `Example:` Shows how this method or struct is correctly used
        - `IMPORTANT:` Points out crucial information like unique behavior or pitfalls
- **GitHub Wiki**
    - Written like a tutorial and focuses on general examples rather than individual methods
    - Each module has its own page plus additional pages for combined functionality and extras

If you're a newcomer to gamedev or C++ you should start with the Getting Started page in the wiki.
Contrary if you have experience with the concepts the in-header documentation will likely be enough to guide you.

### 3. The tech-stack of `magique`:

- raylib
    - Drawing, shaders
    - File loading: .mp3, .wav, .png, .gif, models and shaders...
    - Random Numbers: GetRandomValue()
    - Audio
- EnTT
    - Entity component system
    - Views, groups, signals...
- ankerl
    - Optimized hashmap and hashset
- FastNoiseLite
    - Fast and customizable noise generation

Paradigms:

- 1 Main Thread + Job System
    - Main thread handles calling render and update functions correctly
    - Job system allows to submit and await concurrent work packages

- Fixed update tick rate
    - Default is 60 updates per second
- Variable render rate
    - Supports any framerate (60-240fps)
- Entity Component System