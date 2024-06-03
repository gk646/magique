#ifndef MULTIRESOLUTIONGRID_H
#define MULTIRESOLUTIONGRID_H

#include <cassert>

#include <vector>
#include <ankerl/unordered_dense.h>

// IMPORTANT: Use a custom hashmap here and possibly a custom vector
// You can then remove the includes
template <typename Key, typename Value>
using HashMapType = ankerl::unordered_dense::map<Key, Value>; // Insert custom type here

template <typename Value>
using VectorType = std::vector<Value>; // Inset custom type here


// Make lookup tables
// Make 3 top level data arrays:
//  - CellEntries (From where to where the range goes), DataBlock (saves the data contigousy),
//  - Map CellID to a CellEntry -> HashMap<CellID, index> , reserve some open slots (64)
//  - In the CellEntry save the memory location and count and a pointer to the next block (if collision happen)


using CellID = uint64_t;
// This creates a unique value - both values are unqiue themselves so their concatenated version is aswell
// We still use a hash function on it to get more byte range (all bits flipped)
inline CellID GetCellID(const int cellX, const int cellY) { return static_cast<uint64_t>(cellX) << 32 | cellY; }

template <typename T, int size>
struct DataBlock final
{
    static constexpr int NO_NEXT_BLOCK = UINT16_MAX;
    T data[size];                  // Fixed size data block
    uint16_t count = 0;            // Current number of elements
    uint16_t next = NO_NEXT_BLOCK; // Index of the next block or -1 if if its the end

    [[nodiscard]] bool isFull() const { return count == size; }
    // Can happen that its full but no next one is inserted yet
    [[nodiscard]] bool hasNext() const { return next != NO_NEXT_BLOCK; }

    void add(T val)
    {
        assert(count < size);
        data[count++] = val;
    }
    template <typename Container>
    void append(Container& elems) const
    {
        for (int i = 0; i < count; ++i)
        {
            elems.insert(data[i]);
        }
    }
};

template <typename V, int blockSize = 16>
struct SingleResolutionHashGrid final
{
    HashMapType<CellID, int> cellMap{};
    VectorType<DataBlock<V, blockSize>> dataBlocks{};
    int cellSize;

    explicit SingleResolutionHashGrid(const int cellSize) : cellSize(cellSize) {}

    void insert(V val, const float x, const float y, const int w, const int h)
    {
        int x1 = static_cast<int>(x) / cellSize;
        int y1 = static_cast<int>(y) / cellSize;
        int x2 = (static_cast<int>(x) + w) / cellSize;
        int y2 = (static_cast<int>(y) + h) / cellSize;

        CellID cellID = GetCellID(x1, y1);
        insertElement(cellID, val);

        if (x1 != x2) [[unlikely]]
        {
            cellID = GetCellID(x2, y1);
            insertElement(cellID, val);

            if (y1 != y2) [[unlikely]]
            {
                cellID = GetCellID(x1, y2);
                insertElement(cellID, val);

                cellID = GetCellID(x2, y2);
                insertElement(cellID, val);
                return;
            }
        }

        if (y1 != y2) [[unlikely]]
        {
            cellID = GetCellID(x1, y2);
            insertElement(cellID, val);
        }
    }

    template <typename Container>
    void query(Container& elems, const float x, const float y, const int w, const int h)
    {
        const int x1 = static_cast<int>(x) / cellSize;
        const int y1 = static_cast<int>(y) / cellSize;
        const int x2 = (static_cast<int>(x) + w) / cellSize;
        const int y2 = (static_cast<int>(y) + h) / cellSize;

        CellID cellID = GetCellID(x1, y1);
        queryElements(cellID, elems);

        if (x1 != x2) [[unlikely]]
        {
            cellID = GetCellID(x2, y1);
            queryElements(cellID, elems);

            if (y1 != y2) [[unlikely]]
            {
                cellID = GetCellID(x1, y2);
                queryElements(cellID, elems);

                cellID = GetCellID(x2, y2);
                queryElements(cellID, elems);
                return;
            }
        }

        if (y1 != y2) [[unlikely]]
        {
            cellID = GetCellID(x1, y2);
            queryElements(cellID, elems);
        }
    }

    void clear()
    {
        cellMap.clear();
        dataBlocks.clear();
    }

