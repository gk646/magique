## Contributing to `magique`

Iam very happy about any contributions, specifically:

- Improving the documentation in the wiki or in the source files
- Reviewing code and fixing mistakes (leaks, rare bugs)
- Improving performance or usability of existing code
- Testing and bug reports!
- Adding new features and completely new modules

Everything besides new features will likely be merged instantly after review.

### Contributing New Features

If you want to contribute a new feature check the [roadmap](https://github.com/gk646/magique/blob/main/ROADMAP.md) first.
If you cant find it in the roadmap consider the following points before adding it:

- Is the new feature highly usable
    - This means intuitive, logical and clearly formulated
    - It can be more complex if it's optional and not essential
- Does the new feature solve a new problem
    - Make sure it's not already solved
- Is the new feature in a state that it won't be replaced soon
    - The feature and its use cases should be evaluated from different angles

Drafts and suggestions are very welcome all the time!

### Folder structure

#### src/

All sub folders should only contain implementation files of their public interface header:

- `magique/core/Core.h -> src/core/Core.cpp`

If the include folder has a subfolder the implementation avoids this:

- `magique/assets/types/Playlist.h -> src/assets/Playlist.cpp`

This is to avoid confusion with other folders in the source directory.

Other than implementation files the source directory should only contain select folders:

- **headers**: Contains headers that help with the implementation of the source files

**Exception**: src/internal is also the place to put all internal code used throughout the modules and has multiple
folders

### Code Style

Use the `.clang-format` file at the root of the project to format all source code (excluding dependencies).

Specific code rules:

- Whenever casting use `static_cast<T>()` over the C-style cast `(T)`

### Include style

#### For implementation files

The include order and style is the following:

- 1: third-party or STL includes with `<..>`
- 2: first party (/include/magique) includes with `<..>`
- 3: internal includes with `"..."`

Always include from the root and specify the full include path. No relative includes. After every category one newline.

Example:

```cpp
#include <entt/registry.hpp>

#include <magique/core/Core.h>

#include "headers/Helper.h"
#include "headers/Helper2.h"
```

#### For public headers

Same as for implementation files but without new lines between the categories to save space.

### Types

``

It can be slightly faster, but compiles faster and uses fewer templates and more allows control (e.g. unordered erase).
Don't expose it to the user, which means dont use it in public structs or when you return a vector to the user.