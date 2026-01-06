#include <magique/ui/Menu.h>
#include "raylib/raylib.h"

namespace magique
{

    void Menu::switchToParent()
    {

    }

    void Menu::updateInputs()
    {
        if (IsMouseButtonPressed(KEY_ESCAPE))
        {
            switchToParent();
        }
    }

} // namespace magique
