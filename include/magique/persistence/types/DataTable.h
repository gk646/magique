#ifndef MAGIQUE_DATATABLE_H
#define MAGIQUE_DATATABLE_H

#include <initializer_list>
#include <vector>
#include <magique/util/Defines.h>
#include <magique/util/InternalTypes.h>
#include <magique/util/Logging.h>

//-----------------------------------------------
// DataTable
//-----------------------------------------------
// .....................................................................
// This is made for saving gamedata in a database like fashing
// This class provides all the necessary
// .....................................................................

namespace magique
{
    template <typename... Types>
    struct DataTable final
    {
        using ColumnsTuple = std::tuple<Types...>;
        //----------------- TABLE -----------------//

        // Constructs an EMPTY data table - specify the column names
        // Example:     DataTable table{"age", "height"};
        DataTable(const std::initializer_list<const char*>& args);

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
        template <int column>
        void set(const auto& value, int row);

        // Assigns the given row new values
        void setRow(const ColumnsTuple& tuple, int row);

        // Assigns the given row new values
        void setRow(ColumnsTuple&& tuple, int row);

        // Adds a new row with the given values
        void addRow(const ColumnsTuple& tuple);

        // Adds a new row with the given values
        void addRow(ColumnsTuple&& tuple);

        //----------------- GET -----------------//
        // 0 - based indexing

        // Row getters
        ColumnsTuple& operator[](int row);
        const ColumnsTuple& operator[](int row) const;

        // Cell getters
        // Specify the column in the template -> type safety
        template <int column>
        auto& getCell(int row);
        template <int column>
        const auto& getCell(int row) const;

        // Get the cell by name
        template <typename T>
        T& getCell(int row, const char* name);
        template <typename T>
        const T& getCell(int row, const char* name) const;

        // Returns the underlying data vector
        const std::vector<ColumnsTuple>& getData() const;

        //----------------- ITERATORS -----------------//
        // iterators for rows

        Iterator<ColumnsTuple> begin();
        Iterator<ColumnsTuple> end();

        Iterator<const ColumnsTuple> begin() const;
        Iterator<const ColumnsTuple> end() const;

    private:
        template <typename T>
        constexpr size_t SizeOf()
        {
            return sizeof(T);
        }
        template <std::size_t... Indices, typename... Types>
        constexpr std::array<size_t, sizeof...(Types)> calculateOffsets(std::index_sequence<Indices...>,
                                                                        std::tuple<Types...>)
        {
            return {
                (Indices == 0 ? 0 : (SizeOf<std::tuple_element_t<Indices - 1, std::tuple<Types...>>>()) + ... + 0)...};
        }
        int offsets[sizeof...(Types)]{};                             // Accumulative offset for the columns
        char names[sizeof...(Types)][MAGIQUE_MAX_TABLE_NAME_SIZE]{}; // Column names
        std::vector<ColumnsTuple> data;                              // Data storage row-wise
        int rowSize = (sizeof(Types) + ...);                         // Byte size of 1 row
        int columns = sizeof...(Types);                              // Amount of columns
    };


    //----------------- IMPLEMENTATION -----------------//
    template <typename... Types>
    DataTable<Types...>::DataTable(const std::initializer_list<const char*>& args)
    {
        int i = 0;
        for (const auto arg : args)
        {
            if (arg == nullptr)
            {
                LOG_ERROR("Passing nullptr as column name");
                continue;
            }
            int len = strlen(arg);
            std::memcpy(names[i], arg, std::min(MAGIQUE_MAX_TABLE_NAME_SIZE, len));
            i++;
        }
    }
    template <typename... Types>
    int DataTable<Types...>::getRows() const
    {
        return data.size();
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
    template <int column>
    void DataTable<Types...>::set(const auto& value, int row)
    {
        std::get<column>(data[row]) = value;
    }
    template <typename... Types>
    void DataTable<Types...>::setRow(const ColumnsTuple& tuple, int row)
    {
        data[row] = tuple;
    }
    template <typename... Types>
    void DataTable<Types...>::setRow(ColumnsTuple&& tuple, int row)
    {
        data[row] = std::move(tuple);
    }
    template <typename... Types>
    void DataTable<Types...>::addRow(const ColumnsTuple& tuple)
    {
        data.push_back(tuple);
    }
    template <typename... Types>
    void DataTable<Types...>::addRow(ColumnsTuple&& tuple)
    {
        data.emplace_back(tuple);
    }
    template <typename... Types>
    typename DataTable<Types...>::ColumnsTuple& DataTable<Types...>::operator[](const int row)
    {
        return data[row];
    }
    template <typename... Types>
    const typename DataTable<Types...>::ColumnsTuple& DataTable<Types...>::operator[](const int row) const
    {
        return data[row];
    }
    template <typename... Types>
    template <int column>
    auto& DataTable<Types...>::getCell(int row)
    {
        return std::get<column>(data[row]);
    }
    template <typename... Types>
    template <int column>
    const auto& DataTable<Types...>::getCell(int row) const
    {
        return std::get<column>(data[row]);
    }
    template <typename... Types>
    template <typename T>
    T& DataTable<Types...>::getCell(int row, const char* name)
    {
        return
    }
    template <typename... Types>
    const std::vector<typename DataTable<Types...>::ColumnsTuple>& DataTable<Types...>::getData() const
    {
        return data;
    }
    template <typename... Types>
    Iterator<typename DataTable<Types...>::ColumnsTuple> DataTable<Types...>::begin()
    {
        return Iterator<ColumnsTuple>(data.data());
    }
    template <typename... Types>
    Iterator<typename DataTable<Types...>::ColumnsTuple> DataTable<Types...>::end()
    {
        return Iterator<ColumnsTuple>(data.data() + data.size());
    }
    template <typename... Types>
    Iterator<const typename DataTable<Types...>::ColumnsTuple> DataTable<Types...>::begin() const
    {
        return Iterator<ColumnsTuple>(data.data());
    }
    template <typename... Types>
    Iterator<const typename DataTable<Types...>::ColumnsTuple> DataTable<Types...>::end() const
    {
        return Iterator<ColumnsTuple>(data.data() + data.size());
    }


} // namespace magique

#endif //MAGIQUE_DATATABLE_H