#include <magique/ui/controls/Window.h>

namespace magique
{

    Window::Window(const float x, const float y, const float w, const float h, const float moverHeight) :
        UIContainer(x, y, w, h)
    {
        moverHeightP = moverHeight / h;
    }

} // namespace magique