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
#ifndef CXSTRUCTS_SRC_CXALGOS_MISC_H_
#  define CXSTRUCTS_SRC_CXALGOS_MISC_H_

#  include <ctime>
#  include <vector>
#  include "../cxstructs/Geometry.h"

namespace cxstructs {
template <typename S, typename B>
std::vector<std::vector<S>> maze_simple_generation(const B& obstacle_val, Point size, Point start,
                                                   Point end) {
  std::srand(std::time(nullptr));

  // Initialize the maze with random walls and empty spaces
  std::vector<std::vector<S>> maze(size.y(), std::vector<S>(size.x()));
  for (int y = 0; y < size.y(); y++) {
    for (int x = 0; x < size.x(); x++) {
      if (std::rand() % 2 == 0) {
        maze[y][x] = obstacle_val;
      }
    }
  }

  int x = start.x();
  int y = start.y();
  while (x != end.x() || y != end.y()) {
    maze[y][x] = S();

    if (x < end.x()) {
      x++;
    } else if (x > end.x()) {
      x--;
    } else if (y < end.y()) {
      y++;
    } else if (y > end.y()) {
      y--;
    }
  }
  maze[end.y()][end.x()] = S();
  return maze;
}

}  // namespace cxstructs
#endif  //CXSTRUCTS_SRC_CXALGOS_MISC_H_
