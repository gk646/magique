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
#ifndef CXSTRUCTS_SRC_MACHINELEARNING_FNN_H_
#  define CXSTRUCTS_SRC_MACHINELEARNING_FNN_H_

#  include "../cxconfig.h"

#  ifndef CX_LOOP_FNN

#    pragma message("|FNN.h| Using Matrices for calculations. '#define CX_LOOP_FNN' to switch")

namespace cxhelper {
using namespace cxstructs;
struct Layer {
  mat weights_;
  mat bias_;
  mat w_sums_;
  mat inputs_;

  uint_16_cx in_;
  uint_16_cx out_;
  float learnR_;

  func a_func;
  func d_func;

 public:
  Layer()
      : weights_(), bias_(), w_sums_(), in_(0), out_(0), a_func(cxstructs::relu),
        d_func(cxstructs::d_relu), learnR_(0.5) {}
  Layer(uint_16_cx in, uint_16_cx out, func a_func, float learnR)
      : in_(in), out_(out), learnR_(learnR), inputs_(1, in), a_func(a_func) {
    if (a_func == cxstructs::relu) {
      d_func = cxstructs::d_relu;
    } else if (a_func == cxstructs::sig) {
      d_func = cxstructs::d_sig;
    } else {
      d_func = [](float x) {
        return static_cast<float>(1.0);
      };
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-0.3, 0.3);

    weights_ = mat(in, out, [&dis, &gen](int i) { return dis(gen); });
    bias_ = mat(1, out, [&dis, &gen](int i) { return dis(gen); });
    w_sums_ = mat(1, out, [&dis, &gen](int i) { return dis(gen); });
  }
  Layer& operator=(const Layer& other) {
    if (this != &other) {

      in_ = other.in_;
      out_ = other.out_;

      learnR_ = other.learnR_;
      inputs_ = other.inputs_;
      weights_ = other.weights_;
      bias_ = other.bias_;
      w_sums_ = other.w_sums_;

      a_func = other.a_func;
      d_func = other.d_func;
    }
    return *this;
  }
  ~Layer() = default;
  [[nodiscard]] mat forward(mat& in) {
    inputs_ = in;

    w_sums_ = in * weights_;  // matrix dimensions: (batch x in) * (in x out) = batch x out

    // Add the bias to each row of 'w_sums_'
    for (int i = 0; i < in.n_rows(); i++) {
      for (int j = 0; j < out_; j++) {
        w_sums_(i, j) += bias_(0, j);
      }
    }

    mat out = w_sums_;
    out.mat_op(a_func);
    return out;
  }
  [[nodiscard]] mat backward(mat& error) {
    w_sums_.mat_op(d_func);

    // Apply the derivative to each row of 'error'
    for (int i = 0; i < error.n_rows(); i++) {
      for (int j = 0; j < out_; j++) {
        error(i, j) *= w_sums_(i, j);
      }
    }

    //compute the gradient of the weights and bias
    mat d_weights =
        inputs_.transpose() * error;  //matrix dimensions: (in x batch) * (batch x out) = in x out
    mat d_bias = error.sum_cols();    //sum the error over the batch dimension
    d_bias.scale(1.0F / error.n_rows());

    // Update the weights and bias
    d_weights.scale(learnR_);
    d_bias.scale(learnR_);
    weights_ -= d_weights;
    bias_ -= d_bias;

    // Compute the error for the previous layer
    mat n_error = error * weights_.transpose();
    return n_error;
  }
};
}  // namespace cxhelper
namespace cxstructs {
using namespace cxhelper;
/**
 * <h2>Feedforward Neural Network</h2>
 *
 */
class FNN {
  Layer* layers_;
  std::vector<int> bounds_;
  uint_16_cx len_;
  float learnR_;
  func_M loss_function_;

 public:
  explicit FNN(
      const std::vector<int>& bound, func a_func, float learnR,
      func_M loss_function = mat::mean_sqr_abs_err,
      func last_layer_func = [](float x) { return x; })
      : learnR_(learnR), len_(bound.size() - 1), bounds_(bound), loss_function_(loss_function) {
    layers_ = new Layer[len_];
    for (int i = 1; i < len_ + 1; i++) {
      if (i == len_) {
        layers_[i - 1] = Layer(bounds_[i - 1], bounds_[i], last_layer_func, learnR_);
        break;
      }
      layers_[i - 1] = Layer(bounds_[i - 1], bounds_[i], a_func, learnR_);
    }
  }
  ~FNN() { delete[] layers_; }

