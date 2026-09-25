<img src="logos/magique_banner.png" title="banner"/>

[![Join the chat](https://img.shields.io/badge/discord-join-7289DA.svg?logo=discord&longCache=true&style=flat)](https://discord.gg/YAsvnxAmX7)
![Version Badge](https://img.shields.io/badge/Version:-0.9.41-blue)
[![License: Zlib](https://img.shields.io/badge/License-Zlib-lightgrey.svg)](https://opensource.org/licenses/Zlib)

`magique` is a **C++ 26** beginner-friendly cross-platform 2D engine for programmers (or those yet to
be!).
It combines your favourite game libraries (
*[raylib](https://github.com/raysan5/raylib), [EnTT](https://github.com/skypjack/entt) and more*) with **many custom
written modules** into a featureful standalone engine.
With a **carefully designed API** and **extensive documentation** it is the perfect basis for every C++ 2D game project.

> No fancy interface, no visual helpers, no gui tools or editors... just coding in pure spartan-programmers way.
> Are you ready to enjoy coding?
>
> -- *raylib*

Join the [official discord](https://discord.gg/YAsvnxAmX7) for help, bug reports and discussions.  
Checkout the `magique` [YouTube channel](https://www.youtube.com/@gk646-yt) for spotlights, development sessions
and tutorials.

#### Made with magique

|                                                     [Turtle's Journey](https://github.com/gk646/turtles_journey)                                                     | [Space Fighter](https://store.steampowered.com/app/4695600?utm_source=github) (Steam)                                                                                                                                                             |
|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------:|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| <a href="https://github.com/gk646/turtles_journey"><img src="https://github.com/gk646/turtles_journey/blob/main/pics/main_menu.png" alt="Screenshot 1" width="250"/> | <a href="https://store.steampowered.com/app/4695600?utm_source=github"><img src="https://shared.fastly.steamstatic.com/store_item_assets/steam/apps/4695600/5d8bab783ca6bc4cc2fa3a8fbc3d7e1c6af4aa49/header.jpg" alt="Screenshot 1" width="250"/> |

---

### Feature highlights:

- **Easy to learn and use**
    - _**No external dependencies!**_ (all batteries included)
    - _**Fully documented headers**_, many **_examples_** and **_video guides_**!
    - Supports _**[Tiled](https://www.mapeditor.org/)**_ _(.tmj, .tsj)_ and [_**aseprite**_](https://www.aseprite.org/)
      (_.ase_) formats
- **Fast and modern**
    - Uses _**custom datastructures and algorithms**_ to provide maximum performance
    - Takes advantage of _**multithreading, data-driven design and cache locality**_
    - Uses latest C++26 reflection features to improve development workflow
- **Large feature set with `pay for what you use` policy**
    - Blazingly fast **2D raytracing** with soft shadows for any shape
    - Integrated Asset-Packing and Loading with **compression, encryption and checksum support**
    - Automatic texture stitching into configurable atlases
    - _**Procedural particle system**_ inspired by Godot 4
    - Object-oriented UI framework with automatic **scaling, anchoring and alignment tools**
    - Powerful **in-engine console** with custom commands that can modify the gamestate
    - _PathFinding, Keybindings, Localization, 2DSound_ and many more!
- **Networking**
    - _**Unified API**_ that allows to seamlessly switch between Steam or GameNetworkingSockets for local and global
      networking
- **Steam integration**
    - Intuitive wrapper around the SteamSDK to access like _**lobbies, stats, matchmaking and chat**_

`magique` is using the following popular libraries:

- **[raylib](https://github.com/raysan5/raylib)** _(6.1-dev)_: rendering, resource loading, sound,...
- **[EnTT](https://github.com/skypjack/entt)** _(4.0.0)_: fast and modern entity-component system
- **[unordered_dense](https://github.com/martinus/unordered_dense)** _(4.8.1)_: optimized dense hashmap and hashset
- **[FastNoiseLite](https://github.com/Auburn/FastNoiseLite)** _(1.1.1)_: fast and easy noise generation
- **[GameNetworkingSockets](https://github.com/ValveSoftware/GameNetworkingSockets)** _(1.41-dev)_: local networking via IP
- **[Steam SDK](https://partner.steamgames.com/)** _(1.65)_:  steam features and global P2P networking
- **[glaze](https://github.com/stephenberry/glaze)** _(8.3.0)_: Fast JSON serialization with reflection support
- **[cute_headers](https://github.com/RandyGaul/cute_headers)** _(latest)_: _cute_tiled.h_ and _cute_asprite.h_ for
  resource loading
- **[blake3-tiny](https://github.com/michaelforney/blake3-tiny)** _(latest)_: BLAKE3 hashing
- **[tiny-AES](https://github.com/kokke/tiny-AES-c)** _(latest)_: AES encryption

### Quickstart

Checkout the [video guide](https://www.youtube.com/watch?v=6mQwd6NjNcw) for a step by step explanation!

1. Set up an emtpy new project folder and create the following files:

- **_main.cpp_**
  ```c++
  #include <magique/magique.hpp>
  struct MyGame final : Game
  {
      void onUpdateGame(GameState state) override { printf("Hello World!\n"); }
      void onDrawGame(GameState state, Camera2D& camera) override { DrawRectangleRec(Rect::CenteredOn(GetScreenDims() / 2, 50), RED) }
  };
  
  int main()
  {
      MyGame game{};
      return game.run();
  }
  ```

- _**CMakeLists.txt**_

  ```cmake
  cmake_minimum_required(VERSION 3.20)
  project(game)
  
  add_executable(game main.cpp) # Add your executable
  
  # Automatically downloads the latest magique
  include(FetchContent)
  FetchContent_Declare(magique GIT_REPOSITORY https://github.com/gk646/magique GIT_TAG main)
  FetchContent_MakeAvailable(magique)
  
  # Link your project against magique - automatically sets up include paths and dependencies
  target_link_libraries(game PUBLIC magique) 
  ```

3. To build your game executable inside a build folder call this via a terminal in the new folder:

```shell
mkdir build && cd build
cmake ..
make -j 8 && ./game
```

**With these simple steps you created your first project with `magique`!**

### Is magique for you?

`magique` is aimed at **lower-level game development** in C++. It's intended to be the starting point for a game project
and meant to be **extended by user written code**! As such it does not have all solutions for any game! Instead, it
tries to offer many _optional_ general purpose modules that find usage in a multitude of projects. However, there are some
**fundamental principles and restrictions** that you have to follow when using magique:

- **Game template**
    - You have to overwrite the `core/Game.h:Game` class and implement your game logic in its virtual methods
- **ECS** (entity component system)
    - Every game object is supposed to be an entity with components
    - The PositionC component is implicit for every object! (see: `ecs/Components.h`)
- **No Advanced Physics**
    - There is no builtin physics engine apart from your reaction to a collision
        - You can of course disable integrated collision handling and roll your own physics engine

Other than the listed points magique is very modular and customizable and many modules can be disabled or replaced by
user code with _no overhead_.

_**Even if you don't want to use the whole engine there are plenty of interesting concepts within `magique` that you can
use (copy) or learn from.**_

### Documentation

There are 3 ways `magique` is documented:

- **In-Header Documentation**
    - Each public module, function and struct comes with a comment or description
    - Tags are used to provide more insight:
        - `Note:` A helpful sidenote about usage, behavior, relation to other methods or common errors
        - `Default:` The default value or behavior for that method or struct
        - `Failure:` The behavior or return value if the method fails its purpose
        - `Example:` Shows how this method or struct is correctly used
        - `IMPORTANT:` Points out crucial information like unique behavior or pitfalls
- **[magique YouTube](https://www.youtube.com/@gk646-yt)**
    - Contains tutorials, showcase and development videos
- **Examples**
    - There are many examples in `examples/` which come in 3 categories:
        - _**games**_: complete games from resources to gameplay showcasing many features
        - _**demos**_: showcase individual functionality that need their own resources
        - _**headers**_: a single header meant to be included in an empty project (see [QuickStart](#QuickStart))

`magique` is designed to be fully understandable and usable with the documentation provided in the source.

### Paradigms and design philosophy

> *Software has no functionality if it cannot be used!*

The core philosophy behind `magique` is usability. Using libraries or software should be simple and intuitive.
In this case usability means you think about the users mental state, their existing knowledge and goals. Then you
provide them with what they need to help them achieve their goal without restricting their freedom. On top of that, by
adhering to formatting, naming and implementation rules you allow the user to build a mental image of what to expect.
Then, when presented new functionality (headers, modules) its already clear how to use them and what they likely do
internally to achieve that.

#### API Design

> _What if raylib was an engine? ..._

`magique` follows the API design of raylib. This means a clear pascal case naming
scheme with descriptive names and baked in namespacing: e.g. FormatSet (), FormatGet (). Similarly, every public method
struct or enum is documented and each module has a compact documentation header as well. This documentation is also
enriched with tips or warnings about usage or side effects. 
The API also specifically only tries to rely on well known types like std::vector or std::string.
If explicitly needed magique exposes types that are completely
used managed or defined (e.g. EntityType, AtlasID, ...). This is done with care as the library has less control over
their usage behavior, and it might not be clear in what way this type is intended to be used.

#### Threading

`magique` started out with 2 threads, one for drawing the other for updates. This causes many issues like the two
threads being de-synced so updates happen while rendering is going on. Also, precise input handling becomes a problem if
the threads go at different tick rates.
Then I found a way to still have this notion of update and draw tick at different arbitrary tick rates on the same
thread.
The assumption is that both ticks fit within the timeframe set by the render rate.
Example with 120fps and 60 logic tick rate:
The whole thread can take at most 8ms (from the render thread) otherwise the next tick is delayed.
Next you find out the ratio at which logic ticks occurs -> 120/60 = 2 -> every two render ticks an update tick has to
happen
This means an update tick doesn't happen precisely every 16ms but its still consistent over the long run (or across 1
second).
To smooth out the additional update time ever 2 ticks I save the time it took each time. Then if next tick an update
tick
occurs I preemptively sleep less so the next thread tick has longer. This again means a render tick doesn't happen every
8ms precisely but is still generally.
The limitation to this approach is that the ticks cannot take as long as they can usually. If the render tick already
8ms the update tick will always delay the next thread tick. Which in turn delays render and update ticks.
The higher the framerate the more this becomes apparent: 60fps -> 16ms time | 120fps -> 8ms | 240fps -> 4ms
But for the kind of games this engine is for this approach should be very good.

#### Entity Component System (ECS)

`magique` uses the concept of an ecs. Handling game objects as entities with components should in almost all cases be
better over other approach as it nudges you to design modular systems and diverse interactions. I have made
object-oriented
hierarchies in the past, and it almost always turns out to be a mess and there's nothing you cant do with an ECS. Also,
the very nice typesafety and compile-time features of EnTT makes it a joy to use in C++.

#### Scripting

I initially started with Lua as a scripting language. But the main problem I encountered is that by allowing users to
create their own components and functions it's very hard to create a good scripting interface. You would then have to
auto export those types possibly with a macro. This problem doesn't
occur in other engines as they know ALL their components upfront and don't expose a lot of internal workings.
This approach allows for any custom type inside C++, while keeping the configuration manageable and type safe!

### Further Resources

- [raylib wiki](https://github.com/raysan5/raylib/wiki)
- [raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html) *(all methods)*
- [entt wiki](https://github.com/skypjack/entt/wiki)
- [glaze wiki](https://stephenberry.github.io/glaze/json/)