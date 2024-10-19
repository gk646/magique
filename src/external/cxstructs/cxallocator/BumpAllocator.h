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

#ifndef BUMPALLOCATOR_H
#define BUMPALLOCATOR_H


namespace cxstructs
{
    struct DynamicBumpAllocator final
    {
        unsigned char* memory = nullptr;
        size_t size = 0;
        size_t capacity = 0;

        explicit DynamicBumpAllocator(const int reserve = 1) : capacity(reserve)
        {
            assert(reserve > 0 && "Must allocate at least some memory");
            memory = static_cast<unsigned char*>(malloc(reserve));
        }

        ~DynamicBumpAllocator() { free(memory); }

        // Returns a valid memory region with size 'sizeof(T)'
        template <typename T>
        T* allocate()
        {
            if (capacity < size + sizeof(T)) [[unlikely]]
            {
                assert(false && "Too many bytes requested");
                exit(1);
            }
            const auto ptr = memory + size;
            size += sizeof(T);
            return static_cast<T*>(ptr);
        }

        // Returns a valid memory region with size 'bytes'
        void* allocate(const size_t bytes)
        {
            if (capacity < size + bytes) [[unlikely]]
            {
                assert(false && "Too many bytes requested");
                exit(1);
            }
            void* ptr = memory + size;
            size += bytes;
            return ptr;
        }

        // Resets the allocator to its start
        void reset() { size = 0; }
    };

} // namespace cxstructs
#endif //BUMPALLOCATOR_H