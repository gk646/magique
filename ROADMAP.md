This is a rough outlines of ideas that can be integrated into `magique` in the future.  
Use [GitHub issues](https://github.com/gk646/magique) to suggest new features, and they will be added here.

### Game Dev

**Camera**

- added interface for camera shake
- ueses impulse forces

**Pathfinding**

- Optimize pathfinding by searching both directions: front->end AND end->front
    - for this we need to figure out when a path is found and how to connect the results of both
    - this makes pathfinding generally faster and A LOT faster around obstacles


**Procedural Level Generation**

- Give utilities to generate maps based on manually created room cells
- Could use wave function collapse

**Simple Minimap**

- Minimap drawing given a tilemap
- Custom color mappings can be set
    - To tile categories, solid...

**Canvas UI Controls**

- Panel with zoom able and draggable background

**Plugins**

- make a new header that helps with creating plugins for the game
- methods for loading and getting methods
- error handling and permission control
- => what language (cant be c++ due to safety concerns: prob lua or python or Java?)

### Core

**Use more SIMD for higher performance**

- Use [xsimd](https://github.com/xtensor-stack/xsimd) to simplify simd usage
    - Hashgrid: accelerate iteration
    - Pathfinding:
    - Shadows:
    - Collision Detection:

**More Performance Stats**

- Implement more basic performance stats
    - Longest frame-to-frame, 99% percentile, ...

### UI

**More Controls**

- DropDown

**Default Multiplayer Chat**

- Implement reusable chat for multiplayer with default style

**Menu/Page/Layout Designer**

- Simple class that takes some information and structures them in a layout
- Possible by adding single containers each with dimensions and a layout enum specifier
- Can easily then draw in the containers within the layout

### Assets

**Better change detection for asset pack**

- Use file hashing instead of size and name check
    - Already implemented in ValidateImage()


### Examples

**Pong**

- Add pong example
- Add local multiplayer as well

### Misc

**Modernize**

- Review TaskExecutor and clean it up
    - Spit up tasks to all threads
    - Make internal handling easier and cleaner
