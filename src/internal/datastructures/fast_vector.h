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

#pragma once
#ifndef FAST_VECTOR_H
#define FAST_VECTOR_H

#include <cassert>
#include <cstring> // std::memcpy()

template <class T>
struct fast_vector
{
    using size_type = int32_t;

    fast_vector() = default;
    fast_vector(const fast_vector& other);
    explicit fast_vector(size_type reserve);
    fast_vector(fast_vector&& other) noexcept;
    fast_vector& operator=(const fast_vector& other);
    fast_vector& operator=(fast_vector&& other) noexcept;
    ~fast_vector();

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

    bool empty() const noexcept;
    size_type size() const noexcept;
    void reserve(size_type new_cap);
    size_type capacity() const noexcept;
    void shrink_to_fit();

    // Modifiers

    void clear() noexcept;

    void push_back(const T& value);
    void push_back(T&& value);

    template <class... Args>
    void emplace_back(Args&&... args);


    void insert(T* pos, T&& object);
    void insert(T* pos, const T& object);

    // Keeps order
    void erase(const T& val);
    T* erase(T* pos);
    T* erase(T* start, T* end);

    // Doesnt keep order
    void erase_unordered(const T& val);

    void pop_back();

    void resize(size_type count);

    static constexpr size_type grow_factor = 3;

private:
    T* m_data = nullptr;
    size_type m_size = 0;
    size_type m_capacity = 0;
};

template <class T>
fast_vector<T>::fast_vector(size_type res)
{
    reserve(res);
}

template <class T>
fast_vector<T>::fast_vector(const fast_vector& other) : m_size(other.m_size), m_capacity(other.m_capacity)
{
    m_data = static_cast<T*>(std::malloc(sizeof(T) * other.m_capacity));

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        std::memcpy(m_data, other.m_data, m_size * sizeof(T));
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
}

template <class T>
auto fast_vector<T>::operator=(const fast_vector& other) -> fast_vector<T>&
{
    if (this != &other)
    {
        T* new_data = static_cast<T*>(std::malloc(sizeof(T) * other.m_capacity));

        if constexpr (std::is_trivially_copyable_v<T>)
        {
            std::memcpy(new_data, other.m_data, other.m_size * sizeof(T));
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

        std::free(m_data);

        m_data = new_data;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
    }

    return *this;
}

template <class T>
auto fast_vector<T>::operator=(fast_vector&& other) noexcept -> fast_vector<T>&
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
fast_vector<T>::~fast_vector()
{
    if constexpr (!std::is_trivially_destructible_v<T>)
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            m_data[i].~T();
        }
    }

    std::free(m_data);
}

// Element access

template <class T>
T& fast_vector<T>::operator[](size_type pos)
{
    assert(pos < m_size && "Position is out of range");
    return m_data[pos];
}

template <class T>
const T& fast_vector<T>::operator[](size_type pos) const
{
    assert(pos < m_size && "Position is out of range");
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
void fast_vector<T>::reserve(size_type new_cap)
{
    if (new_cap > m_capacity)
    {
        T* new_data = static_cast<T*>(malloc(new_cap * sizeof(T)));

        if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_destructible_v<T>)
        {
            std::memcpy(new_data, m_data, m_size * sizeof(T));
        }
        else
        {
            for (size_type i = 0; i < m_size; ++i)
            {
                new (new_data + i) T(move(m_data[i]));
                m_data[i].~T();
            }
        }

        std::free(m_data);
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
            m_data = reinterpret_cast<T*>(std::realloc(m_data, sizeof(T) * m_size));
            assert(m_data != nullptr && "Reallocation failed");
        }
        else
        {
            T* new_data_location = reinterpret_cast<T*>(std::malloc(sizeof(T) * m_size));
            assert(new_data_location != nullptr && "Allocation failed");

            copy_range(begin(), end(), new_data_location);
            destruct_range(begin(), end());

            std::free(m_data);

            m_data = new_data_location;
        }
    }
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
void fast_vector<T>::push_back(const T& value)
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
void fast_vector<T>::push_back(T&& value)
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
        new (m_data + m_size) T(std::move(value));
    }

    m_size++;
}

template <class T>
template <class... Args>
void fast_vector<T>::emplace_back(Args&&... args)
{
    static_assert(!std::is_trivial_v<T>, "Use push_back() instead of emplace_back() with trivial types");

    if (m_size == m_capacity)
    {
        reserve(m_capacity * grow_factor + 1);
    }

    new (m_data + m_size) T(std::forward<Args>(args)...);

    m_size++;
}

template <class T>
void fast_vector<T>::insert(T* pos, T&& object)
{
    assert(pos >= m_data && pos <= m_data + m_size && "Iterator out of bounds");

    size_type index = pos - m_data;

    if (m_size == m_capacity)
    {
        reserve(m_capacity == 0 ? 5 : 2 * m_capacity);
    }

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        std::memmove(m_data + index + 1, m_data + index, (m_size - index) * sizeof(T));
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
void fast_vector<T>::insert(T* pos, const T& object)
{
    assert(pos >= m_data && pos <= m_data + m_size && "Iterator out of bounds");

    size_type index = pos - m_data;

    if (m_size == m_capacity)
    {
        reserve(m_capacity == 0 ? 5 : 2 * m_capacity);
    }

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        std::memmove(m_data + index + 1, m_data + index, (m_size - index) * sizeof(T));
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
void fast_vector<T>::erase(const T& val)
{
    const size_type size = m_size;
    for (size_type i = 0; i < size; ++i)
    {
        if (m_data[i] == val)
        {
            --m_size;
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                std::memmove(m_data + i, m_data + i + 1, (m_size - i) * sizeof(T));
            }
            else
            {
                for (size_type j = i; j < m_size; ++j)
                {
                    m_data[j] = std::move(m_data[j + 1]);
                }
                m_data[m_size].~T();
            }
            return;
        }
    }
}

template <class T>
T* fast_vector<T>::erase(T* pos)
{
    assert(pos >= m_data && pos < m_data + m_size && "Iterator out of bounds");

    size_type index = pos - m_data; // Calculate index of the element to be erased

    if constexpr (std::is_trivially_copyable_v<T>)
    {
        std::memmove(pos, pos + 1, (m_data + m_size - pos - 1) * sizeof(T));
    }
    else
    {
        for (size_type i = index; i < m_size - 1; ++i)
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
        std::memmove(start, end, (m_data + m_size - end) * sizeof(T));
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
void fast_vector<T>::erase_unordered(const T& val)
{
    const size_type size = m_size;
    for (size_type i = 0; i < size; ++i)
    {
        if (m_data[i] == val)
        {
            --m_size;
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                m_data[i] = m_data[m_size];
            }
            else
            {
                m_data[i] = std::move(m_data[m_size]);
                m_data[m_size].~T();
            }
            return;
        }
    }
}

template <class T>
void fast_vector<T>::pop_back()
{
    assert(m_size > 0 && "Container is empty");

    if constexpr (!std::is_trivial_v<T>)
    {
        m_data[m_size - 1].~T();
    }

    m_size--;
}

template <class T>
void fast_vector<T>::resize(size_type count)
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
#endif //FAST_VECTOR_H