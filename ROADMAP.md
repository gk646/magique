This is a rough outlines of ideas that can be integrated into `magique` in the future.  
Use [GitHub issues](https://github.com/gk646/magique) to suggest new features, and they will be added here.

### Game Dev

Pathfinding

- Add strategy API or some kind of customization of the pathfinding based on inputs
  - idea is to allow different profiles: ranged, aggressive melee, skittering, fleeing, ...
- Optimize pathfinding by searching both directions: front->end AND end->front
- for this we need to figure out when a path is found and how to connect the results of both
- this makes pathfinding generally faster and A LOT faster around obstacles

Quest System

- Implemented node based quest system with many predefined quest nodes
- Kill, Collect, GoTo,...
- Quests can then be made via loading a script file or with BuilderPattern in source code

Procedural Level Generation
- Give utilities to generate maps based on manually created room cells
- Could use wave function collapse

### Core

### Misc

### Buildsystem