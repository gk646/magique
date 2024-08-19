#ifndef AABBTREE_H
#define AABBTREE_H

template <typename T>
struct ColliderStorage final
{
    template <int size>
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
        void query(Container& elems) const
        {
            for (int i = 0; i < count; ++i)
            {
                elems.push_back(data[i]);
            }
        }
    };

    magique::vector<DataBlock<7>> dataBlocks; // Two cache lines

    void insert(const T val, int num)
    {
        assert(num < dataBlocks.size() && "Oout ");
        auto* block = &dataBlocks[num];

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
};

struct AABBTree final
{
    struct Node final
    {
        float x, y, w, h;              // Bounding box for the node
        uint16_t left = UINT16_MAX;    // Index of the left child node
        uint16_t right = UINT16_MAX;   // Index of the right child node
        uint16_t blockID = UINT16_MAX; // Index of the block
        bool isLeaf() const { return left == -1 && right == -1; }
    };

    ColliderStorage<magique::StaticCollider> storage;
    magique::vector<Node> nodes;

    void insert(float x, float y, float w, float h)
    {

    }

    magique::CollisionInfo query(float x, float y, float w, float h) {}
};

#endif //AABBTREE_H