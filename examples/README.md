## Examples

Examples are split into three **categories:**

- ### Games:
    - Full game examples with their own build system and resources
- ### Headers:
  - Single headers meant to be included in an empty magique project:
    ```c++
    #include "ExampleHeader.h"
    int main()
    {
    Example test{};
    return test.run();
    }
    ```
  - Showcase a specific functionality and how its used within magique
- ### Demos:
    - Like headers, they showcase functionality but need their own build system and resources