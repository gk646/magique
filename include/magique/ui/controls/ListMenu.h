#ifndef MAGEQUEST_LISTMENU_H
#define MAGEQUEST_LISTMENU_H

#include <string>
#include <vector>
#include <magique/ui/UIObject.h>

//===============================================
// List Menu
//===============================================
// .....................................................................
// List menu is a control that holds a list of string values stacked vertically for the user to choose from
// .....................................................................

namespace magique
{

    struct ListMenu : UIObject
    {

        void onDraw(const Rectangle& bounds) override;

        void clear();
        void add(const char* item);
        void remove(const char* item);

        const std::vector<std::string>& getItems() const;

    protected:
        void drawDefault(const Font& font, float size, Color back, Color outline, Color text);

    private:
        std::vector<std::string> values;
    };
} // namespace magique


#endif //MAGEQUEST_LISTMENU_H
