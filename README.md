<img src="logos/magique_banner.png" title="banner"/>

[![Join the chat](https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat)](https://discord.gg/YAsvnxAmX7)
![Version Badge](https://img.shields.io/badge/Version:-0.2.0-blue)
![License Badge](https://img.shields.io/github/license/gk646/magique)

> _What if raylib was an engine? ..._

`magique` is a **C++ 20** beginner-friendly cross-platform 2D game engine for programmers (or those yet to be!).
It's carefully designed to be **intuitive and easy to learn**, at installation through development and up to sharing the
finished game. By using a small **stack of proven libraries** and many **custom crafted modules** it aims to answer all
game development needs in definitive way both in **performance and usability**.

Join the [official discord](https://discord.gg/YAsvnxAmX7) for help, bug reports and discussions.  
Checkout the[ official YouTube channel](https://www.youtube.com/@magique-engine) for spotlights, development sessions
and tutorials.

---

Most notable features:

- **Easy to learn and use**
    - **No external dependencies!** (all dependencies in the source)
    - All `raylib` functionalities are the same allowing for an easy transition
    - _**_Fully documented headers_**_ and GitHub wiki
    - **Many examples** from single headers to whole games! (see _examples/_)
    - Intuitive public API optimized for maximum usability
    - Robust logging and error handling with debugger
- **Fast and modern**
    - Internally uses _**custom datastructures and algorithms**_ to provide maximum performance
    - Explicitly _**optimized for compile time**_ internally and externally!
    - Takes advantage of _**multithreading, SIMD, data-driven design and cache locality**_ where possible
- **Large feature set with `pay for what you use` policy**
    - Asset-Packing and Loading with **compression and encryption support**
    - Automatic texture stitching into configurable atlases
    - Task based loading interface handling load-priority and background loading of assets
    - _**Procedural particle system**_ inspired by Godot4
    - Persistence API for gamesaves, gameconfigs and typesafe database emulator
    - Typesafe C++ scripting system allowing for custom methods and classes with native speed
    - Out of the box support for _**[Tiled](https://www.mapeditor.org/) exports**_ (.tmx, .tsx)
    - Object-oriented UI framework with automatic scaling and intuitive anchoring and alignment tools
    - _**Achievements, InGame-CommandLine, NoiseGeneration, 2DSound, AIAgent, VirtualClock, and many more!**_
- **Multiplayer support**
    - Seamless transition between local and global networking with a custom unified API
- **Steam integration**
    - Provided a Steam SDK, allows access to _**steam features**_ like matchmaking and chat

`magique` is using the following popular libraries:

- **[raylib](https://github.com/raysan5/raylib):** rendering, resource loading, sound,...
- **[EnTT](https://github.com/skypjack/entt):** fast and modern entity-component system
- **[ankerl](https://github.com/martinus/unordered_dense):** optimized dense hashmap and hashset
- **[FastNoiseLite](https://github.com/Auburn/FastNoiseLite):** fast and easy noise generation
- **[GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets):** local networking via IP
- **[Steam SDK](https://partner.steamgames.com/)** _(if provided)_:  steam features and global P2P networking

### Minimal Example

```c++
using namespace magique;
struct MyGame final : Game
{
    void updateGame(GameState gameState) override { printf("Hello World!\n"); }
    void drawGame(GameState gameState, Camera2D& camera) override { DrawRectangle(0, 0, 50, 50, RED); }
};

int main()
{
    MyGame game{};
    return game.run();
}
```

### 1. Is magique for you?

`magique` is aimed at **lower-level game development** in C++. It's intended to be the starting point for a game project
and meant to be **extended by user written code**!
As such it does not have all solutions for any game! Instead, it tries to offer many optional general purpose
modules that find usage in a multitude of projects. However, there are some **fundamental principles and restrictions**
that you have to follow when using magique:

- Game template
    - You have to overwrite the core/Game.h class and implement your game logic in its virtual methods
- ECS (entity component system)
    - Every game object is supposed to be an entity with components
    - The PositionC component is implicit for every object! (look at _ecs/Components.h_)
- Scripting (Collisions)
    - For now the only way to react to engine internal events like static/dynamic collisions is to use the provided C++
      scripting system
    - Collisions are detected internally and the collision point, normal vector and penetration depth can be processed
      by the user

Other than the listed points magique is very modular and customizable and many modules can be disabled or replaced by
user code.

**_Even if you don't want to use the whole engine there are plenty of interesting concepts within magique that you can use or learn from.**_

### 2. Installation

`magique` is using CMake as its build system hence it's recommended for projects to use it as well.
Configuration of the library happens inside `include/magique/config.h` (documented in source).

#### CMake (recommended)

When using CMake just download or clone this repository and add it as subdirectory with the path:

```cmake
add_subdirectory(path/to/magique)
# Your project add_executable(MyGame main.cpp)
target_link_libraries(MyGame PRIVATE magique)
```

This will automatically build magique with your project and set up the include path. Don't forget to link your project
against `magique`! This approach will automatically build for the platform your using.
Inside the `CMakeLists.txt` you can configure the build setup e.g. enabling Steam, sanitizer...

####  Custom or prebuilt binaries

Doing a custom build even without CMake should be very straightforward:  
_Compile all files in src into a single library with /include as include path._ _(conditionally exclude steam or multiplayer if you're not using it)_

If released you can download the latest binaries, link against them and just copy the include folder into your project.

_**Feel free to copy single headers, implementations or modules in any combination, as the license allows. A lot of the
modules can easily be made into standalone units.**_

### 3. Documentation

There are 3 ways `magique` is documented:

- **In-Header Documentation**
    - Each public function or struct comes with a comment or description
    - Tags are used to provide more insight:
        - `Note:` A helpful sidenote about usage, behavior, relation to other methods or common errors
        - `Default:` The default value or behavior for that method or struct
        - `Failure:` The behavior or return value if the method fails its purpose
        - `Example:` Shows how this method or struct is correctly used
        - `IMPORTANT:` Points out crucial information like unique behavior or pitfalls
- **[YouTube Channel](https://www.youtube.com/@magique-engine)**
    - Contains tutorial, walkthrough, showcase and development videos
- **GitHub Wiki**
    - Written like a tutorial and focuses on general examples rather than individual methods
    - Each module has its own page plus additional pages for combined functionality and extras
    - Might be incomplete

If you're a newcomer to gamedev or C++ you should start with the Getting Started page in the wiki.
Contrary if you have experience with the concepts the in-header documentation will likely be enough to guide you.

### 4. Paradigms and design philosophy of `magique`

> *Software has no functionality if it cannot be used!*

The core philosophy behind `magique` is usability. Using libraries or software should be understandable and intuitive.
In this case usability means you think about the users mental state, their existing knowledge and goals. Then you provide them with what they need to help them achieve their goal without restricting their freedom. On top of that, by adhering to formatting, naming and implementation rules you allow the user to build a mental image of what to expect. Then, when presented new functionality (headers, modules) its already clear how to use them and what they likely do internally to achieve that.


### API Design

Why should magique be

#### Threading
`magique` started out with 2 threads, one for drawing the other for updates. This causes many issues like the two threads being de-synced so updates happen while rendering is going on. Also, precise input handling becomes a problem if the threads go at different tick rates.
Then I found a way to still have this notion of update and draw tick at different arbitrary tick rates on the same thread.
The assumption is that both ticks fit within the timeframe set by the render rate.
Example with 120fps and 60 logic tick rate:
The whole thread can take at most 8ms (from the render thread) otherwise the next tick is delayed.
Next you find out the ratio at which logic ticks occurs -> 120/60 = 2 -> every two render ticks a update tick has to happen
This means a update tick doesn't happen precisely every 16ms but its still consistent over the long run (or across 1 second).
To smooth out the additional update time ever 2 ticks i save the time it took each time. Then if next tick a update tick occurs i preemptively sleep less so the next thread tick has longer. This again means a render tick doesnt happen every 8ms precisely but is still generally.
The limitation to this approach is that the ticks cannot take as long as they can usually. If the render tick already 8ms the update tick will always delay the next thread tick. Which in turn delays render and update ticks.
The higher the framerate the more this becomes apparent: 60fps -> 16ms time  | 120fps -> 8ms | 240fps -> 4ms
But for the kind of games this engine is for this approach should be very good.

### Entity Component System (ECS)

`magique` uses the concept of a ecs. Handling game objects as entities with components should in almost all cases be better over other approach as it nudges you to design modular systems and diverse interactions. If made object-oriented hierarchies in the past and it almost always turns out to be a mess and there's nothing you cant do with an ECS. Also, the very nice typesafety of EnTT makes it a joy to use in C++.