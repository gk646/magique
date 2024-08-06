#ifndef MULTIRESOLUTIONGRID_H
#define MULTIRESOLUTIONGRID_H

// This is a cache friendly "top-level" data structure
// https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
// Originally inspired by the above post to just move all the data of the structure to the top level
// This simplifies memory management and layout, it uses just a single vector for data
// This is achieved by mapping between dimensions, here between a cell and a memory block
// Now its still not perfect but definitely very fast
// Also the problem of multiple insertions is efficiently solved by accumulating with a hashmap
// Its almost mandatory to use a memory consistent map like a dense map thats a vector internally aswell
// This simplifies memory and thus cache friendlyness even more
// With this setup you have 0 (zero) allocations in game ticks which involves completely clearing and refilling grid

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
            elems.push_back(data[i]);
        }
    }
};

template <typename V, int blockSize = 16>
struct SingleResolutionHashGrid final
{
    magique::HashMap<CellID, int> cellMap{};
    magique::vector<DataBlock<V, blockSize>> dataBlocks{};
    int cellSize;

    explicit SingleResolutionHashGrid(const int cellSize) : cellSize(cellSize) {}

    void insert(V val, const float x, const float y, const float w, const float h)
    {
        const int x1 = static_cast<int>(x) / cellSize;
        const int y1 = static_cast<int>(y) / cellSize;
        const int x2 = static_cast<int>(x + w) / cellSize;
        const int y2 = static_cast<int>(y + h) / cellSize;

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
    void query(Container& elems, const float x, const float y, const float w, const float h) const
    {
        const int x1 = static_cast<int>(x) / cellSize;
        const int y1 = static_cast<int>(y) / cellSize;
        const int x2 = static_cast<int>(x + w) / cellSize;
        const int y2 = static_cast<int>(y + h) / cellSize;

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
    void queryElements(const CellID id, Container& elems) const
    {
        const auto it = cellMap.find(id);
        if (it == cellMap.end()) [[unlikely]] // Most elements should be together
        {
            return;
        }
        const int blockIdx = it->second;
        const DataBlock<V, blockSize>* startBlock = &dataBlocks[blockIdx];

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


#endif //MULTIRESOLUTIONGRID_H