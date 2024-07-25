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
#ifndef CXSTRUCTS_SRC_MACHINELEARNING_K_NN_H_
#  define CXSTRUCTS_SRC_MACHINELEARNING_K_NN_H_

#  include <exception>
#  include <vector>
#  include "../cxconfig.h"
#  include "../cxstructs/Geometry.h"
#  include "../cxstructs/HashMap.h"
#  include "../cxstructs/QuadTree.h"
#  include "../cxstructs/row.h"
/**
 * <h2>k-Nearest Neighbour</h2>
 *
 * is a simple instance-based machine learning algorithm that tries to classify new data points by comparing
 * it to existing ones. Specifically to k number of closest points, hence the name.<p>
 *
 */
namespace cxstructs {

enum class DISTANCE_FUNCTION_2D { EUCLIDEAN, MANHATTAN };
enum class DISTANCE_FUNCTION_XD { EUCLIDEAN, MANHATTAN, CHEBYSHEV, COSINE };

/**
 * @tparam C the categories
 */
template <typename C>  // category
struct DataPoint_ {
  using Category = C;
  virtual float x() const = 0;
  virtual float y() const = 0;
  virtual float getWeight() const = 0;
  virtual C getCategory() = 0;
};

template <typename DP_>
class kNN_2D {
  using Category = typename DP_::Category;
  D_func dist_func;
  QuadTree<DP_> space;

  uint_32_cx n_points;
  DP_* data_ptr;

  inline void get_k_closest(float x, float y, int k, vec<DP_*>& k_closest) {
    Rect search_space = {x, y, 1, 1};

    while (k_closest.size() < k) {  //expanding till at least k size
      k_closest = space.get_subrect(search_space);
      search_space.x()--;
      search_space.y()--;
      search_space.height() += 2;
      search_space.width() += 2;
    }

    if (k_closest.size() > k) {
      auto c_dist_func = dist_func;
      //rearranging with custom comparator, ascending to distance
      std::nth_element(k_closest.begin(), k_closest.begin() + k, k_closest.end(),
                       [c_dist_func, x, y](DP_* f, DP_* s) {
                         float dist1 = c_dist_func(x, y, f->x(), f->y());
                         float dist2 = c_dist_func(x, y, s->x(), s->y());
                         return dist1 < dist2;
                       });
      k_closest.resize(k);  // erasing elements after the k-th position
    }
  }

