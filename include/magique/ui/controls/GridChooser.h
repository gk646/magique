#ifndef MAGIQUE_GRIDCHOOSER_H
#define MAGIQUE_GRIDCHOOSER_H

#include <cmath>
#include <magique/ui/controls/ListChooser.h>
#include <magique/core/Draw.h>
#include <magique/ui/UI.h>

//===============================================
// GridChooser
//===============================================
// .....................................................................
// GridChooser is similar to the ListChooser but items are arranged in a grid (left right, up down orientation)
// Also items can have a texture and text associated with it
// Note: For identification you can use any type K (key) - your class needs to then implement operator== with that type
// .....................................................................

namespace magique
{

    // How to draw an item - returns the dimensions of the entry - called for all entries
    // Called with top left position of entry, text, index and status
    template <typename T>
    using DrawGridItemFunc = std::function<Point(Point pos, const T& item, int idx, bool hovered, bool selected)>;

    template <typename T>
    struct GridChooser : UIObject
    {
        GridChooser(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode mode = ScalingMode::FULL);

        // Sets the gap between items - vertical & horizontal
        void setGap(float gap);
        float getGap() const;

        // Allows to set the default size for items that are added
        // Note: This is useful to avoid visual glitches during the first draw when elements change line due to dims
        void setDefaultItemSize(Point dims);

        // Add a new item
        void addItem(const T& item);

        // Returns true if an item where key == item was removed
        template <typename K>
        bool removeItem(const K& key);

        // Returns the item when key == item
        // Failure: Returns nullptr if not found
        template <typename K>
        T* getItem(const K& key);
        T* getItem(int index);

        // Removes all items and resets hovered and selected index
        void clear();

        // Selects the item when key == item
        template <typename K>
        void setSelected(const K& key);
        void setSelected(int index);

        int getSelectedIndex() const;

        // Calls getItem(int) with the selected index
        T* getSelected();

        // Set a callback called everytime a NEW value is selected (clicked)
        void setOnSelect(const SelectFunc<T>& func);

        // Set a callback called everytime a NEW value is hovered (can be quick often!) or falls back to selected if valid
        void setOnHover(const SelectFunc<T>& func);

        // Sets a custom function to draw items
        // Default: Uses drawDefaultEntry
        void setDrawEntryFunc(const DrawGridItemFunc<T>& func);

    protected:
        void onDraw(const Rect& bounds) override;

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateState();
        }

        // Checks for selections
        void updateState();

    private:
        SelectFunc<T> selectFunc;
        SelectFunc<T> hoverFunc;
        DrawGridItemFunc<T> drawFunc;
        struct Entry final
        {
            T item;
            Point dims;
        };
        std::vector<Entry> entries;
        Point defaultDims{};
        int hovered = -1;
        int selected = -1;
        float gap = 2;
    };


} // namespace magique


// IMPLEMENTATION


namespace magique
{
    template <typename T>
    GridChooser<T>::GridChooser(Rect bounds, Anchor anchor, Point inset, ScalingMode mode) :
        UIObject(bounds, anchor, inset, mode)
    {
    }

    template <typename T>
    void GridChooser<T>::setGap(float newGap)
    {
        gap = newGap;
    }

    template <typename T>
    float GridChooser<T>::getGap() const
    {
        return gap;
    }

    template <typename T>
    void GridChooser<T>::setDefaultItemSize(Point dims)
    {
        defaultDims = dims;
    }

    template <typename T>
    void GridChooser<T>::addItem(const T& item)
    {
        entries.emplace_back(item, defaultDims);
    }

    template <typename T>
    template <typename K>
    bool GridChooser<T>::removeItem(const K& key)
    {
        return std::erase_if(entries, [&](const auto& entry) { return entry.item == key; }) > 0;
    }
    template <typename T>
    template <typename K>
    T* GridChooser<T>::getItem(const K& key)
    {
        auto it = std::ranges::find_if(entries, [&](const auto& entry) { return entry.item == key; });
        if (it != entries.end())
        {
            return &it->item;
        }
        return nullptr;
    }

    template <typename T>
    T* GridChooser<T>::getItem(int index)
    {
        if (index < (int)entries.size())
        {
            return &entries[index].item;
        }
        return nullptr;
    }


    template <typename T>
    void GridChooser<T>::clear()
    {
        entries.clear();
        hovered = -1;
        setSelected(-1);
    }

    template <typename T>
    template <typename K>
    void GridChooser<T>::setSelected(const K& key)
    {
        for (int i = 0; i < (int)entries.size(); i++)
        {
            if (entries[i].item == key)
            {
                selected = i;
                return;
            }
        }
    }

    template <typename T>
    void GridChooser<T>::setSelected(int index)
    {
        selected = index;
    }

    template <typename T>
    int GridChooser<T>::getSelectedIndex() const
    {
        return selected;
    }

    template <typename T>
    T* GridChooser<T>::getSelected()
    {
        return getItem(getSelectedIndex());
    }

    template <typename T>
    void GridChooser<T>::setOnSelect(const SelectFunc<T>& func)
    {
        selectFunc = func;
    }

    template <typename T>
    void GridChooser<T>::setOnHover(const SelectFunc<T>& func)
    {
        hoverFunc = func;
    }

    template <typename T>
    void GridChooser<T>::setDrawEntryFunc(const DrawGridItemFunc<T>& func)
    {
        drawFunc = func;
    }

    template <typename T>
    void GridChooser<T>::onDraw(const Rect& bounds)
    {
        if (!drawFunc)
        {
            LOG_WARNING("No drawfunc set on GridChooser");
            return;
        }

        Point pos = bounds.pos() + gap;
        float end = bounds.topRight().x - gap;
        float lineHeight = 0;
        for (int i = 0; i < (int)entries.size(); i++)
        {
            auto& entry = entries[i];
            if (pos.x + entry.dims.x > end)
            {
                pos.x = bounds.x + gap;
                pos.y += lineHeight;
                lineHeight = 0;
            }
            entry.dims = drawFunc(pos, entry.item, i, hovered == i, selected == i);
            lineHeight = std::max(lineHeight, entry.dims.y + gap);
            pos.x += entry.dims.x + gap;
        }
    }

    template <typename T>
    void GridChooser<T>::updateState()
    {
        const auto& bounds = getBounds();
        Point pos = bounds.pos() + gap;
        float end = bounds.topRight().x - gap;
        float lineHeight = 0;
        hovered = -1;
        for (int i = 0; i < (int)entries.size(); i++)
        {
            const auto& entry = entries[i];
            if (pos.x + entry.dims.x > end)
            {
                pos.x = bounds.x + gap;
                pos.y += lineHeight;
                lineHeight = 0;
            }

            const Rect itemRect = {pos, entry.dims};
            if (itemRect.contains(GetMousePosition()) && !LayeredInput::GetIsMouseConsumed())
            {
                hovered = i;
                if (LayeredInput::IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    if (i != selected)
                    {
                        selected = i;
                        if (selectFunc)
                            selectFunc(entries[i].item);
                    }
                    LayeredInput::ConsumeMouse();
                }
            }
            lineHeight = std::max(lineHeight, entry.dims.y + gap);
            pos.x += entry.dims.x + gap;
        }
        if (hoverFunc)
        {
            if (hovered != -1)
            {
                hoverFunc(*getItem(hovered));
            }
            else if (selected != -1)
            {
                hoverFunc(*getSelected());
            }
        }
        setSize({bounds.width, std::max(pos.y - bounds.y, getStartBounds().height)});
    }
} // namespace magique

#endif // MAGIQUE_GRIDCHOOSER_H
