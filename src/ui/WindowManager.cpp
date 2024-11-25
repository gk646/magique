// SPDX-License-Identifier: zlib-acknowledgement
#include <magique/ui/WindowManager.h>
#include <magique/ui/controls/Window.h>
#include <magique/util/Logging.h>

#include "internal/utils/STLUtil.h"
#include "internal/globals/UIData.h"
#include "internal/headers/CollisionPrimitives.h"

namespace magique
{
    struct WindowManagerData final
    {
        std::vector<Window*> windows;
        std::vector<internal::WindowManagerMapping> nameMapping;
        HashSet<const Window*> shownSet;
        Window* hoveredWindow = nullptr;

        void removeWindow(const Window* window)
        {
            for (auto it = windows.begin(); it != windows.end(); ++it)
            {
                if (*it == window)
                {
                    windows.erase(it);
                    break;
                }
            }
            // If pred evaluates to true the element gets deleted
            auto pred = [](internal::WindowManagerMapping& mapping, const Window* window)
            { return mapping.window == window; };
            UnorderedDelete(nameMapping, window, pred);
        }

        bool getNameExists(const char* name) const
        {
            for (const auto& m : nameMapping)
            {
                if (strcmp(m.name, name) == 0)
                {
                    return true;
                }
            }
            return false;
        }

        bool getWindowExists(const Window* window) const
        {
            for (const auto* w : windows)
            {
                if (w == window)
                {
                    return true;
                }
            }
            return false;
        }
    };

    static WindowManagerData WINDOW_DATA{};

    WindowManager& GetWindowManager()
    {
        static WindowManager WINDOW_MANAGER = {};
        return WINDOW_MANAGER;
    }

    void WindowManager::draw()
    {
        for (auto* w : WINDOW_DATA.windows)
        {
            if (WINDOW_DATA.shownSet.contains(w))
                w->draw();
        }
    }

    void WindowManager::addWindow(Window* window, const char* name)
    {
        MAGIQUE_ASSERT(name != nullptr, "Passed name cannot be null");
        auto& winData = WINDOW_DATA;
        winData.windows.push_back(window);

        // Add name mapping
        const int len = static_cast<int>(strlen(name));
        if (len + 1 > MAGIQUE_MAX_NAMES_LENGTH)
        {
            LOG_WARNING("Given name is longer than configured!: %s", name);
            return;
        }
        for (const auto& mapping : winData.nameMapping)
        {
            if (strcmp(mapping.name, name) == 0)
            {
                LOG_ERROR("Given name already exists");
                return;
            }
        }
        internal::WindowManagerMapping mapping;
        std::memcpy(mapping.name, name, len);
        mapping.name[MAGIQUE_MAX_NAMES_LENGTH - 1] = '\0';
        mapping.window = window;
        winData.nameMapping.push_back(mapping);
        setShown(window, true);
    }

    Window* WindowManager::getWindow(const char* name)
    {
        MAGIQUE_ASSERT(name != nullptr, "Passed name cannot be null");
        for (const auto& m : WINDOW_DATA.nameMapping)
        {
            if (strcmp(m.name, name) == 0) [[unlikely]]
            {
                return m.window;
            }
        }
        LOG_WARNING("No window with such name: %s", name);
        return nullptr;
    }

