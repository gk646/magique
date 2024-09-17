#ifndef MAGIQUE_MATRIX_H
#define MAGIQUE_MATRIX_H

#include <cassert>

struct Matrix final
{
    float* arr = nullptr;
    int rows = 0;
    int cols = 0;

    // Base
    Matrix() = default;
    Matrix(const int rows, const int cols) : rows(rows), cols(cols)
    {
        arr = static_cast<float*>(malloc(sizeof(float) * rows * cols));
        assert(arr && "Memory allocation failed");
        memset(arr, 0, sizeof(float) * rows * cols);
    }

    // Special
    Matrix(const Matrix& o) : rows(o.rows), cols(o.cols)
    {
        arr = static_cast<float*>(malloc(sizeof(float) * rows * cols));
        assert(arr && "Memory allocation failed");
        std::memcpy(arr, o.arr, sizeof(float) * rows * cols);
    }
    Matrix& operator=(Matrix&& other) noexcept
    {
        if (this != &other)
        {
            free(arr);

            arr = other.arr;
            rows = other.rows;
            cols = other.cols;

            other.arr = nullptr;
            other.rows = 0;
            other.cols = 0;
        }
        return *this;
    }
    Matrix(Matrix&& other) noexcept : arr(other.arr), rows(other.rows), cols(other.cols)
    {
        other.arr = nullptr;
        other.rows = 0;
        other.cols = 0;
    }
    ~Matrix()
    {
        free(arr);
        arr = nullptr;
    }

    float& operator()(const int row, const int col)
    {
        assert(row < rows && col < cols && "Index out of bounds");
        return arr[row * cols + col];
    }
    float operator()(const int row, const int col) const
    {
        assert(row < rows && col < cols && "Index out of bounds");
        return arr[row * cols + col];
    }

    // Operators
    Matrix& operator=(const Matrix& other)
    {
        if (this != &other)
        {
            free(arr);

            rows = other.rows;
            cols = other.cols;

            arr = static_cast<float*>(malloc(sizeof(float) * rows * cols));
            assert(arr && "Memory allocation failed");
            std::memcpy(arr, other.arr, sizeof(float) * rows * cols);
        }
        return *this;
    }
    Matrix operator*(const Matrix& o) const
    {
        assert(cols == o.rows && "Matrix dimensions must align for multiplication");
        Matrix result(rows, o.cols);
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < o.cols; ++j)
            {
                float sum = 0.0f;
                for (int k = 0; k < cols; ++k)
                {
                    sum += arr[i * cols + k] * o.arr[k * o.cols + j];
                }
                result.arr[i * o.cols + j] = sum;
            }
        }
        return result;
    }
    Matrix& operator*=(const Matrix& o)
    {
        assert(cols == o.rows && "Matrix dimensions must align for multiplication");
        const Matrix result = *this * o;
        *this = result;
        return *this;
    }

    // Access to raw data
    [[nodiscard]] float* getData() const { return arr; }
};

#endif //MAGIQUE_MATRIX_H