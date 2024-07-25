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
#ifndef CXSTRUCTS_DFS_H
#  define CXSTRUCTS_DFS_H

#  include <iostream>
#  include <unordered_map>
#  include <utility>
#  include <vector>
#  include "../cxconfig.h"
#  include "../cxstructs/HashSet.h"
#  include "../cxstructs/Pair.h"
#  include "../cxstructs/vec.h"

namespace cxstructs {

/**
 * Treats the vector as an adjacency matrix
 * @return - the number of reachable nodes from the given start node
 */
template <typename T>
int depth_first_search(std::vector<std::vector<T>>& mat, int nodeIndex,
                       std::vector<bool> vis = {}) {
  int count = 1;
  if (mat.empty() || nodeIndex >= mat.size()) return 0;
  if (vis.empty()) {
    count++;
    vis = std::vector(mat.size(), false);
  }

  for (int i = 0; i < mat[nodeIndex].size(); ++i) {
    if (mat[nodeIndex][i] == 1 && !vis[i]) {
      vis[i] = true;
      count += depth_first_search(mat, nodeIndex, vis);
    }
  }
  return count;
}
template <typename State, typename Action>
vec<vec<std::pair<Action, State>>>
shortest_path_search(State start, std::unordered_map<State, Action> (*successors)(State),
                     bool (*is_goal)(State)) {
  if (is_goal(start)) {
    return {{Action(), start}};
  }
  auto explored = HashSet<Action, State>();
  auto frontier = vec<std::pair<Action, State>>();
  while (!frontier.empty()) {
    auto path = frontier.pop_front();
    auto s = path.at[-1].second;
  }
}
}  // namespace cxstructs
#  ifdef CX_INCLUDE_TESTS
namespace cxtests {  // namespace cxtests
static void TEST_DFS() {
  std::cout << "TESTING DEPTH FIRST SEARCH" << std::endl;
  std::vector<std::vector<int>> mat = {
      {0, 1, 0, 1, 0}, {1, 0, 1, 0, 0}, {0, 1, 0, 0, 0}, {1, 0, 0, 0, 1}, {0, 0, 0, 1, 0}};

  // Perform depth_first_search from node 0
  int reachableNodes = depth_first_search(mat, 0);

  CX_ASSERT(reachableNodes == 5, "");
}
}  // namespace cxtests
#  endif

#endif  // CXSTRUCTS_DFS_H
