#ifndef MAGEQUEST_LIST_CHOOSER_H
#define MAGEQUEST_LIST_CHOOSER_H

#include <string>
#include <vector>
#include <magique/ui/UIObject.h>
#include <magique/core/Engine.h>
#include <functional>

//===============================================
// List Chooser
//===============================================
// .....................................................................
// ListChooser is a control that holds a list of string values stacked vertically for the user to choose from
// If an entry is clicked, it's selected. Selection is persistent (until changed) and can be queried and cleared
// It's very customizable through DrawEntryFunc - using this you can completely customize the entry:
//  - Drawing icons, animations or multiline text, ...
// .....................................................................

namespace magique
{

    template <typename T>
    using SelectFunc = std::function<void(const T& item)>;

    // How to draw an item - returns the height of the entry - called for all entries
    // Called with top left position of entry, text , index and status
    using DrawItemFunc = std::function<float(Point pos, std::string_view txt, int idx, bool hovered, bool selected)>;

    struct ListChooser : UIObject
    {
        // Creates a new ListMenu from coordinates in the logical UI resolution
        ListChooser(Rect bounds, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode mode = ScalingMode::FULL);

        // Draws all entries by calling drawEntryDefault or (if set) uses a custom draw function
        void onDraw(const Rect& bounds) override;

        void onUpdate(const Rect& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateState();
        }

        // Removes all values
        void clear();
        bool empty() const;

        // Inserts a new value at the specified position
        void add(std::string_view item, int index = -1);

        // Removes the value with the given name
        bool remove(const char* item);
        bool remove(int index);

        // Returns the index of the hovered element
        // -1 if nothing is hovered
        int getHoveredIndex() const;
        const char* getHovered() const;

        int getSelectedIndex() const;
        const char* getSelected() const;
        // Use -1 to clear - does NOT call the callback
        void setSelected(int index = -1);
        void setSelected(std::string_view item);

        // Allows to set a custom callback called everytime a (new) value is selected
        void setOnSelect(const SelectFunc<std::string>& func);

        // Sets a custom function to draw items
        // Default: Uses drawDefaultEntry
        void setDrawEntryFunc(const DrawItemFunc& func);

    protected:
        // Draws a default representation of an entry
        float drawDefaultEntry(const Point& pos, std::string_view txt, bool isHovered, bool isSelected) const;

        // Updates hovered and selected items and sets the height based on element count
        void updateState();

    private:
        SelectFunc<std::string> selectFunc;
        DrawItemFunc drawFunc;
        struct Entry final
        {
            std::string text;
            float height;
        };
        std::vector<Entry> entries;
        int hovered = -1;
        int selected = -1;
        friend struct Dropdown;
    };
} // namespace magique


#endif // MAGEQUEST_LIST_CHOOSER_H
