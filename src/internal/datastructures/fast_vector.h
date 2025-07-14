/*The MIT License (MIT)

Copyright (c) 2019 Vladislav Lukyanov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

// This is a fixed and expanded version of the original header
// Added iterator erase, erase, insert...
// Main reasons: less compile time, actually slightly faster, less includes, less templates

#pragma once
#ifndef FAST_VECTOR_H
#define FAST_VECTOR_H

#if defined(__MINGW32__) || defined(__GNUG__)
#include <utility>
#endif

#include <type_traits>
#include <cassert>
#include <cstdlib>
#include <cstring>

template <class T>
struct fast_vector
{
    using size_type = int;

    fast_vector() = default;
    fast_vector(const fast_vector& other) noexcept;
    explicit fast_vector(size_type reserve) noexcept;
    fast_vector(fast_vector&& other) noexcept;
    fast_vector& operator=(const fast_vector& other) noexcept;
    fast_vector& operator=(fast_vector&& other) noexcept;
    ~fast_vector() noexcept;

    // Element access

    T& operator[](size_type pos);
    const T& operator[](size_type pos) const;

    T& front();
    const T& front() const;

    T& back();
    const T& back() const;

    T* data() noexcept;
    const T* data() const noexcept;

    // Iterators

    T* begin() noexcept;
    const T* begin() const noexcept;

    T* end() noexcept;
    const T* end() const noexcept;

    // Capacity

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] size_type size() const noexcept;
    void reserve(size_type new_cap) noexcept;
    [[nodiscard]] size_type capacity() const noexcept;
    void shrink_to_fit();

    // Modifiers

    // Sets a new size - only use if you manually copied into the vector (and reserver before)
    void set_size(size_type new_size);
    void clear() noexcept;

    void push_back(const T& value) noexcept;
    void push_back(T&& value) noexcept;

    template <class... Args>
    void emplace_back(Args&&... args) noexcept;

    void insert(const T* pos, T&& object);
    void insert(const T* pos, const T& object);

    // Keeps order
    bool erase(const T& val);
    T* erase(T* pos);
    T* erase(T* start, T* end);

    void pop_back() noexcept;

    void resize(size_type count) noexcept;
    void resize(size_type count, const T& val) noexcept;

    bool contains(const T& val) const noexcept;

    static constexpr size_type grow_factor = 2;

private:
    T* m_data = nullptr;
    size_type m_size = 0;
    size_type m_capacity = 0;
};

template <class T>
fast_vector<T>::fast_vector(const size_type res) noexcept
{
    reserve(res);
}

template <class T>
fast_vector<T>::fast_vector(const fast_vector& other) noexcept : m_size(other.m_size), m_capacity(other.m_capacity)
{
    m_data = static_cast<T*>(malloc(sizeof(T) * other.m_capacity));

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memcpy(m_data, other.m_data, m_size * sizeof(T));
    }
    else
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            new (m_data + i) T(other.m_data[i]);
        }
    }
}

template <class T>
fast_vector<T>::fast_vector(fast_vector&& other) noexcept :
    m_data(other.m_data), m_size(other.m_size), m_capacity(other.m_capacity)
{
    other.m_data = nullptr;
    other.m_size = 0;
    other.m_capacity = 0;
}

template <class T>
fast_vector<T>& fast_vector<T>::operator=(const fast_vector& other) noexcept
{
    if (this != &other)
    {
        T* new_data = static_cast<T*>(malloc(sizeof(T) * other.m_capacity));

        if constexpr (std::is_trivially_copyable_v<T>)
        {
            mem(new_data, other.m_data, other.m_size * sizeof(T));
        }
        else
        {
            for (size_type i = 0; i < other.m_size; ++i)
            {
                new (new_data + i) T(other.m_data[i]);
            }
        }

        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_type i = 0; i < m_size; ++i)
            {
                m_data[i].~T();
            }
        }

        free(m_data);

        m_data = new_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
    }

    return *this;
}

template <class T>
fast_vector<T>& fast_vector<T>::operator=(fast_vector&& other) noexcept
{
    if (this != &other)
    {
        m_data = other.m_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;

        other.m_data = nullptr;
    }
    return *this;
}

template <class T>
fast_vector<T>::~fast_vector() noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            m_data[i].~T();
        }
    }

    free(m_data);
}

// Element access

template <class T>
T& fast_vector<T>::operator[](size_type pos)
{
    assert(pos < m_size && pos >= 0 && "Position is out of range");
    return m_data[pos];
}

template <class T>
const T& fast_vector<T>::operator[](size_type pos) const
{
    assert(pos < m_size && pos >= 0 && "Position is out of range");
    return m_data[pos];
}

template <class T>
T& fast_vector<T>::front()
{
    assert(m_size > 0 && "Container is empty");
    return m_data[0];
}

template <class T>
const T& fast_vector<T>::front() const
{
    assert(m_size > 0 && "Container is empty");
    return m_data[0];
}

template <class T>
T& fast_vector<T>::back()
{
    assert(m_size > 0 && "Container is empty");
    return m_data[m_size - 1];
}

template <class T>
const T& fast_vector<T>::back() const
{
    assert(m_size > 0 && "Container is empty");
    return m_data[m_size - 1];
}

template <class T>
T* fast_vector<T>::data() noexcept
{
    return m_data;
}

template <class T>
const T* fast_vector<T>::data() const noexcept
{
    return m_data;
}

// Iterators

template <class T>
T* fast_vector<T>::begin() noexcept
{
    return m_data;
}

template <class T>
const T* fast_vector<T>::begin() const noexcept
{
    return m_data;
}

template <class T>
T* fast_vector<T>::end() noexcept
{
    return m_data + m_size;
}

template <class T>
const T* fast_vector<T>::end() const noexcept
{
    return m_data + m_size;
}

// Capacity

template <class T>
bool fast_vector<T>::empty() const noexcept
{
    return m_size == 0;
}

template <class T>
typename fast_vector<T>::size_type fast_vector<T>::size() const noexcept
{
    return m_size;
}

template <class T>
void fast_vector<T>::reserve(const size_type new_cap) noexcept
{
    if (new_cap > m_capacity)
    {
        T* new_data = static_cast<T*>(malloc(new_cap * sizeof(T)));

        if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_destructible_v<T>)
        {
            memcpy(new_data, m_data, m_size * sizeof(T));
        }
        else
        {
            for (size_type i = 0; i < m_size; ++i)
            {
                new (new_data + i) T(std::move(m_data[i]));
                m_data[i].~T();
            }
        }

        free(m_data);
        m_data = new_data;
        m_capacity = new_cap;
    }
}

template <class T>
typename fast_vector<T>::size_type fast_vector<T>::capacity() const noexcept
{
    return m_capacity;
}

template <class T>
void fast_vector<T>::shrink_to_fit()
{
    if (m_size < m_capacity)
    {
        if constexpr (std::is_trivial_v<T>)
        {
            m_data = static_cast<T*>(realloc(m_data, sizeof(T) * m_size));
            assert(m_data != nullptr && "Reallocation failed");
        }
        else
        {
            T* new_data_location = static_cast<T*>(malloc(sizeof(T) * m_size));
            assert(new_data_location != nullptr && "Allocation failed");

            copy_range(begin(), end(), new_data_location);
            destruct_range(begin(), end());

            free(m_data);

            m_data = new_data_location;
        }
    }
}

template <class T>
void fast_vector<T>::set_size(size_type new_size)
{
    m_size = new_size;
}

// Modifiers

template <class T>
void fast_vector<T>::clear() noexcept
{
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            m_data[i].~T();
        }
    }
    m_size = 0;
}

template <class T>
void fast_vector<T>::push_back(const T& value) noexcept
{
    if (m_size == m_capacity)
    {
        reserve(m_capacity * grow_factor + 1);
    }

    if constexpr (std::is_trivial_v<T>)
    {
        m_data[m_size] = value;
    }
    else
    {
        new (m_data + m_size) T(value);
    }

    m_size++;
}

template <class T>
void fast_vector<T>::push_back(T&& value) noexcept
{
    if (m_size == m_capacity)
    {
        reserve((m_capacity * grow_factor) + 1);
    }

    if constexpr (std::is_trivial_v<T>)
    {
        m_data[m_size] = value;
    }
    else
    {
        new (m_data + m_size) T(std::move(value));
    }

    m_size++;
}

template <class T>
template <class... Args>
void fast_vector<T>::emplace_back(Args&&... args) noexcept
{
    //    static_assert(!std::is_trivial_v<T>, "Use push_back() instead of emplace_back() with trivial types");

    if (m_size == m_capacity)
    {
        reserve(m_capacity * grow_factor + 1);
    }

    new (m_data + m_size) T{std::forward<Args>(args)...};

    m_size++;
}

template <class T>
void fast_vector<T>::insert(const T* pos, T&& object)
{
    assert(pos >= m_data && pos <= m_data + m_size && "Iterator out of bounds");

    size_type index = pos - m_data;

    if (m_size == m_capacity)
    {
        reserve(m_capacity == 0 ? 5 : 2 * m_capacity);
    }

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memmove(m_data + index + 1, m_data + index, (m_size - index) * sizeof(T));
        m_data[index] = std::move(object);
    }
    else
    {
        for (size_type i = m_size; i > index; --i)
        {
            m_data[i] = std::move(m_data[i - 1]);
        }
        m_data[index] = std::move(object);
    }

    ++m_size;
}

template <class T>
void fast_vector<T>::insert(const T* pos, const T& object)
{
    assert(pos >= m_data && pos <= m_data + m_size && "Iterator out of bounds");

    const auto index = pos - m_data;

    if (m_size == m_capacity)
    {
        reserve(m_capacity == 0 ? 5 : 2 * m_capacity);
    }

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memmove(m_data + index + 1, m_data + index, (m_size - index) * sizeof(T));
        m_data[index] = object;
    }
    else
    {
        for (size_type i = m_size; i > index; --i)
        {
            m_data[i] = std::move(m_data[i - 1]);
        }
        m_data[index] = object;
    }

    ++m_size;
}

template <class T>
bool fast_vector<T>::erase(const T& val)
{
    const size_type size = m_size;
    for (size_type i = 0; i < size; ++i)
    {
        if (m_data[i] == val)
        {
            --m_size;
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                memmove(m_data + i, m_data + i + 1, (m_size - i) * sizeof(T));
            }
            else
            {
                for (size_type j = i; j < m_size; ++j)
                {
                    m_data[j] = std::move(m_data[j + 1]);
                }
                m_data[m_size].~T();
            }
            return true;
        }
    }
    return false;
}

template <class T>
T* fast_vector<T>::erase(T* pos)
{
    assert(pos >= m_data && pos < m_data + m_size && "Iterator out of bounds");

    const auto index = pos - m_data; // Calculate index of the element to be erased

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memmove(pos, pos + 1, (m_data + m_size - pos - 1) * sizeof(T));
    }
    else
    {
        for (auto i = index; i < m_size - 1; ++i)
        {
            m_data[i] = std::move(m_data[i + 1]);
        }
        m_data[m_size - 1].~T();
    }

    --m_size;

    return m_data + index;
}

template <class T>
auto fast_vector<T>::erase(T* start, T* end) -> T*
{
    assert(start >= m_data && start < m_data + m_size && "Start iterator out of bounds");
    assert(end >= m_data && end <= m_data + m_size && "End iterator out of bounds");
    assert(start <= end && "Invalid range");

    size_type start_index = start - m_data;                           // Calculate start index
    const size_type end_index = end - m_data;                         // Calculate end index
    const size_type num_elements_to_remove = end_index - start_index; // Number of elements to remove

    if (num_elements_to_remove == 0)
        return start;

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        memmove(start, end, (m_data + m_size - end) * sizeof(T));
    }
    else
    {
        for (size_type i = start_index; i < m_size - num_elements_to_remove; ++i)
        {
            m_data[i] = std::move(m_data[i + num_elements_to_remove]);
        }
        for (size_type i = 0; i < num_elements_to_remove; ++i)
        {
            m_data[m_size - 1 - i].~T();
        }
    }

    m_size -= num_elements_to_remove;

    return m_data + start_index;
}

template <class T>
void fast_vector<T>::pop_back() noexcept
{
    assert(m_size > 0 && "Container is empty");

    if constexpr (!std::is_trivial_v<T>)
    {
        m_data[m_size - 1].~T();
    }

    m_size--;
}

template <class T>
void fast_vector<T>::resize(const size_type count) noexcept
{
    if (count > m_capacity)
    {
        reserve(count);
    }

    if constexpr (!std::is_trivially_constructible_v<T>)
    {
        if (count > m_size)
        {
            for (size_type i = m_size; i < count; ++i)
            {
                new (m_data + i) T();
            }
        }
        else
        {
            for (size_type i = count; i < m_size; ++i)
            {
                m_data[i].~T();
            }
        }
    }
    m_size = count;
}

template <class T>
void fast_vector<T>::resize(const size_type count, const T& val) noexcept
{
    if (count > m_capacity)
    {
        reserve(count);
    }

    if (count > m_size) [[likely]]
    {
        if constexpr (std::is_trivially_constructible_v<T>)
        {
            for (size_type i = m_size; i < count; ++i)
            {
                m_data[i] = val;
            }
        }
        else
        {
            for (size_type i = m_size; i < count; ++i)
            {
                new (m_data + i) T(val);
            }
        }
    }
    else
    {
        for (size_type i = count; i < m_size; ++i)
        {
            m_data[i].~T();
        }
    }

    m_size = count;
}

template <class T>
bool fast_vector<T>::contains(const T& val) const noexcept
{
    for (size_type i = 0; i < m_size; ++i)
    {
        if (m_data[i] == val)
        {
            return true;
        }
    }
    return false;
}
#endif //FAST_VECTOR_H