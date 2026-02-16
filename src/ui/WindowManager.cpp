// SPDX-License-Identifier: zlib-acknowledgement
#include <deque>
#include <algorithm>

#include <magique/ui/WindowManager.h>
#include <magique/ui/controls/Window.h>
#include <magique/util/Logging.h>

#include "internal/globals/UIData.h"

namespace magique
{
    struct WindowManagerData final
    {
        std::vector<Window*> windows;
        HashSet<const Window*> shownSet;
        Window* hoveredWindow = nullptr;

        struct MoveCommand final
        {
            Window* move;
            Window* inFrontOf;
        };
        std::deque<MoveCommand> moveFrontQueue;
    };

    inline WindowManagerData WINDOW_DATA{};
    inline WindowManager WINDOW_MANAGER{};

    WindowManager& WindowManagerGet() { return WINDOW_MANAGER; }

    void WindowManager::draw()
    {
        for (int i = 0; i < (int)WINDOW_DATA.windows.size(); ++i)
        {
            auto* w = WINDOW_DATA.windows[i];
            if (WINDOW_DATA.shownSet.contains(w))
            {
                w->draw();
            }
        }
    }

    void WindowManager::addWindow(Window* window)
    {
        if (std::ranges::contains(WINDOW_DATA.windows, window))
        {
            LOG_WARNING("Window already managed: %s", window->getName().c_str());
            return;
        }
        WINDOW_DATA.windows.emplace_back(window);
        setShown(window, true);
    }

    Window* WindowManager::getWindow(const char* name)
    {
        MAGIQUE_ASSERT(name != nullptr, "Passed name cannot be null");
        auto it = std::ranges::find_if(WINDOW_DATA.windows, [&](Window* e) { return e->getName() == name; });
        if (it != WINDOW_DATA.windows.end())
        {
            return *it;
        }
        LOG_WARNING("No tracked window with such name: %s", name);
        return nullptr;
    }

    bool WindowManager::removeWindow(Window* window)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        return std::erase(WINDOW_DATA.windows, window) > 0;
    }

    void WindowManager::setShown(Window* window, const bool shown)
    {
        if (window == nullptr)
        {
            return;
        }
        if (shown)
        {
            WINDOW_DATA.shownSet.insert(window);
        }
        else
        {
            WINDOW_DATA.shownSet.erase(window);
        }
    }

    bool WindowManager::getIsShown(Window* window)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        return WINDOW_DATA.shownSet.contains(window);
    }

    void WindowManager::toggleShown(Window* window)
    {
        if (window == nullptr)
        {
            return;
        }
        if (WINDOW_DATA.shownSet.contains(window))
        {
            WINDOW_DATA.shownSet.erase(window);
        }
        else
        {
            WINDOW_DATA.shownSet.insert(window);
        }
    }

    const std::vector<Window*>& WindowManager::getWindows() { return WINDOW_DATA.windows; }

    void WindowManager::moveInFrontOf(Window* moved, Window* inFrontOf)
    {
        if (moved == nullptr || inFrontOf == nullptr)
        {
            return;
        }
        WINDOW_DATA.moveFrontQueue.push_back({moved, inFrontOf});
    }

    bool WindowManager::getIsCovered(Window* window, const Point pos)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        // Iterated in reverse - last drawn is the front most window - return as soon as we find obstruction
        for (auto it = WINDOW_DATA.windows.rbegin(); it != WINDOW_DATA.windows.rend(); ++it)
        {
            if (getIsShown(*it)) // only search visible ones
            {
                if (*it == window) // We found the given window - nothing is in front
                    return false;
                if (Rect{(*it)->getBounds()}.contains(pos))
                {
                    return true; // We found something, and it's not the given window - covered
                }
            }
        }
        return false; // Found nothing?
    }

    void WindowManager::makeTopMost(Window* window)
    {
        if (window != nullptr)
        {
            // We defer the order change due to many possible complications - i experienced them
            // Either infinite loops and other things when reordering while drawing...
            WINDOW_DATA.moveFrontQueue.push_back({window, nullptr});
        }
    }

    Window* WindowManager::getTopShownWindow()
    {
        if (WINDOW_DATA.windows.empty())
        {
            return nullptr;
        }

        for (auto it = WINDOW_DATA.windows.rbegin(); it != WINDOW_DATA.windows.rend(); ++it)
        {
            if (getIsShown(*it))
            {
                return *it;
            }
        }
        return nullptr;
    }

    Window* WindowManager::getHoveredWindow() { return WINDOW_DATA.hoveredWindow; }

    void WindowManager::hideAll()
    {
        for (auto* win : WINDOW_DATA.windows)
        {
            setShown(win, false);
        }
    }

    static void MoveInfrontImpl(Window* moved, Window* inFrontOf)
    {
        // Moving 'moved'(3) behind 'inFrontOf'(5) (last drawn element is on top)
        // [1][2][3][4][5]

        // [3]              -> assign to temporary
        // [1][2][4][5][0]  -> copy all element after 1 to the front
        // [1][2][4][5][3]  -> assign position of target element

        if (moved == inFrontOf)
        {
            // LOG_WARNING("Cant move the window before itself? O.o");
            return;
        }

        bool foundMoved = false;
        for (const auto* win : WINDOW_DATA.windows)
        {
            if (win == moved)
            {
                foundMoved = true;
            }
            else if (win == inFrontOf)
            {
                if (foundMoved)
                {
                    break;
                }
                LOG_WARNING("'moved' window is already in front of the 'inFrontOf' window!");
                return;
            }
        }

        auto& windows = WINDOW_DATA.windows;
        const auto targetIt = std::ranges::find(windows, inFrontOf);
        const auto targetIndex = std::distance(windows.begin(), targetIt);
        const auto movedIt = std::ranges::find(windows, moved);
        windows.erase(movedIt);
        windows.insert(windows.begin() + targetIndex, moved);
    }

    void WindowManager::update()
    {
        // Iterated in reverse - last drawn is the front most window - as soon as we find a hovered the ones behind cant be
        WINDOW_DATA.hoveredWindow = nullptr;
        for (auto it = WINDOW_DATA.windows.rbegin(); it != WINDOW_DATA.windows.rend(); ++it)
        {
            if (WINDOW_DATA.shownSet.contains(*it))
            {
                if ((*it)->getIsHovered())
                {
                    WINDOW_DATA.hoveredWindow = *it;
                    break;
                }
            }
        }

        auto& queue = WINDOW_DATA.moveFrontQueue;
        while (!queue.empty())
        {
            auto& [move, inFrontOf] = queue.front();
            MoveInfrontImpl(move, inFrontOf == nullptr ? WINDOW_DATA.windows.back() : inFrontOf);
            queue.pop_front();
        }
    }

} // namespace magique
