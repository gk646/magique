// Copyright (c) 2023 gk646
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef SLOT_ALLOCATOR_H
#define SLOT_ALLOCATOR_H

namespace cxstructs
{
    // Slot allocator caters to the usage pattern of having reoccurring allocations with the same size
    template <int N>
    struct SlotAllocator final
    {
        struct MemorySlot final
        {
            unsigned char* memory = nullptr;
            int size = 0;
            bool used = false;
        };

        MemorySlot slots[N]{};
        int size = 0;

        ~SlotAllocator() { destroy(); }

        // Returns a valid memory region with size 'sizeof(T)'
        template <typename T>
        T* allocate()
        {
            return allocate(sizeof(T));
        }

        // Returns a valid memory region with size 'bytes'
        void* allocate(const size_t bytes)
        {
            for (int i = 0; i < N; i++)
            {
                auto& slot = slots[i];
                if (!slot.used && slot.size == static_cast<int>(bytes))
                {
                    slot.used = true;
                    return slot.memory;
                }
            }
            if (size >= N) [[unlikely]]
            {
                assert(false && "Too many slots requested");
                exit(1);
            }
            slots[size] = MemorySlot{(unsigned char*)malloc(bytes), (int)bytes, true};
            return slots[size++].memory;
        }

        void free(const void* ptr)
        {
            for (int i = 0; i < N; i++)
            {
                auto& slot = slots[i];
                if (slot.used && slot.memory == ptr)
                {
                    slot.used = false;
                    return;
                }
            }
        }

        // Resets the allocator to its start
        void destroy()
        {
            for (auto& slot : slots)
            {
                free(slot.memory);
                slot.used = false;
                slot.size = 0;
            }
        }
    };

} // namespace cxstructs

#endif //SLOT_ALLOCATOR_H