  mat forward(const mat& in) {
    mat retval = in;
    for (int i = 0; i < len_; i++) {
      retval = layers_[i].forward(retval);
    }
    return retval;
  }
  void train(mat& in, mat& target, uint_16_cx n = 10, uint_16_cx batchSize = 10) {
    for (int k = 0; k < n; k++) {
      mat outputs = forward(in);  // dims: batch x last-layer-target

      outputs = loss_function_(outputs, target);  // target dims : batch x last-layer-target

      for (int i = len_ - 1; i > -1; i--) {
        outputs = layers_[i].backward(outputs);
      }
    }
  }
  vec<float> get_weights(int layer, int row) { return layers_[layer].weights_.get_row(row); }

#    ifdef CX_INCLUDE_TESTS
#      include "../cxutil/cxtime.h"
  static void TEST() {
    std::cout << "TESTING FNN" << std::endl;

    mat inputs = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
    std::vector<std::vector<float>> init_vec = {{1}, {0}, {1}, {0}};
    cxstructs::mat expected_outputs(init_vec);

    for (int i = 0; i < 5; ++i) {
      FNN fnn({2, 2, 1}, cxstructs::sig, 0.1);
      fnn.train(inputs, expected_outputs, 50000);
      mat output = fnn.forward(inputs);
      output -= expected_outputs;
      for (size_t j = 0; j < inputs.n_rows(); ++j) {
        CX_ASSERT(output(0, j) < 0.1, "");
      }
    }
  }
#    endif
};
}  // namespace cxstructs

#  else
#    pragma message("|FNN.h| Using FNN.h without matrices, calculations are loop based.")
namespace cxhelper {
struct Layer {
  float* weights_;
  float* bias_;
  float* w_sums_;
  std::vector<float> inputs_;
  uint_16_cx in_;
  uint_16_cx out_;
  func a_func;
  func d_func;
  float learnR_;

 public:
  Layer()
      : weights_(nullptr), bias_(nullptr), w_sums_(nullptr), in_(0), out_(0),
        a_func(cxstructs::relu), d_func(cxstructs::d_relu), learnR_(0.5) {}
  Layer(uint_16_cx in, uint_16_cx out, func a_func, float learnR)
      : in_(in), out_(out), a_func(a_func), learnR_(learnR), inputs_(in) {
    if (a_func == cxstructs::relu) {
      d_func = cxstructs::d_relu;
    } else if (a_func == cxstructs::sig) {
      d_func = cxstructs::d_sig;
    } else {
      d_func = [](float x) {
        return static_cast<float>(1.0);
      };
    }
    weights_ = new float[in_ * out_];
    bias_ = new float[out_];
    w_sums_ = new float[out_];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-0.3, 0.3);
    for (int i = 0; i < in_ * out_; ++i) {
      weights_[i] = dis(gen);
    }

    for (int i = 0; i < out_; ++i) {
      bias_[i] = dis(gen);
    }
  }
  Layer(const Layer& other)
      : in_(other.in_), out_(other.out_), a_func(other.a_func), learnR_(other.learnR_),
        inputs_(other.inputs_) {
    weights_ = new float[in_ * out_];
    bias_ = new float[out_];
    w_sums_ = new float[out_];
    std::copy(other.weights_, other.weights_ + in_ * out_, weights_);
    std::copy(other.bias_, other.bias_ + out_, bias_);
    std::copy(other.w_sums_, other.w_sums_ + out_, w_sums_);
    d_func = other.d_func;
  }

