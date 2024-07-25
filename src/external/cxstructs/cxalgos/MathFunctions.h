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
#ifndef CXSTRUCTS_SRC_ALGORITHMS_MATHFUNCTIONS_H_
#  define CXSTRUCTS_SRC_ALGORITHMS_MATHFUNCTIONS_H_

#  include <cmath>
#  include <cstdint>
#  include <type_traits>
#  include "../cxconfig.h"
#  include "../cxutil/cxmath.h"

namespace cxstructs {

/**
 * @brief Approximates the definite integral of a function over a given interval.
 *
 * @tparam Function A callable object (e.g. function, lambda) that takes a double as an argument and returns a double.
 * @param fx The function to be integrated.
 * @param a The start of the interval over which to integrate.
 * @param b The end of the interval over which to integrate.
 * @param steps The number of steps to use in the approximation (optional, default is 10 mil).
 * @return The approximate value of the integral over the specified interval.
 */
template <typename Function,
          typename = std::enable_if_t<std::is_invocable_r_v<double, Function, double>>>
double integral_aprox(Function fx, double a, double b, uint_32_cx steps = 10000000) {
  double width = b - a;
  double step_size = width / steps;
  double integral = 0;
  for (uint_32_cx i = 0; i < steps; i++) {
    integral += fx(a) * step_size;
    a += step_size;
  }
  return integral;
}
/**
 * @brief Calculates the volume of revolution of a function around the x-axis using numerical integration.
 *
 * @tparam Function A callable object or function that takes a single double as an argument and returns a double.
 *                  The function represents the curve to be revolved around the x-axis.
 *
 * @param fx The function representing the curve. It should take a single double argument representing the x-coordinate
 *           and return the corresponding y-coordinate.
 * @param a The starting x-coordinate of the interval over which the volume is to be calculated.
 * @param b The ending x-coordinate of the interval over which the volume is to be calculated.
 * @param steps The number of steps to use in the numerical integration. A higher value increases accuracy but also
 *              increases computation time. Default is 10 mil.
 *
 * @return The volume of revolution of the function around the x-axis over the specified interval.
 *
 * @note This function uses a numerical integration technique to approximate the volume. The accuracy of the result
 *       depends on the number of steps used.
 */
template <typename Function,
          typename = std::enable_if_t<std::is_invocable_r_v<double, Function, double>>>
double integral_volume_solids_of_revolution(Function fx, double a, double b,
                                            uint_32_cx steps = 10000000) {
  double volume = 0;
  double width = b - a;
  double step_size = width / steps;
  for (uint_32_cx i = 0; i < steps; i++) {
    volume += CX_PI * std::pow(fx(a), 2) * step_size;
    a += step_size;
  }
  return volume;
}
template <typename Function,
          typename = std::enable_if_t<std::is_invocable_r_v<double, Function, double>>>
double integral_arc_length(Function fx, double a, double b, uint_32_cx steps = 10000000) {
  double arc_length = 0;
  double step_size = (b - a) / steps;
  double step_squared = step_size * step_size;
  for (uint_32_cx i = 0; i < steps; i++) {
    arc_length += std::sqrt(step_squared + std::pow(fx(a + step_size) - fx(a), 2));
    a += step_size;
  }
  return arc_length;
}
}  // namespace cxstructs
#  ifdef CX_INCLUDE_TESTS
namespace cxtests {
using namespace cxstructs;
static void TEST_MATH() {
  std::cout << "TESTING MATH FUNCTIONS" << std::endl;
  auto integral = integral_aprox([](double x) { return x * x; }, 0, 5, 100000);
  CX_ASSERT(integral - std::pow(5, 3) / 3 < 0.0001, "");
  auto volume = integral_volume_solids_of_revolution([](double x) { return std::sqrt(x); }, 0, 4);

  CX_ASSERT(volume - CX_PI * 8 < 0.001, "");
  auto length =
      integral_arc_length([](double x) { return (1.0 / 3.0) * std::pow((x * x + 2), 3.0 / 2.0); },
                          0, std::sqrt(8), 100000);

  CX_ASSERT(cxstructs::next_power_of_2(3) == 4, "");
  CX_ASSERT(cxstructs::next_power_of_2(3) != 3, "");
  CX_ASSERT(cxstructs::next_power_of_2(10) == 16, "");
  CX_ASSERT(cxstructs::next_power_of_2(53) == 64, "");
}
}  // namespace cxtests
#  endif
#endif  //CXSTRUCTS_SRC_ALGORITHMS_MATHFUNCTIONS_H_
