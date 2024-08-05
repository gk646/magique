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
#ifndef SMALLVECTOR_H
#define SMALLVECTOR_H

#include <cstdint>
#include <cstring>
#include <cassert>

template <typename T>
class VectorIterator {
 public:
  using difference_type = int64_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  VectorIterator(pointer ptr) : ptr_(ptr) {}

  reference operator*() const { return *ptr_; }
  pointer operator->() { return ptr_; }

  // Prefix increment
  VectorIterator& operator++() {
    ++ptr_;
    return *this;
  }
  // Postfix increment
  VectorIterator operator++(int) {
    VectorIterator tmp = *this;
    ++*this;
    return tmp;
  }

  // Prefix decrement
  VectorIterator& operator--() {
    --ptr_;
    return *this;
  }
  // Postfix decrement
  VectorIterator operator--(int) {
    VectorIterator tmp = *this;
    --*this;
    return tmp;
  }

  // Arithmetic operators
  VectorIterator& operator+=(difference_type offset) {
    ptr_ += offset;
    return *this;
  }
  VectorIterator operator+(difference_type offset) const { return VectorIterator(ptr_ + offset); }
  VectorIterator& operator-=(difference_type offset) {
    ptr_ -= offset;
    return *this;
  }
  VectorIterator operator-(difference_type offset) const { return VectorIterator(ptr_ - offset); }
  difference_type operator-(const VectorIterator& other) const { return ptr_ - other.ptr_; }

  reference operator[](difference_type index) const { return *(ptr_ + index); }

  // Comparison operators
  bool operator==(const VectorIterator& other) const { return ptr_ == other.ptr_; }
  bool operator!=(const VectorIterator& other) const { return ptr_ != other.ptr_; }
  bool operator<(const VectorIterator& other) const { return ptr_ < other.ptr_; }
  bool operator<=(const VectorIterator& other) const { return ptr_ <= other.ptr_; }
  bool operator>(const VectorIterator& other) const { return ptr_ > other.ptr_; }
  bool operator>=(const VectorIterator& other) const { return ptr_ >= other.ptr_; }

 private:
  pointer ptr_;
};

namespace cxstructs {
template <typename T, uint32_t N = 16, typename size_type = uint16_t>
struct SmallVector {
  SmallVector() : size_(0), capacity_(N), data_(stack_data_) {}

  ~SmallVector() {
    if (data_ != stack_data_) {
      delete[] data_;
    }
  }

  void push_back(const T& value) {
    ensure_capacity(size_ + 1);
    data_[size_++] = value;
  }

  void pop_back() {
    assert(size_ > 0);
    --size_;
  }

  // ensures capacity - initializes all new values
  void resize(size_type new_size, const T& val) {
    if (new_size > size_) {
      ensure_capacity(new_size);
      for (size_type i = size_; i < new_size; ++i) {
        data_[i] = val;
      }
      size_ = new_size;
    }
  }

  T& operator[](size_type index) {
    assert(index < size_);
    return data_[index];
  }

  const T& operator[](size_type index) const {
    assert(index < size_);
    return data_[index];
  }

  size_type size() const { return size_; }

  size_type capacity() const { return capacity_; }

  const T* data() const { return data_; }

  T* data() { return data_; }

  [[nodiscard]] bool empty() const { return size_ == 0; }

  using iterator = VectorIterator<T>;
  using const_iterator = VectorIterator<const T>;

  iterator begin() { return iterator(data_); }
  const_iterator begin() const { return const_iterator(data_); }
  iterator end() { return iterator(data_ + size_); }
  const_iterator end() const { return const_iterator(data_ + size_); }

 private:
  void ensure_capacity(size_type new_capacity) {
    if (new_capacity <= capacity_) {
      return;
    }

    new_capacity = capacity_ * 2;
    T* new_data = new T[new_capacity];

    std::memcpy(new_data, data_, size_ * sizeof(T));

    if (data_ != stack_data_) {
      delete[] data_;
    }

    data_ = new_data;
    capacity_ = new_capacity;
  }

  size_type size_;
  size_type capacity_;
  T* data_;
  T stack_data_[N];
};
}  // namespace cxstructs
#endif  //SMALLVECTOR_H