 public:
  kNN_2D(std::vector<DP_>& data, DISTANCE_FUNCTION_2D distance_function, Rect bounds = {})
      : space({}), data_ptr(data.data()), n_points(data.size()) {
    if (distance_function == DISTANCE_FUNCTION_2D::EUCLIDEAN) {
      dist_func = cxstructs::euclidean;
    } else {
      dist_func = cxstructs::manhattan;
    }
    if (bounds.width() == 0 && bounds.height() == 0) {
      float max_x = std::numeric_limits<float>::min();
      float max_y = std::numeric_limits<float>::min();
      float min_x = std::numeric_limits<float>::max();
      float min_y = std::numeric_limits<float>::max();
      for (auto& dp : data) {
        if (dp.x() > max_x) {
          max_x = dp.x();
        }
        if (dp.x() < min_x) {
          min_x = dp.x();
        }
        if (dp.y() > max_y) {
          max_y = dp.y();
        }
        if (dp.y() < min_y) {
          min_y = dp.y();
        }
      }
      space.set_bounds({min_x, min_y, max_x - min_x, max_y - min_y});
    }
    for (auto& dp : data) {
      space.insert(std::move(dp));
    }
  }
  /**
 * Classifies a point based on the absolute count of categories in the k closest points.
 * @param x The x-coordinate of the point.
 * @param y The y-coordinate of the point.
 * @param k The number of closest points to consider.
 * @return The category of the point.
 * @throws std::logic_error If there are not enough data points or no category is found.
 */
  inline Category classify_by_category_count(float x, float y, int k) {
    if (n_points < k) {
      throw std::logic_error("not enough data points");
    }
    vec<int> catg_values(128, 0);  // max categories
    vec<DP_*> k_closest;

    get_k_closest(x, y, k, k_closest);  //getting pointer list of closest points

    for (auto& ptr : k_closest) {
      catg_values[ptr->getCategory()]++;
    }

    int index = -1;
    index = catg_values.max_element();

    if (index == -1) {
      throw std::logic_error("no category found!");
    }

    return Category(index);
  }
  /**
 * Classifies a point based on the sum of distances to the k closest points in each category.
 * @param x The x-coordinate of the point.
 * @param y The y-coordinate of the point.
 * @param k The number of closest points to consider.
 * @return The category of the point.
 * @throws std::logic_error If no category is found.
 */
  inline Category classify_by_sum_distance(float x, float y, int k) {
    vec<float> catg_values(128, 0);  // max categories
    vec<DP_*> k_closest;

    get_k_closest(x, y, k, k_closest);  //getting pointer list of closest points

    for (auto& ptr : k_closest) {
      catg_values[ptr->getCategory()] += dist_func(x, y, ptr->x(), ptr->y());
    }

    int index = -1;
    index = catg_values.max_element();

    if (index == -1) {
      throw std::logic_error("no category found!");
    }

    return Category(index);
  }
  /**
 * Classifies a point based on the sum of weights of the k closest points in each category.
 * @param x The x-coordinate of the point.
 * @param y The y-coordinate of the point.
 * @param k The number of closest points to consider.
 * @return The category of the point.
 * @throws std::logic_error If no category is found.
 */
  inline Category classify_by_sum_weight(float x, float y, int k) {
    vec<float> catg_values(128, 0);  // max categories
    vec<DP_*> k_closest;

    get_k_closest(x, y, k, k_closest);  //getting pointer list of closest points

    for (auto& ptr : k_closest) {
      catg_values[ptr->getCategory()] += ptr->getWeight();
    }

    int index = -1;
    index = catg_values.max_element();

    if (index == -1) {
      throw std::logic_error("no category found!");
    }

    return Category(index);
  }
  /**
 * Classifies a point based on the sum of weighted distances to the k closest points in each category.
 * @param x The x-coordinate of the point.
 * @param y The y-coordinate of the point.
 * @param k The number of closest points to consider.
 * @return The category of the point.
 * @throws std::logic_error If no category is found.
 */
  inline Category classify_by_sum_weighted_distance(float x, float y, int k) {
    vec<float> catg_values(128, 0);  // max categories
    vec<DP_*> k_closest;

    get_k_closest(x, y, k, k_closest);  //getting pointer list of closest points

    for (auto& ptr : k_closest) {
      catg_values[ptr->getCategory()] += dist_func(x, y, ptr->x(), ptr->y()) * ptr->getWeight();
    }

    int index = -1;
    index = catg_values.max_element();

    if (index == -1) {
      throw std::logic_error("no category found!");
    }

    return Category(index);
  }
#  ifdef CX_INCLUDE_TESTS
  static void TEST() {
    enum Category { A, B, C };
    struct DataPoint : public DataPoint_<Category> {
      Category category;
      float x_;
      float y_;
      float weight = 1;
      DataPoint() {}
      DataPoint(float x, float y, Category category) : x_(x), y_(y), category(category) {}
      float x() const final { return x_; }
      float y() const final { return y_; }
      Category getCategory() final { return category; }
      float getWeight() const final { return weight; }
    };

    std::cout << "TESTING k-NN" << std::endl;

    std::vector<DataPoint> data{};
    data.emplace_back(1, 2, Category::A);
    data.emplace_back(2, 3, Category::A);
    data.emplace_back(3, 4, Category::A);
    data.emplace_back(4, 5, Category::B);
    data.emplace_back(5, 6, Category::B);
    data.emplace_back(6, 7, Category::B);
    data.emplace_back(7, 8, Category::C);
    data.emplace_back(8, 9, Category::C);
    data.emplace_back(9, 10, Category::C);
    Category cat1;
    kNN_2D<DataPoint> knn(data, DISTANCE_FUNCTION_2D::EUCLIDEAN);

    std::cout << "   Testing absolute classification" << std::endl;
    cat1 = knn.classify_by_category_count(0, 0, 4);
    CX_ASSERT((int)cat1 == 0, "");
    cat1 = knn.classify_by_category_count(5, 5, 4);
    CX_ASSERT((int)cat1 == 1, "");

    std::cout << "   Testing distance weighted classification" << std::endl;
    cat1 = knn.classify_by_sum_distance(0, 0, 4);
    CX_ASSERT((int)cat1 == 0, "");
    cat1 = knn.classify_by_sum_distance(5, 5, 4);
    CX_ASSERT((int)cat1 == 1, "");

    std::cout << "   Testing weight classification" << std::endl;
    cat1 = knn.classify_by_sum_weight(0, 0, 4);
    CX_ASSERT((int)cat1 == 0, "");
    cat1 = knn.classify_by_sum_weight(5, 5, 4);
    CX_ASSERT((int)cat1 == 1, "");

    std::cout << "   Testing weighted * dist classification" << std::endl;
    cat1 = knn.classify_by_sum_weighted_distance(0, 0, 4);
    CX_ASSERT((int)cat1 == 0, "");
    cat1 = knn.classify_by_sum_weighted_distance(5, 5, 4);
    CX_ASSERT((int)cat1 == 1, "");
  }
#  endif
};

class kNN_XD {
  //will implement a k-Tree for that
};

}  // namespace cxstructs
#endif  //CXSTRUCTS_SRC_MACHINELEARNING_K_NN_H_
