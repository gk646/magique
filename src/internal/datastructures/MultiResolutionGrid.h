#ifndef MULTI_RESOLUTION_GRID_H
#define MULTI_RESOLUTION_GRID_H

// This is a cache friendly "top-level" data structure
// https://stackoverflow.com/questions/41946007/efficient-and-well-explained-implementation-of-a-quadtree-for-2d-collision-det
// Originally inspired by the above post to just move all the data of the structure to the top level
// This simplifies memory management and layout, it uses just a single vector for data
// This is achieved by mapping between dimensions, here between a cell and a memory block
// Now It's still not perfect but definitely very fast
// Also the problem of multiple insertions is efficiently solved by accumulating with a hashmap
// Its almost mandatory to use a memory consistent map like a dense map that's a vector internally as well
// This simplifies memory and thus cache friendliness even more
// With this setup you have 0 (zero) allocations in game ticks which involves completely clearing and refilling grid

using CellID = int64_t;
// This creates a unique value - both values are unique themselves so their concatenated version is as well
// We still use a hash function on it to get more byte range (all bits flipped)
inline CellID GetCellID(const int cellX, const int cellY) { return static_cast<int64_t>(cellX) << 32 | cellY; }

template <int cellSize, typename IdFunc, typename Func>
void RasterizeRect(Func func, IdFunc idFunc, const float x, const float y, const float w, const float h)
{
    if (w > cellSize || h > cellSize) [[unlikely]] // It's bigger than a single cell
    {
        if (w > cellSize * 2 || h > cellSize * 2) [[unlikely]] // Unlimited cells
        {
            const int startX = static_cast<int>(x) / cellSize;
            const int startY = static_cast<int>(y) / cellSize;
            const int endX = static_cast<int>(x + w) / cellSize;
            const int endY = static_cast<int>(y + h) / cellSize;
            for (int i = startY; i <= endY; ++i)
            {
                for (int j = startX; j <= endX; ++j)
                {
                    func(idFunc(j, i));
                }
            }
            return;
        }
        // 4 corners, the 4 middle points of the edges and the middle pointer -> 9 potential cells
        const int x1 = static_cast<int>(x) / cellSize;
        const int y1 = static_cast<int>(y) / cellSize;
        const int x2 = static_cast<int>(x + w) / cellSize;
        const int y2 = static_cast<int>(y + h) / cellSize;

        const int xhalf = static_cast<int>(x + w / 2.0F) / cellSize;
        const int yhalf = static_cast<int>(y + h / 2.0F) / cellSize;
        // Process the corners
        func(idFunc(x1, y1)); // Top-left
        if (x1 != x2)
            func(idFunc(x2, y1)); // Top-right
        if (y1 != y2)
            func(idFunc(x1, y2)); // Bottom-left
        if (x1 != x2 && y1 != y2)
            func(idFunc(x2, y2)); // Bottom-right

        // edge midpoints
        if (xhalf != x1 && xhalf != x2)
        {
            func(idFunc(xhalf, y1)); // Top-edge midpoint
            if (y1 != y2)
                func(idFunc(xhalf, y2)); // Bottom-edge midpoint
        }
        if (yhalf != y1 && yhalf != y2)
        {
            func(idFunc(x1, yhalf)); // Left-edge midpoint
            if (x1 != x2)
                func(idFunc(x2, yhalf)); // Right-edge midpoint
        }

        // Process the center point
        if (xhalf != x1 && xhalf != x2 && yhalf != y1 && yhalf != y2)
            func(idFunc(xhalf, yhalf));
        return;
    }
    // Its smaller than a cell -> maximum of 4 cells
    const int x1 = static_cast<int>(x) / cellSize;
    const int y1 = static_cast<int>(y) / cellSize;
    const int x2 = static_cast<int>(x + w) / cellSize;
    const int y2 = static_cast<int>(y + h) / cellSize;

    func(idFunc(x1, y1));

    if (x1 != x2)
    {
        func(idFunc(x2, y1));

        if (y1 != y2)
        {
            func(idFunc(x1, y2));
            func(idFunc(x2, y2));
            return;
        }
    }

    if (y1 != y2)
    {
        func(idFunc(x1, y2));
    }
}

