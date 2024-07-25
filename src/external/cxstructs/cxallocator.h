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
#define CX_FINISHED
#ifndef CXSTRUCTS_SRC_CXALLOCATOR_H_
#  define CXSTRUCTS_SRC_CXALLOCATOR_H_

/*
Use the last template option to specify whether to use the allocator or not cxstruct<Type,false>.
Generally use the CXAllocator if you use the cxstruct for longer and as a standalone.
In turn, generally do not use it if it's a temporary or fixed size structure
In case of slower performance just switch to the other.
 */

#  include <deque>
#  include <vector>
#  include "cxconfig.h"

namespace cxstructs {
template <uint_32_cx BlockSize, uint_16_cx ReservedBlocks>
class Pool {
  uint_32_cx size_;
  std::deque<void*> addrs_;
  std::vector<uint8_t*> blocks_;

  void allocateBlock() {
    auto* block = new uint8_t[BlockSize];
    for (uint_32_cx i = 0; i < BlockSize; i += size_) {
      addrs_.push_back(&block[i]);
    }
    blocks_.push_back(block);
  }

 public:
  explicit Pool(uint_32_cx size) : size_(size) {
    blocks_.reserve(ReservedBlocks * 2);
    for (uint_16_cx i = 0; i < ReservedBlocks; i++) {
      allocateBlock();
    }
  }
  ~Pool() {
    for (auto block : blocks_) {
      delete[] block;
    }
  }

  auto allocate() -> void* {
    if (addrs_.empty()) {
      allocateBlock();
    }
    auto ptr = addrs_.back();
    addrs_.pop_back();
    return ptr;
  }
  void deallocate(void* ptr) { addrs_.push_back(ptr); }
};

template <typename T, size_t BlockSize, uint_16_cx ReservedBlocks>
class CXPoolAllocator {
  using PoolType = Pool<BlockSize, ReservedBlocks>;
  PoolType pool_;

 public:
  using value_type = T;
  using is_always_equal = std::false_type;
  CXPoolAllocator() : pool_(PoolType(sizeof(T))) {}
  CXPoolAllocator(const CXPoolAllocator& other) = default;
  CXPoolAllocator(CXPoolAllocator&& other) noexcept = default;
  CXPoolAllocator& operator=(const CXPoolAllocator& other) = default;
  CXPoolAllocator& operator=(CXPoolAllocator&& other) noexcept = default;
  ~CXPoolAllocator() = default;
  T* allocate(size_t n) {
    if (n > 1) {
      return static_cast<T*>(malloc(sizeof(T) * n));
    } else {
      return (T*)pool_.allocate();
    }
  }
  void deallocate(T* ptr, size_t n) {
    if (n > 1) {
      free(ptr);
      return;
    } else {
      pool_.deallocate(ptr);
    }
  }
};
}  // namespace cxstructs
#endif  //CXSTRUCTS_SRC_CXALLOCATOR_H_
