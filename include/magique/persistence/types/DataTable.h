#ifndef MAGIQUE_DATATABLE_H
#define MAGIQUE_DATATABLE_H

#include <initializer_list>
#include <vector>
#include <magique/util/Defines.h>

//-----------------------------------------------
// DataTable
//-----------------------------------------------
// .....................................................................
// This is made for saving gamedata in a database like fashing
// This class provides all the necessary
// .....................................................................

namespace magique
{
    struct Column final
    {
        const char* name = nullptr;
        int size = 0;

        // Specify the type
        template <typename T>
        explicit Column(const char* name) : name(name), size(sizeof(T))
        {
        }
    };

    template <typename... Types>
    struct DataTable final
    {
        using ColumnsTuple = std::tuple<Types...>;
        //----------------- TABLE -----------------//

        // Specify the column names
        // Example:     DataTable table{Column<int>("age"), Column<float>("height")};
        explicit DataTable(const char* args...);

        // Returns the amount of rows
        [[nodiscard]] int getRows() const;

        // Returns the amount of columns
        [[nodiscard]] constexpr int getColumns() const;

        // Returns the index of the given column name
        // Faliure: Returns -1 if the column is not found
        int getColumnIndex(const char* column) const;

        // Returns a vector containing all the columns names
        // Note: vector is created for each method call
        [[nodiscard]] std::vector<const char*> getColumnNames() const;

        //----------------- SET -----------------//
        // 0 - based indexing

        // Sets a value at the specified position
        template <typename T>
        void set(const T& value, int row, int column);

        void setRow(const ColumnsTuple& tuple, int row);

        //----------------- GET -----------------//
        // 0 - based indexing

        ColumnsTuple& getRow(int row) { return getData(row, 0); }

        // Returns the value at the specified position
        template <typename T>
        const T& get(int row, int column) const
        {
            return *static_cast<const T*>(getData(row, column));
        }

        // Returns the value at the specified position
        template <typename T>
        T& get(int row, int column)
        {
            return *static_cast<const T*>(getData(row, column));
        }

    private:
        void addColumn(const char* name);

        bool addColumnImpl(const char* name, int bytes);

        void* getData(int row, int column, int size);
        [[nodiscard]] const void* getData(int row, int column) const;

        void setData(int row, int column);

        void* data = nullptr;                                        // Data storage row-wise
        int rows = 0;                                                // Number of rows
        int offsets[sizeof...(Types)]{};                             // Accumulative offset for the columns
        char names[sizeof...(Types)][MAGIQUE_MAX_TABLE_NAME_SIZE]{}; // Column names
        constexpr int rowSize = (sizeof(Types) + ...);               // Byte size of 1 row
        constexpr int columns = sizeof...(Types);                    // Amount of columns
    };


    //----------------- IMPLEMENTATION -----------------//
    template <typename... Types>
    DataTable<Types...>::DataTable(const char* args, ...)
    {
        const std::initializer_list<const char*> list{args...};
        for (const auto arg : list)
        {
            addColumn(arg);
        }
    }
    template <typename... Types>
    int DataTable<Types...>::getRows() const
    {
        return rows;
    }
    template <typename... Types>
    constexpr int DataTable<Types...>::getColumns() const
    {
        return columns;
    }
    template <typename... Types>
    int DataTable<Types...>::getColumnIndex(const char* column) const
    {
        for (int i = 0; i < columns; ++i)
        {
            if (strcmp(names[i], column) == 0)
                return i;
        }
        return -1;
    }
    template <typename... Types>
    std::vector<const char*> DataTable<Types...>::getColumnNames() const
    {
        std::vector<const char*> ret;
        ret.reserve(columns + 1);
        for (auto name : names)
        {
            ret.push_back(name);
        }
        return ret;
    }
    template <typename... Types>
    template <typename T>
    void DataTable<Types...>::set(const T& value, int row, int column)
    {
        setData(row, column, sizeof(T));
    }
    template <typename... Types>
    void DataTable<Types...>::setRow(const ColumnsTuple& tuple, int row)
    {

    }

} // namespace magique

#endif //MAGIQUE_DATATABLE_H