    void reserve(const int cells, const int expectedTotalEntites)
    {
        cellMap.reserve(cells);
        dataBlocks.reserve(expectedTotalEntites / blockSize);
    }

private:
    void insertElement(const CellID id, V val)
    {
        const auto it = cellMap.find(id);
        int blockIdx;
        if (it == cellMap.end()) [[unlikely]] // Most elements should be together
        {
            blockIdx = static_cast<int>(dataBlocks.size());
            cellMap.insert({id, blockIdx});
            dataBlocks.push_back({});
        }
        else
        {
            blockIdx = it->second;
        }

        auto* block = &dataBlocks[blockIdx];

        while (block->hasNext())
        {
            block = &dataBlocks[block->next];
        }

        if (block->isFull()) [[unlikely]] // Only happens once each block
        {
            const auto nextIdx = static_cast<uint16_t>(dataBlocks.size());
            block->next = nextIdx;
            dataBlocks.push_back({});
            // Re allocation can invalidate the reference !!!!
            block = &dataBlocks[nextIdx];
        }

        block->add(val);
    }

    template <typename Container>
    void queryElements(const CellID id, Container& elems)
    {
        const auto it = cellMap.find(id);
        if (it == cellMap.end()) [[unlikely]] // Most elements should be together
        {
            return;
        }
        const int blockIdx = it->second;
        DataBlock<V, blockSize>* startBlock = nullptr;
        startBlock = &dataBlocks[blockIdx];

        startBlock->append(elems);
        while (startBlock->hasNext())
        {
            assert(startBlock->isFull());
            startBlock = &dataBlocks[startBlock->next];
            startBlock->append(elems);
        }
    }

    static_assert(std::is_trivially_constructible_v<V> && std::is_trivially_destructible_v<V>);
    static_assert(sizeof(V) <= 8, "You should only use small id types");
};

/*
template <typename ValueType, int blockSize = 16>
struct MultiResolutionHashGrid final
{

    int smallCellSize;
    int largeCellSize;

    MultiResolutionHashGrid(int smallSize, int largeSize) : smallCellSize(smallSize), largeCellSize(largeSize) {}

    void insertEntity(ValueType val, const float x, const float y, const int width, const int height)
    {
        if (width < largeCellSize && height < largeCellSize)
        {
            insertIntoGrid(val, x, y, width, height, smallCellSize, smallGrid);
        }
        else
        {
            insertIntoGrid(val, x, y, width, height, largeCellSize, largeGrid);
        }
    }

    void query(std::vector<ValueType>& collector, float x, float y, int width, int height)
    {
        queryGrid(x, y, width, height, largeCellSize, largeGrid);

        if (collector.empty()) // Nothing found in the bigger grid
            return;

        queryGrid(x, y, width, height, smallCellSize, smallGrid);
    }

    void clear() {}

    static CellID GetCellID(const int cellX, int cellY) { return static_cast<uint64_t>(cellX) << 32 | cellY; }

private:
    static void queryGrid(float x, float y, int w, int h, int cellSize, std::vector<ValueType>& coll, Map& grid)
    {
        int cx1 = static_cast<int>(x) / cellSize;
        int cy1 = static_cast<int>(y) / cellSize;
        int cx2 = (static_cast<int>(x) + w) / cellSize;
        int cy2 = (static_cast<int>(y) + h) / cellSize;

        coll.append_range(grid[GetCellID(cx1, cy1)]);

        if (cx1 != cx2) [[unlikely]]
        {
            coll.append_range(grid[GetCellID(cx2, cx1)]);

            if (cy1 != cy2)
            {
                hash = HashCell(cx1, cy2, cellSize);
                coll.append_range(grid[hash]);


                hash = HashCell(cx2, cy2, cellSize);
                coll.append_range(grid[hash]);
                return;
            }
        }

        if (cy1 != cy2)
        {
            hash = HashCell(cx1, cy2, cellSize);
            coll.append_range(grid[hash]);
        }
    }

    static void insertIntoGrid(ValueType val, float x, float y, int w, int h, int cellSize, Map& grid)
    {
        int x1 = static_cast<int>(x) / cellSize;
        int y1 = static_cast<int>(y) / cellSize;
        int x2 = (static_cast<int>(x) + w) / cellSize;
        int y2 = (static_cast<int>(y) + h) / cellSize;


        CellID hash = HashCell(x1, y1, cellSize);
        grid[hash].push_back(val);

        if (x1 != x2) [[unlikely]]
        {
            hash = HashCell(x2, y1, cellSize);
            grid[hash].push_back(val);

            if (y1 != y2)
            {
                hash = HashCell(x1, y2, cellSize);
                grid[hash].push_back(val);

                hash = HashCell(x2, y2, cellSize);
                grid[hash].push_back(val);
                return;
            }
        }

        if (y1 != y2)
        {
            hash = HashCell(x1, y2, cellSize);
            grid[hash].push_back(val);
        }
    }

    static int intersect(int l1, int t1, int r1, int b1, int l2, int t2, int r2, int b2)
    {
        return l2 <= r1 && r2 >= l1 && t2 <= b1 && b2 >= t1;
    }
};



*/

#endif //MULTIRESOLUTIONGRID_H