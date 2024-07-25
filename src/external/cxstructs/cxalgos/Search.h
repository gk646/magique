// Copyright (c) 2023 gk646
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#define CX_FINISHED
#ifndef CXSTRUCTS_BINARYSEARCH_H
#define CXSTRUCTS_BINARYSEARCH_H

#include <cstdint>
#include "../cxconfig.h"

namespace cxhelper {  // helper methods to provide clean calling interface
template <typename T>
bool binarySearch_recursive_internal(T* arr, T target, int_32_cx low, int_32_cx high) {
  if (low > high) {
    return false;
  }
  int_32_cx mid = low + (high - low) / 2;

  if (arr[mid] == target) {
    return true;
  } else if (arr[mid] < target) {
    return binarySearch_recursive_internal(arr, target, mid + 1, high);
  } else {
    return binarySearch_recursive_internal(arr, target, low, mid - 1);
  }
}
}  // namespace cxhelper

namespace cxstructs {
/**
 * Binary search on the specified ASCENDED SORTED array without recursion <p>
 * runtime: O(log(n))
 * @tparam T the used datatype
 * @param arr search array
 * @param target target value to search for
 * @param len the length of the given array
 * @return true if the target was found inside arr_
 */
template <typename T>
bool binary_search(T* arr, T target, int_32_cx len) {
  int_32_cx low = 0;
  int_32_cx high = len - 1;
  int_32_cx mid;
  while (low <= high) {
    mid = low + (high - low) / 2;
    if (arr[mid] == target) {
      return true;
    }
    if (arr[mid] < target) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }
  return false;
}

/**
 * Binary search on the specified ASCENDED SORTED array with recursion <p>
* runtime: O(log(n))<p>
 *
 * Recursion is generally slower and more memory intensive
 * @tparam T the used datatype
 * @param arr search array
 * @param target target value to search for
 * @param len the length of the given array
 * @return true if the target was found inside arr_
 */
template <typename T>
bool binary_search_recursive(T* arr, T target, int_32_cx len) {
  if (len == 0) {
    return false;
  }
  return cxhelper::binarySearch_recursive_internal(arr, target, 0, len - 1);
}

// Returns the index at which the element should be inserted
template <typename T>
int binary_search_index(T* arr, T target, int_32_cx len, bool ascending) {
  if (ascending) {
    int_32_cx low = 0;
    int_32_cx high = len;
    while (low < high) {
      int_32_cx mid = low + (high - low) / 2;
      if (arr[mid] < target) {
        low = mid + 1;
      } else {
        high = mid;
      }
    }
    return low;
  }

  int_32_cx low = 0;
  int_32_cx high = len;

  while (low < high) {
    int_32_cx mid = low + (high - low) / 2;
    if (arr[mid] > target) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }

  return low;
}

}  // namespace cxstructs
#endif  // CXSTRUCTS_BINARYSEARCH_H