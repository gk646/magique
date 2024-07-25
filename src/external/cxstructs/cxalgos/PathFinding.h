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
#ifndef CXSTRUCTS_ASTAR_PATHFINDING_H
#  define CXSTRUCTS_ASTAR_PATHFINDING_H

#  include <cstdint>
#  include <queue>
#  include <type_traits>
#  include "../cxconfig.h"
#  include "../cxstructs/Geometry.h"
#  include "../cxstructs/HashSet.h"
#  include "../cxstructs/Pair.h"
#  include "../cxstructs/PriorityQueue.h"

namespace cxhelper {
using namespace cxstructs;
struct Node {
  Point position;
  uint16_t f_cost;
  uint16_t g_cost;
  uint16_t h_cost;
  Node* parent;
  Node() {}
  Node(const Node& n)
      : f_cost(n.f_cost), position(n.position), parent(n.parent), h_cost(n.h_cost),
        g_cost(n.g_cost) {}
  Node(Point position, uint16_t g_cost, uint16_t h_cost, Node* parent)
      : position(position), g_cost(g_cost), h_cost(h_cost), f_cost(g_cost + h_cost),
        parent(parent) {}

  inline bool operator<(const Node& o) const { return f_cost < o.f_cost; }
  inline bool operator>(const Node& o) const { return f_cost > o.f_cost; }
  inline bool operator==(const Node& o) const { return f_cost == o.f_cost; }
  inline bool operator<=(const Node& o) const { return f_cost <= o.f_cost; }
};

inline std::vector<Point> reconstruct_path(Node* target_node) {
  std::vector<Point> path;
  Node* node = target_node;
  while (node) {
    path.push_back(node->position);
    node = node->parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}

}  // namespace cxhelper

namespace cxstructs {
/**
 * <h2>A star</h2> is a pathfinding algorithm using clever heuristics to find the shortest path.<p>
 * It generally works by having a field of nodes which is the search space and then giving each node
 * a g_cost ( the distance form the start) and a h_cost ( mostly the distance form the target).<p>
 * These are then added to generate a score for the nodes quality. Each step the node with the best score is removed and check for being the end position. If not up 4 new nodes ( the ones around the node without diagonals)
 * are added. This repeats until either all nodes are visited or the end was found.
 *
 * @tparam S field type
 * @tparam B  blocked value type (can be same type as S)
 * @param field the 2D search space
 * @param blocked_val a arbitrary value to represent an obstacle
 * @param start the starting point
 * @param target the target point
 * @return the shortest path from start to target in Points
 */
using namespace cxhelper;
template <typename S, typename B>
std::vector<Point> astar_pathfinding(const std::vector<std::vector<S>>& field, const B& blocked_val,
                                     const Point& start, const Point& target) {
  PriorityQueue<Node> frontier;
  HashSet<Point> closedSet;
  vec<row<2, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
  frontier.emplace(start, 0, start.dist(target), nullptr);
  while (!frontier.empty()) {
    Node* current = new Node(frontier.top());
    frontier.pop();

    if (current->position == target) {
      return reconstruct_path(current);
    }

    closedSet.insert(current->position);

    for (const auto& dir : directions) {
      int newX = static_cast<int>(current->position.x()) + dir[0];
      int newY = static_cast<int>(current->position.y()) + dir[1];
      Point new_pos(newX, newY);
      if (newX >= 0 && newY < field.size() && newY >= 0 && newX < field[0].size()
          && field[newY][newX] != blocked_val && !closedSet.contains(new_pos)) {

        uint16_t tentative_g_cost = current->g_cost + 1;
        uint16_t h_cost =
            abs(newX - static_cast<int>(target.x())) + abs(newY - static_cast<int>(target.y()));

        frontier.emplace(new_pos, tentative_g_cost, h_cost, current);
      }
    }
  }
  return {};
}

}  // namespace cxstructs
#  ifdef CX_INCLUDE_TESTS
namespace cxtests {  // namespace cxtests
using namespace cxstructs;
static void TEST_PATH_FINDING() {
  std::cout << "TESTING PATHFINDING" << std::endl;
  std::vector<std::vector<int>> maze = {{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                                        {1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1},
                                        {1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1},
                                        {1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1},
                                        {1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
                                        {1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1},
                                        {1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1},
                                        {1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
                                        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
                                        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}

  };
  Point start(1, 1);
  Point target(11, 8);

  auto path = astar_pathfinding(maze, 1, start, target);

  CX_ASSERT(path[path.size() - 3] == Point(11, 6), "");
}
}  // namespace cxtests
#  endif

#endif  //CXSTRUCTS_ASTAR_PATHFINDING_H