  // Assignment operator
  Layer& operator=(const Layer& other) {
    if (this != &other) {
      delete[] weights_;
      delete[] bias_;
      delete[] w_sums_;

      in_ = other.in_;
      out_ = other.out_;
      a_func = other.a_func;
      learnR_ = other.learnR_;
      inputs_ = other.inputs_;
      d_func = other.d_func;

      weights_ = new float[in_ * out_];
      bias_ = new float[out_];
      w_sums_ = new float[out_];
      std::copy(other.weights_, other.weights_ + in_ * out_, weights_);
      std::copy(other.bias_, other.bias_ + out_, bias_);
      std::copy(other.w_sums_, other.w_sums_ + out_, w_sums_);
    }
    return *this;
  }
  ~Layer() {
    delete[] weights_;
    delete[] bias_;
    delete[] w_sums_;
  }
  [[nodiscard]] std::vector<float> forward(std::vector<float>& in) {
    inputs_ = in;
    std::vector<float> out(out_, 0);
    float w_sum;
    for (int i = 0; i < out_; i++) {
      w_sum = 0;
      for (int j = 0; j < in_; j++) {
        w_sum += in[j] * weights_[j * out_ + i];
      }
      w_sums_[i] = w_sum + bias_[i];
      out[i] = a_func(w_sums_[i]);
    }
    return out;
  }
  [[nodiscard]] std::vector<float> backward(std::vector<float>& error) const {
    std::vector<float> n_error(in_, 0);
    for (int i = 0; i < out_; i++) {
      error[i] *= d_func(w_sums_[i]);
      for (int j = 0; j < in_; j++) {
        n_error[j] += weights_[j * out_ + i] * error[i];
        weights_[j * out_ + i] -= inputs_[j] * error[i] * learnR_;
      }
      bias_[i] -= learnR_ * error[i];
    }
    return n_error;
  }
};
}  // namespace cxhelper
cxstructs {
  using namespace cxhelper;
  /**
 * <h2>Feedforward Neural Network</h2>
 * Simple FNN implementation using mean average error and either sigmoid or ReLu
 * The last layer is always linear (better for simple problems and regression tasks)
 */
  class FNN {
    Layer* layers_;
    std::vector<uint_16_cx> bounds_;
    uint_16_cx len_;
    float learnR_;

   public:
    explicit FNN(const std::vector<uint_16_cx>& bound, func a_func, float learnR)
        : learnR_(learnR), len_(bound.size() - 1), bounds_(bound) {
      layers_ = new Layer[len_];
      for (int i = 1; i < len_ + 1; i++) {
        if (i == len_) {
          layers_[i - 1] = Layer(bounds_[i - 1], bounds_[i], [](float x) { return x; }, learnR_);
          break;
        }
        layers_[i - 1] = Layer(bounds_[i - 1], bounds_[i], a_func, learnR_);
      }
    }
    ~FNN() { delete[] layers_; }
    std::vector<float> forward(const std::vector<float>& in) {
      std::vector<float> retval = in;
      for (int i = 0; i < len_; i++) {
        retval = layers_[i].forward(retval);
      }
      return retval;
    }

    void train(const std::vector<float>& in, const std::vector<float>& out, uint_16_cx n = 10) {
      for (int k = 0; k < n; k++) {
        std::vector<float> retval = forward(in);

        for (int i = 0; i < retval.size(); i++) {
          retval[i] = (retval[i] - out[i]);
        }

        for (int i = len_ - 1; i > -1; i--) {
          retval = layers_[i].backward(retval);
        }
      }
    }
    void train(const std::vector<std::vector<float>>& in,
               const std::vector<std::vector<float>>& out, uint_16_cx n = 10) {

      for (int k = 0; k < n; k++) {

        for (int l = 0; l < in.size(); l++) {
          std::vector<float> retval = forward(in[l]);

          for (int i = 0; i < retval.size(); i++) {
            retval[i] = 2 * (retval[i] - out[l][i]);
          }

          for (int i = len_ - 1; i > -1; i--) {
            retval = layers_[i].backward(retval);
          }
        }
      }
    }

#    ifdef CX_INCLUDE_TESTS
#      include "../cxutil/cxtime.h"
    static void TEST() {
      std::cout << "TESING FNN" << std::endl;

      const std::vector<std::vector<float>> inputs = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
      const std::vector<std::vector<float>> expected_outputs = {{1}, {0}, {1}, {0}};

      for (int i = 0; i < 10; ++i) {
        FNN fnn({2, 2, 1}, cxstructs::sig, 0.16);
        cxstructs::now();
        fnn.train(inputs, expected_outputs, 50000);
        cxstructs::printTime();
        for (size_t j = 0; j < inputs.size(); ++j) {
          double output = fnn.forward(inputs[j])[0];
          double expected = expected_outputs[j][0];
          CX_ASSERT(output - expected < 0.1, "");
        }
      }
    }
#    endif
  };
}  // cxstructs

#  endif  // CX_LOOP_FNN
#  undef CX_MATRIX
#endif  //CXSTRUCTS_SRC_MACHINELEARNING_FNN_H_
