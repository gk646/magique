#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstdio>
#include <cstdlib>
#include <magique/util/Logging.h>


// Will get a custom allocator interface similar to this gdc talk
// https://www.youtube.com/watch?v=fcBZEZWGYek&pp=ygUcc2ltcGxlIGNvbGxvc3N1c2UgcmVtYWtlIGdkYw%3D%3D

// Override the global new operator
void* operator new(const size_t size)
{
    LOG_ALLOC("Allocating %d bytes", static_cast<int>(size));
    void* ptr = malloc(size);
    if(size > 100000)
    {
        volatile int b = 5;
    }

    if (ptr)
        return ptr;

    return nullptr;
}

// Override the global delete operator
 void operator delete(void* ptr) noexcept
{
    LOG_ALLOC("Deallocating");

    free(ptr);
}

// You might also want to override the array versions of new and delete
 void* operator new[](size_t size)
{
    return ::operator new(size); // Redirect to single-object new
}

 void operator delete[](void* ptr) noexcept
{
    ::operator delete(ptr); // Redirect to single-object delete
}

// Additional overloads might be needed for custom behavior and specific compilers
 void operator delete(void* ptr, size_t size) noexcept
{
    LOG_ALLOC("Deallocating %d bytes", static_cast<int>(size));
    free(ptr);
}

 void operator delete[](void* ptr, size_t size) noexcept
{
    ::operator delete(ptr, size); // Redirect to single-object delete with size
}


#endif // ALLOCATOR_H