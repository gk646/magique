#ifndef MAGIQUE_TABLE_H
#define MAGIQUE_TABLE_H

#include <variant>
#include <vector>
#include <magique/ui/UIObject.h>

//===============================================
// Table
//===============================================
// .....................................................................
//
// .....................................................................

namespace magique
{

    using TableRow = std::variant<std::string, float, int>;

    struct Table : UIObject
    {

        Table(Rect size, Anchor anchor = Anchor::NONE, Point inset = {}, ScalingMode scaling = ScalingMode::FULL);

        void addColumn(std::string_view name, float width) {}

    protected:
        void onDraw(const Rect& bounds) override
        {

        }

    private:
        struct ColumnInfo final
        {
            std::string name;
            float width;
        };
        std::vector<ColumnInfo> columns;
        std::vector<TableRow> rows;
    };

} // namespace magique

#endif // MAGIQUE_TABLE_H