template <typename T, int size>
struct DataBlock final
{
    static constexpr int NO_NEXT_BLOCK = UINT16_MAX;
    T data[size];                  // Fixed size data block
    uint16_t count = 0;            // Current number of elements
    uint16_t next = NO_NEXT_BLOCK; // Index of the next block or NO_NEXT_BLOCK if if its the end

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
        const auto start = data;
        const auto end = data + count;
        for (auto it = start; it != end; ++it)
        {
            elems.push_back(*it);
        }
    }

    void remove(T val)
    {
        if (count == 1 && data[0] == val)
        {
            count = 0;
            return;
        }

        for (int i = 0; i < count; ++i)
        {
            if (data[i] == val)
            {
                data[i] = data[count - 1];
                --count;
                --i;
            }
        }
    }

    template <typename NewType, typename Pred>
    void removeIf(NewType val, Pred pred)
    {
        if (count == 1 && pred(val, data[0]))
        {
            count = 0;
            return;
        }

        for (int i = 0; i < count; ++i)
        {
            if (pred(val, data[i]))
            {
                data[i] = data[count - 1];
                --count;
                --i;
            }
        }
    }
};

// assuming 4 bytes as value size its 15 * 4 + 2 + 2 = 64 / one cache line
template <typename V, int blockSize = 15, int cellSize = 64 /*power of two is optimized*/>
struct SingleResolutionHashGrid final
{
    magique::HashMap<CellID, int> cellMap{};
    std::vector<DataBlock<V, blockSize>> dataBlocks{};

    void insert(V val, const float x, const float y, const float w, const float h)
    {
        const auto insertFunction = [this, val](const CellID cellID) { insertElement(cellID, val); };
        RasterizeRect<cellSize>(insertFunction, GetCellID, x, y, w, h);
    }

    template <typename Container>
    void query(Container& elems, const float x, const float y, const float w, const float h) const
    {
        const auto queryFunction = [this, &elems](const CellID cellID) { queryElements(cellID, elems); };
        RasterizeRect<cellSize>(queryFunction, GetCellID, x, y, w, h);
    }

    void clear()
    {
        cellMap.clear();
        dataBlocks.clear();
    }

    // This is only efficient when no elements are inserted anymore until the next clear - Leaves holes
    void removeWithHoles(V val)
    {
        for (const auto& pair : cellMap) // Iterate through map to only work on 'root' blocks
        {
            auto& block = dataBlocks[pair.second];
            DataBlock<V, blockSize>* start = &block;
            start->remove(val);
            while (start->hasNext())
            {
                start = &dataBlocks[start->next];
                start->remove(val);
            }
        }
    }

    template <typename T, typename Pred>
    void removeIfWithHoles(T val, Pred pred)
    {
        for (const auto& pair : cellMap) // Iterate through map to only work on 'root' blocks
        {
            auto& block = dataBlocks[pair.second];
            DataBlock<V, blockSize>* start = &block;
            start->removeIf(val, pred);
            while (start->hasNext())
            {
                start = &dataBlocks[start->next];
                start->removeIf(val, pred);
            }
        }
    }

    // Patches the blocks removing any holes
    void patchHoles()
    {
        for (const auto& pair : cellMap) // Iterate through map to only work on 'root' blocks
        {
            patchBlockChain(dataBlocks[pair.second]);
        }
    }

    void reserve(const int cells, const int expectedTotalEntites)
    {
        cellMap.reserve(cells);
        dataBlocks.reserve(expectedTotalEntites / blockSize);
    }

    [[nodiscard]] constexpr int getBlockSize() const { return blockSize; }
    [[nodiscard]] constexpr int getCellSize() const { return cellSize; }

private:
    void patchBlockChain(DataBlock<V, blockSize>& startBlock)
    {
        DataBlock<V, blockSize>* start = &startBlock;
        DataBlock<V, blockSize>* next = nullptr;
        while (start->hasNext())
        {
            next = &dataBlocks[start->next];
            auto count = start->count;
            uint16_t i = 0;
            for (; i + count < blockSize && i < next->count; ++i) // Copy elements from next to current
            {
                start->data[i + count] = next->data[i];
            }
            start->count = count + i;
            next->count -= i;
            memcpy(next->data, next->data + i, sizeof(V) * next->count); // Shift elements to the front

            if (start->count < blockSize)
            {
                assert(next->count == 0 && "if current is not filled next has to be empty");
                start->next = DataBlock<V, blockSize>::NO_NEXT_BLOCK;
                break;
            }
            start = next;
        }
        if (start->count < blockSize)
        {
            start->next = DataBlock<V, blockSize>::NO_NEXT_BLOCK;
        }
    }

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

#endif //MULTI_RESOLUTION_GRID_H