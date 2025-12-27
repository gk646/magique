#ifndef MAGEQUEST_LISTMENU_H
#define MAGEQUEST_LISTMENU_H

#include <string>
#include <vector>
#include <magique/ui/UIObject.h>
#include <magique/core/Core.h>
#include <magique/internal/PlatformIncludes.h>

//===============================================
// List Menu
//===============================================
// .....................................................................
// List menu is a control that holds a list of string values stacked vertically for the user to choose from
// If entry is clicked its selected. Selection is persistent (until changed) and can be queried and cleared
// Its very customizable with a DrawEntryFunc - using this you can completely customize the entry:
//  - Drawing icons, animations or multiline text, ...
// .....................................................................

namespace magique
{

    using SelectFunc = std::function<void(const std::string&)>;

    // How to draw an item - returns the height of the entry - called for all entries
    // Called with top left position of entry, text , index and status
    using DrawEntryFunc = std::function<float(Point pos, const char* txt, int idx, bool hovered, bool selected)>;

    struct ListMenu : UIObject
    {
        // Creates a new ListMenu from coordinates in the logical UI resolution
        ListMenu(float x, float y, float w, float h);
        ListMenu(float w, float h, Anchor anchor = Anchor::NONE, Point inset = 0.0F);

        // Draws all entries by calling drawEntryDefault or if set a custom draw function
        void onDraw(const Rectangle& bounds) override;

        void onUpdate(const Rectangle& bounds, bool wasDrawn) override
        {
            if (wasDrawn)
                updateState();
        }

        // Removes all values
        void clear();
        bool empty() const;

        // Inserts a new value at the specified position
        void add(const char* item, int index = -1);

        // Removes the value with the given name
        bool remove(const char* item);
        bool remove(int index);

        // Returns the index of the hovered element
        // -1 if nothing is hovered
        int getHoveredIndex() const;
        const char* getHovered() const;

        int getSelectedIndex() const;
        const char* getSelected() const;
        // Use -1 to clear
        void setSelected(int index);

        // Allows to set a custom callback called everytime a (new) value is selected
        void setOnSelect(const SelectFunc& func);

        // Sets a custom function to draw entries
        // Default: Uses drawDefaultEntry
        void setDrawEntryFunc(const DrawEntryFunc& func);

    protected:
        // Draws a default representation of an entry
        float drawDefaultEntry(const Point& pos, const char* txt, bool hovered, bool selected) const;

        // Updates hovered and selected items and sets the height based on element count
        void updateState();

    private:
        SelectFunc selectFunc;
        DrawEntryFunc drawFunc;
        struct Entry final
        {
            std::string text;
            float height;
        };
        std::vector<Entry> entries;
        int hovered = -1;
        int selected = -1;
    };
} // namespace magique


#endif //MAGEQUEST_LISTMENU_H
