### Folder structure

#### src/

All sub folders should only contain implementation files of their public interface header. E.g. magique/core/Core.h ->
src/core/Core.cpp. If the include folder has a subfolder so should the implementation e.g
magique/assets/types -> src/assets/types

Three exceptions:

- The root can contain folders for other files
- CMakeLists.txt
- internal module can be more chaotic

The folders should follow a specific naming convention:

- headers: Contains headers that help with the implementation of the source files

Other than that choose a descriptive name based on their function.
Existing ones: systems, globals, datastructures

### Include style

The include order and style is the following:

- 1: third-party or STL includes with `<..>`
- 2: first party (./include/) includes with `<..>`
- 3: internal includes with `"..."`

Always include from the root and specify the full include path. No relative includes. After every category one newline.

Example:

```cpp
#include <entt/registry.hpp>

#include <magique/core/Core.h>

#include "headers/Helper.h"
#include "headers/Helper2.h"
```

### Types

Use fast_vector internally by including:
`#include "internal/datastructures/VectorType.h"`

It can be slightly faster, but compiles faster and less templates and more control