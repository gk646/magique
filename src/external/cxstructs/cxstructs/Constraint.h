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
#ifndef CXSTRUCTS_SRC_CXSTRUCTS_CONSTRAINT_H_
#  define CXSTRUCTS_SRC_CXSTRUCTS_CONSTRAINT_H_

#  include <cstdlib>

namespace cxstructs {

//Fun little class to simply and cleanly concatenate the result of checks
template <bool CONSTRAINT>
class Constraint {
  bool currentVal = true;

 public:
  auto operator==(const Constraint& c) -> bool = delete;

  auto operator+=(const bool val) -> Constraint& {
    if (currentVal) {
      currentVal = val == CONSTRAINT;
    }
    return *this;
  }

  auto operator=(const Constraint& c) -> Constraint& {
    // Guard self assignment
    if (this == &c) {
      return *this;
    }
    currentVal = c.currentVal;
    return *this;
  }

  auto operator+(const bool val) -> void {
    if (currentVal == CONSTRAINT) {
      currentVal = val == CONSTRAINT;
    }
  }

  [[nodiscard]] auto holds() const -> bool { return currentVal; }

  auto exitcode() const -> int  {
    if (currentVal == CONSTRAINT) {
      return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
  }
};

}  // namespace cxstructs

#endif  //CXSTRUCTS_SRC_CXSTRUCTS_CONSTRAINT_H_