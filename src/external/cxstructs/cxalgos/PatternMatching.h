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
#ifndef CXSTRUCTS_SRC_ALGORITHMS_PATTERNMATCHING_H_
#  define CXSTRUCTS_SRC_ALGORITHMS_PATTERNMATCHING_H_

#  include <algorithm>
#  include <string>
#  include "../cxconfig.h"

namespace cxhelper {

inline bool is_prefix(const std::string& pattern, int pos) {
  for (int i = 0; i < pattern.size() - pos; ++i) {
    if (pattern[i] != pattern[i + pos]) {
      return false;
    }
  }
  return true;
}
inline int suffix_length(const std::string& pattern, int pos) {
  int len = 0;
  while (pos >= 0 && pattern[pos] == pattern[pattern.size() - 1 - len]) {
    pos--;
    len++;
  }
  return len;
}

}  // namespace cxhelper

namespace cxstructs {
using namespace cxhelper;

inline int findString_built_in(const std::string& text, const std::string& pattern) {
  if (text.empty() || pattern.empty()) {
    return -1;  // return -1 on empty input
  }

  int count = 0;
  size_t pos = 0;
  //find() returns the first occurrence of the given pattern starting at pos
  while ((pos = text.find(pattern, pos)) != std::string::npos) {
    count++;
    pos++;
  }
  return count > 0 ? count : -1;  // return -1 on not found
}
inline int findString_brute_force(const std::string& text, const std::string& pattern) {
  if (text.empty() || pattern.empty()) {
    return -1;  // return -1 on empty input
  }
  //unsigned integers for better performance
  int_fast32_t count = 0;
  uint_16_cx len = pattern.size();
  uint_32_cx text_len = text.size() - len;
  uint_32_cx i = 0;

  while (i < text_len) {
    for (uint_32_cx j = 0; j < len; j++) {
      if (text[i + j] == pattern[j]) {
        if (j == len - 1) {
          count++;
        }
      } else {
        break;
      }
    }
    i++;
  }
  return count > 0 ? count : -1;  // return -1 on not found
}
inline int findString_KMP(const std::string& text, const std::string& pattern) {
  if (text.empty() || pattern.empty()) {
    return -1;  // return -1 on empty input
  }

  int count = 0;
  uint_16_cx len = pattern.size();
  uint_32_cx text_len = text.size();
  auto* lps = new uint8_t[len];  //lps array

  lps[0] = 0;
  int index = 1;
  int longestPrefix = 0;

  //create the lps array
  while (index < len) {
    if (pattern[index] == pattern[longestPrefix]) {
      longestPrefix++;
      lps[index] = longestPrefix;
      index++;
    } else {
      if (longestPrefix != 0) {
        longestPrefix = lps[longestPrefix - 1];
      } else {
        lps[index] = 0;
        index++;
      }
    }
  }

  //search operation
  for (uint_32_cx i = 0; i < text_len; i++) {
    for (uint_32_cx j = 0; j < len; j++) {
      if (text[i + j] == pattern[j]) {  //compare from i+j onwards
        if (j == len - 1) {
          count++;
        }
      } else {
        i += lps[j];  //skip lps[j] amount
        break;
      }
    }
  }
  delete[] lps;                   //cleanup to prevent memory leaks
  return count > 0 ? count : -1;  // return -1 on not found
}
inline int findString_Boyer_Moore(const std::string& text, const std::string& pattern) {
  if (text.empty() || pattern.empty()) {
    return -1;  // return -1 on empty input
  }

  int count = 0;
  uint_16_cx len = pattern.size();
  uint_32_cx text_len = text.size();

  int bad_char[128];                         //stack array due to fixed size
  std::fill(bad_char, bad_char + 128, len);  //fill array with default values

  //produce bad character table
  for (uint_32_cx i = 0; i < len; i++) {
    bad_char[static_cast<uint8_t>(pattern[i]) & 0x7F] =
        len - 1
        - i;  //cast number to 0-256 and bitwise and with 01111111 to zero the first bit / from my Trie implementation
  }

  auto* good_suffix = new int[len];  //dynamic array
  std::fill(good_suffix, good_suffix + len,
            len);  //fill array with default values

  //calculation of the good suffix shift table
  int last_prefix_pos = len;
  for (int i = len; i > 0; --i) {
    if (is_prefix(pattern, i)) {
      last_prefix_pos = i;
    }
    good_suffix[pattern.size() - i] = last_prefix_pos - i + pattern.size();
  }

  for (int i = 0; i < pattern.size() - 1; ++i) {
    int suffix_len = suffix_length(pattern, i);
    good_suffix[suffix_len] = pattern.size() - 1 - i + suffix_len;
  }

  len--;  //precalculate outside the loop
  uint_32_cx i = 0, j = 0;
  while (i < text_len) {
    for (j = len; pattern[j] == text[i]; --i, --j) {
      if (j == 0) {
        count++;
        break;
      }
    }
    i += std::max(good_suffix[len - j], bad_char[static_cast<uint8_t>(text[i]) & 0x7F]);
  }
  delete[] good_suffix;           //clean up memory
  return count > 0 ? count : -1;  // return -1 on not found
}
}  // namespace cxstructs

#endif  //CXSTRUCTS_SRC_ALGORITHMS_PATTERNMATCHING_H_
