This is a rough outlines of ideas that can be integrated into `magique` in the future.  
Use [GitHub issues](https://github.com/gk646/magique) to suggest new features, and they will be added here.

### Game Dev

**Pathfinding**

- Add strategy API or some kind of customization of the pathfinding based on inputs
    - idea is to allow different profiles: ranged, aggressive melee, skittering, fleeing, ...
- Optimize pathfinding by searching both directions: front->end AND end->front
- for this we need to figure out when a path is found and how to connect the results of both
- this makes pathfinding generally faster and A LOT faster around obstacles

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

**Console**

- make it a true console
- all logging message go to ingame console as well
- default commands
- commands with parameters

### Core

**Use SIMD for higher performance**

- Use [xsimd](https://github.com/xtensor-stack/xsimd) to simplify simd usage
    - Hashgrid: accelerate iteration
    - Pathfinding:
    - Shadows:
    - Collision Detection:

### UI

**More Controls**

- ScrollPane
- DropDown

**Better Default Style**

- Make a consistent default color palette

### Assets

**Better change detection for asset image**

- Use file hashing instead of size and name check
    - Already implemented in ValidateImage()

**Add type checking to asset handles**

- By using ranges (e.g. 0 - 100k for textures) the engine can type check given handles for more security

### Misc

**Modernize**

- Review TaskExecutor and clean it up
    - Spit up tasks to all threads
    - Make internal handling easier and cleaner

**Test**

- add tests persistence, compression and encryption
- collision detection (check against cute.h)