#ifndef MAGIQUE_DATATABLE_H
#define MAGIQUE_DATATABLE_H

#include <vector>
#include <cstring>
#include <initializer_list>
#include <magique/internal/InternalTypes.h>
#include <magique/internal/Macros.h>
#include <magique/util/Logging.h>

//===============================================
// DataTable
//===============================================
// .....................................................................
// This is made for accessing data in a database like fashion
// Compared to databases its statically typed which makes it easier to work with
// .....................................................................

namespace magique
{
    template <typename... Types>
    struct DataTable final
    {
        using ColumnsTuple = std::tuple<Types...>;
        //================= TABLE =================//

        // Constructs an EMPTY data table - specify the column names
        // Example:     DataTable table{"age", "height"};
        DataTable(const std::initializer_list<const char*>& args);

        // Returns the amount of rows
        [[nodiscard]] int getRows() const;

        // Returns the amount of columns
        [[nodiscard]] constexpr int getColumns() const;

        // Returns the index of the given column name
        // Failure: Returns -1 if the column is not found
        int getColumnIndex(const char* column) const;

        // Returns a vector containing all the columns names
        // Note: vector is created for each method call
        [[nodiscard]] std::vector<const char*> getColumnNames() const;

        //================= SET =================//
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

        //================= GET =================//
        // 0 - based indexing

        // Row getters
        ColumnsTuple& operator[](int row);
        const ColumnsTuple& operator[](int row) const;

        // Get the cell with runtime column -> type has to be provided
        template <typename T>
        auto& getCell(int row, int column);
        template <typename T>
        const auto& getCell(int row, int column) const;

        // Get the cell with runtime name -> type has to be provided
        template <typename T>
        T& getCell(int row, const char* name);
        template <typename T>
        const T& getCell(int row, const char* name) const;

        // Returns the underlying data vector
        const std::vector<ColumnsTuple>& getData() const;

        //================= ITERATORS =================//
        // iterators for rows - use with structured bindings for a very nice syntax

        Iterator<ColumnsTuple> begin();
        Iterator<ColumnsTuple> end();

        Iterator<const ColumnsTuple> begin() const;
        Iterator<const ColumnsTuple> end() const;

    private:
        using OffsetArray = std::array<int, sizeof...(Types)>;
        template <std::size_t... Indices>
        constexpr std::array<int, sizeof...(Types)> calculateOffsets(std::index_sequence<Indices...>);
        template <typename T, typename Tuple, std::size_t... Is>
        T& getTupleColumnImpl(int index, Tuple& t, std::index_sequence<Is...>);
        template <typename T>
        T& getTupleColumn(int column, std::tuple<Types...>& t);

        OffsetArray offsets;                                         // Accumulative offset for the columns
        char names[sizeof...(Types)][MAGIQUE_MAX_NAMES_LENGTH]{}; // Column names
        std::vector<ColumnsTuple> data;                              // Data storage row-wise
        int columns = sizeof...(Types);                              // Amount of columns
    };
} // namespace magique


//================= IMPLEMENTATION =================//

namespace magique
{
    template <typename... Types>
    DataTable<Types...>::DataTable(const std::initializer_list<const char*>& args) :
        offsets(calculateOffsets(std::make_index_sequence<sizeof...(Types) - 1>{}))
    {
        int i = 0;
        for (const auto arg : args)
        {
            if (arg == nullptr)
            {
                LOG_ERROR("Passing nullptr as column name");
                continue;
            }
            int len = static_cast<int>(strlen(arg));
            memcpy(names[i], arg, std::min(MAGIQUE_MAX_NAMES_LENGTH, len));
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
        LOG_FATAL("DataTable does not contain column with name:%s", column);
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
    template <typename T>
    auto& DataTable<Types...>::getCell(int row, int column)
    {
        MAGIQUE_ASSERT(column >= 0 && column < columns, "Given column out of bounds");
        return getTupleColumn<T>(column, data[row]);
    }
    template <typename... Types>
    template <typename T>
    const auto& DataTable<Types...>::getCell(int row, int column) const
    {
        MAGIQUE_ASSERT(column >= 0 && column < columns, "Given column out of bounds");
        return getTupleColumn<T>(column, data[row]);
    }
    template <typename... Types>
    template <typename T>
    T& DataTable<Types...>::getCell(int row, const char* name)
    {
        const int col = getColumnIndex(name);
        return getTupleColumn<T>(col, data[row]);
    }
    template <typename... Types>
    template <typename T>
    const T& DataTable<Types...>::getCell(int row, const char* name) const
    {

        int col = getColumnIndex(name);
        return getTupleColumn<T>(col, data[row]);
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
    template <typename... Types>
    template <typename T, typename Tuple, std::size_t... Is>
    T& DataTable<Types...>::getTupleColumnImpl(int index, Tuple& t, std::index_sequence<Is...>)
    {
        T* ptrs[] = {std::is_same_v<T, std::tuple_element_t<Is, Tuple>> ? reinterpret_cast<T*>(&std::get<Is>(t))
                                                                        : nullptr...};
        return *ptrs[index];
    }
    template <typename... Types>
    template <typename T>
    T& DataTable<Types...>::getTupleColumn(int column, std::tuple<Types...>& t)
    {
        static_MAGIQUE_ASSERT((std::is_same_v<T, Types> || ...), "Given type does not exist in the table!");
        MAGIQUE_ASSERT(column >= 0 && column < columns, "Given column out of bounds");
        return getTupleColumnImpl<T>(column, t, std::index_sequence_for<Types...>{});
    }
    template <typename... Types>
    template <std::size_t... Indices>
    constexpr std::array<int, sizeof...(Types)> DataTable<Types...>::calculateOffsets(std::index_sequence<Indices...>)
    {
        OffsetArray ret = {0};
        ((ret[Indices + 1] = ret[Indices] + internal::SizeOf<std::tuple_element_t<Indices, std::tuple<Types...>>>()), ...);
        return ret;
    }

} // namespace magique

#endif //MAGIQUE_DATATABLE_H