    bool WindowManager::removeWindow(Window* window)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        if (!WINDOW_DATA.getWindowExists(window))
        {
            LOG_WARNING("Given window is not managed by the window manager");
            return false;
        }
        WINDOW_DATA.removeWindow(window);
        return true;
    }

    bool WindowManager::removeWindow(const char* name) { return removeWindow(getWindow(name)); }

    void WindowManager::setShown(Window* window, const bool shown)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        if (shown)
        {
            WINDOW_DATA.shownSet.insert(window);
        }
        else
        {
            WINDOW_DATA.shownSet.erase(window);
        }
    }

    void WindowManager::setShown(const char* name, const bool shown) { setShown(getWindow(name), shown); }

    bool WindowManager::getIsShown(Window* window)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        return WINDOW_DATA.shownSet.contains(window);
    }

    bool WindowManager::getIsShown(const char* name) { return getIsShown(getWindow(name)); }

    void WindowManager::toggleShown(Window* window)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");

        if (WINDOW_DATA.shownSet.contains(window))
        {
            WINDOW_DATA.shownSet.erase(window);
        }
        else
        {
            WINDOW_DATA.shownSet.insert(window);
        }
    }

    void WindowManager::toggleShown(const char* name) { toggleShown(getWindow(name)); }

    const std::vector<Window*>& WindowManager::getWindows() { return WINDOW_DATA.windows; }

    void WindowManager::moveInFrontOf(Window* moved, Window* inFrontOf)
    {
        // Moving 'moved'(3) behind 'inFrontOf'(5) (last drawn element is on top)
        // [1][2][3][4][5]

        // [3]              -> assign to temporary
        // [1][2][4][5][0]  -> copy all element after 1 to the front
        // [1][2][4][5][3]  -> assign position of target element

        if (moved == inFrontOf)
        {
            LOG_WARNING("Cant move the window before itself? O.o");
            return;
        }

        bool foundMoved = false;
        for (const auto* w : WINDOW_DATA.windows)
        {
            if (w == moved)
            {
                foundMoved = true;
            }
            else if (w == inFrontOf)
            {
                if (foundMoved)
                    break;
                LOG_WARNING("'moved' window is already in front of the 'inFrontOf' window!");
                return;
            }
        }

        auto& windows = WINDOW_DATA.windows;
        const auto targetIt = std::ranges::find(windows, inFrontOf);
        const auto targetIndex = std::distance(windows.begin(), targetIt);
        auto movedIt = std::ranges::find(windows, moved);
        windows.erase(movedIt);
        windows.insert(windows.begin() + targetIndex, moved);
    }

    void WindowManager::moveInFrontOf(const char* moved, const char* inFrontOf)
    {
        moveInFrontOf(getWindow(moved), getWindow(inFrontOf));
    }

    bool WindowManager::getIsCovered(Window* window, const Point pos)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        // Iterated in reverse - last drawn is the front most window - return as soon as we find obstruction
        for (auto it = WINDOW_DATA.windows.rbegin(); it != WINDOW_DATA.windows.rend(); ++it)
        {
            if (WINDOW_DATA.shownSet.contains(*it)) // only search visible ones
            {
                if (*it == window) // We found the given window - nothing is in front
                    return false;
                const auto bounds = (*it)->getBounds();
                if (PointToRect(pos.x, pos.y, bounds.x, bounds.y, bounds.width, bounds.height))
                {
                    return true; // We found something, and it's not the given window - covered
                }
            }
        }
        return false; // Found nothing?
    }

    bool WindowManager::getIsCovered(const char* window, const Point pos)
    {
        return getIsCovered(getWindow(window), pos);
    }

    void WindowManager::makeTopMost(Window* window)
    {
        MAGIQUE_ASSERT(window != nullptr, "Passed window cannot be null");
        if (WINDOW_DATA.windows.back() == window)
            return;
        moveInFrontOf(window, WINDOW_DATA.windows.back()); // Reuse the method - move
    }

    void WindowManager::makeTopMost(const char* name) { makeTopMost(getWindow(name)); }

    Window* WindowManager::getHoveredWindow() { return WINDOW_DATA.hoveredWindow; }

    void WindowManager::update()
    {
        // Iterated in reverse - last drawn is the front most window - as soon as we find a hovered the ones behind cant be
        const auto mouse = global::UI_DATA.getMousePos();
        for (auto it = WINDOW_DATA.windows.rbegin(); it != WINDOW_DATA.windows.rend(); ++it)
        {
            if (WINDOW_DATA.shownSet.contains(*it))
            {
                const auto bounds = (*it)->getBounds();
                if (PointToRect(mouse.x, mouse.y, bounds.x, bounds.y, bounds.width, bounds.height))
                {
                    WINDOW_DATA.hoveredWindow = *it;
                    break;
                }
            }
        }
    }

} // namespace magique