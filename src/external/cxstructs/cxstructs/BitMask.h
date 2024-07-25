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
#define FINISHED
#ifndef CXSTRUCTS_SRC_CXSTRUCTS_BITFLAG_H_
#define CXSTRUCTS_SRC_CXSTRUCTS_BITFLAG_H_

#include "../cxconfig.h"
#include <initializer_list>

namespace cxstructs {

template <size_t N>
struct SuitableStorageType {
  // Default to uint64_t for N <= 64
  using type =
      std::conditional_t<N <= 8, uint8_t,
                         std::conditional_t<N <= 16, uint16_t, std::conditional_t<N <= 32, uint32_t, uint64_t>>>;
};

/**
 * BitMask for a given enum type
 *
 * Enum constant have to be power of 2:
 * enum Test { ONE = 1, TWO = 2, THREE = 4 };
 * @tparam E the enum
 * @tparam MAX_FLAGS maximum amount of expected flags (can be different from enum type)
 */
template <class E>
class EnumMask {
  E data_ = 0;

 public:
  void set(E flag) noexcept { data_ |= static_cast<E>(flag); }

  void unset(E flag) noexcept { data_ &= ~static_cast<E>(flag); }

  void toggle(E flag) noexcept { data_ ^= static_cast<E>(flag); }

  [[nodiscard]] bool isSet(E flag) const noexcept { return (data_ & static_cast<E>(flag)) != 0; }

  void clear() noexcept { data_ = 0; }

  [[nodiscard]] bool any() const noexcept { return data_ != 0x0; }

  //Compile time check(unfolding)
  template <E... Flags>
  [[nodiscard]] constexpr bool any_of() const noexcept {
    E compositeFlag = (0 | ... | static_cast<E>(Flags));
    return (data_ & compositeFlag) != 0;
  }

  //Compile time check(unfolding)
  template <E... Flags>
  [[nodiscard]] constexpr bool all_of() const noexcept {
    E compositeFlag = (0 | ... | static_cast<E>(Flags));
    return (data_ & compositeFlag) == compositeFlag;
  }

  //Runtime check
  [[nodiscard]] bool any_of(std::initializer_list<E> flags) const noexcept {
    for (auto flag : flags) {
      if ((data_ & static_cast<E>(flag)) != 0) {
        return true;
      }
    }
    return false;
  }

  //Runtime check
  [[nodiscard]] bool all_of(std::initializer_list<E> flags) const noexcept {
    for (auto flag : flags) {
      if ((data_ & static_cast<E>(flag)) == 0) {
        return false;
      }
    }
    return true;
  }
};

#ifdef CX_INCLUDE_TESTS
static void TEST_FLAG_CONTAINERS() {

  enum Test { ONE = 1, TWO = 2, THREE = 4 };
  EnumMask<Test, 1> flag;
  flag.set(ONE);
  CX_ASSERT(flag.isSet(ONE), "");
  flag.clear();
  CX_ASSERT(!flag.isSet(ONE), "");

  auto res = !flag.any_of<ONE, TWO>();
  CX_ASSERT(res, "");

  flag.set(THREE);
  CX_ASSERT(flag.any(), "");
  res = flag.any_of<THREE, TWO>();
  CX_ASSERT(res, "");

  flag.set(TWO);
  res = flag.all_of<THREE, TWO>();
  CX_ASSERT(res, "");
}
#endif

//-----------BLUEPRINT-----------//
/*
enum class MyFlags : uint32_t {
Flag1 = 1 << 0,
Flag2 = 1 << 1,
Flag3 = 1 << 2,
Flag4 = 1 << 3,
Flag5 = 1 << 4,
Flag6 = 1 << 5,
Flag7 = 1 << 6,
Flag8 = 1 << 7,
Flag9 = 1 << 8,
Flag10 = 1 << 9,
Flag11 = 1 << 10,
Flag12 = 1 << 11,
Flag13 = 1 << 12,
Flag14 = 1 << 13,
Flag15 = 1 << 14,
Flag16 = 1 << 15,
Flag17 = 1 << 16,
Flag18 = 1 << 17,
Flag19 = 1 << 18,
Flag20 = 1 << 19,
Flag21 = 1 << 20,
Flag22 = 1 << 21,
Flag23 = 1 << 22,
Flag24 = 1 << 23,
Flag25 = 1 << 24,
Flag26 = 1 << 25,
Flag27 = 1 << 26,
Flag28 = 1 << 27,
Flag29 = 1 << 28,
Flag30 = 1 << 29,
Flag31 = 1 << 30,
Flag32 = 1U << 31,
};
 */
}  // namespace cxstructs
#endif  //CXSTRUCTS_SRC_CXSTRUCTS_BITFLAG_H_