#ifndef ALLOCATOR_H
#define ALLOCATOR_H


#include <cstdlib>
#include <iostream>
#include <new>

// Override the global new operator
inline void *operator new(const size_t size)
{
    std::cout << "Allocating " << size << " bytes\n";
    void *ptr = malloc(size);
    if (ptr)
        return ptr;
    throw std::bad_alloc();
}

// Override the global delete operator
inline void operator delete(void *ptr) noexcept
{
    std::cout << "Deallocating\n";
    free(ptr);
}

// You might also want to override the array versions of new and delete
inline void *operator new[](size_t size)
{
    return ::operator new(size); // Redirect to single-object new
}

inline void operator delete[](void *ptr) noexcept
{
    ::operator delete(ptr); // Redirect to single-object delete
}

// Additional overloads might be needed for custom behavior and specific compilers
inline void operator delete(void *ptr, size_t size) noexcept
{
    std::cout << "Deallocating " << size << " bytes\n";
    free(ptr);
}

inline void operator delete[](void *ptr, size_t size) noexcept
{
    ::operator delete(ptr, size); // Redirect to single-object delete with size
}


#endif // ALLOCATOR_H