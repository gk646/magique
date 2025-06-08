This is a rough outlines of ideas that can be integrated into `magique` in the future.  
Use [GitHub issues](https://github.com/gk646/magique) to suggest new features, and they will be added here.

### Game Dev

**Camera**

- added interface for camera shake
- ueses impulse forces

**Pathfinding**

- Add strategy API or some kind of customization of the pathfinding based on inputs
    - idea is to allow different profiles: ranged, aggressive melee, skittering, fleeing, ...
- Optimize pathfinding by searching both directions: front->end AND end->front
    - for this we need to figure out when a path is found and how to connect the results of both
    - this makes pathfinding generally faster and A LOT faster around obstacles
- Add API to access raw pathfinding grid to do custom algorithms

**Quest System**

- Implemented node based quest system with many predefined quest nodes
- Kill, Collect, GoTo,...
- Quests can then be made via loading a script file or with BuilderPattern in source code

**Procedural Level Generation**

- Give utilities to generate maps based on manually created room cells
- Could use wave function collapse

**Simple Minimap**

- Minimap drawing given a tilemap
- Custom color mappings can be set
    - To tile categories, solid...

**SkillTree**

- Panel with zoom able and draggable background
- Definable skill nodes
- Draw function is implemented by user
- Connections are managed by the engine
- Similar to how you know it from many games e.g. PoE

**Plugins**

- make a new header that helps with creating plugins for the game
- methods for loading and getting methods
- error handling and permission control

**Virtual Clock**

- Add callback at specific timepoint (with repeat option)
- Add callback at x time later from now

**Virtual Calender**

- Implemented on top of virtual clock
- Has support for simulating years, weeks, seasons, lunar seasons...

**Console**

- add tooltip when typing in commands that displays parameters names and the accepted types
    - dynamically highlights the current parameters attributes (like the clion method call tooltip)

### Core

**Use more SIMD for higher performance**

- Use [xsimd](https://github.com/xtensor-stack/xsimd) to simplify simd usage
    - Hashgrid: accelerate iteration
    - Pathfinding:
    - Shadows:
    - Collision Detection:

**Fix/Improve collision detection**

- Add velocity as implicit variable and use it in collision detection/resolving

**More Performance Stats**

- Implement more basic performance stats
    - Longest frame-to-frame, 99% percentile, ...
- Look at performance debug tools
    - Time measurement with macros? for user space functions?
- Memory stats
    - amount of allocations, biggest allocations, ...

### UI

**More Controls**

- ScrollPane
- DropDown

**Default Multiplayer Chat**

- Implement reusable chat for multiplayer with default style

**Menu/Page/Layout Designer**

- Simple class that takes some information and structures them in a layout
- Possible by adding single containers each with dimensions and a layout enum specifier
- Can easily then draw in the containers within the layout

### Assets

**Better change detection for asset image**

- Use file hashing instead of size and name check
    - Already implemented in ValidateImage()

**Add type checking to asset handles**

- By using ranges (e.g. 0 - 100k for textures) the engine can type check given handles for more security

**CSVReader**

- Add statically typed csv reader similar to DataTable to make it easy to work with csv files
- 1 allocation for the string data
- returns a csv file object which is just a vector of the rows with some meta information

### Examples

**Pong**

- Add pong example
- Add local multiplayer as well

### Misc

**Modernize**

- Review TaskExecutor and clean it up
    - Spit up tasks to all threads
    - Make internal handling easier and cleaner

**Test**

- collision detection (check against cute2d.h)