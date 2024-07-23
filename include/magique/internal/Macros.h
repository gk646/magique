#ifndef MAGIQUE_MACROS_H
#define MAGIQUE_MACROS_H

#include <cstdio>

//-----------------------------------------------
// Macros
//-----------------------------------------------
// ................................................................................
// Dont look here
// ................................................................................

#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))

#define FOR_EACH_HELPER(macro, a1, ...) macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))

#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define FUNCTION_CASE(eventType)                                                                                       \
    if constexpr (type == eventType)                                                                                   \
    {                                                                                                                  \
        script->eventType(std::forward<Args>(args)...);                                                                \
    }


#define REGISTER_EVENTS(...)                                                                                           \
    template <EventType type, class Script, class... Args>                                                             \
    void Call(Script* script, Args... args)                                                                            \
    {                                                                                                                  \
        FOR_EACH(FUNCTION_CASE, __VA_ARGS__)                                                                           \
    }

namespace magique::internal
{
    inline void AssertHandler(const char* expr, const char* file, int line, const char* message)
    {
        fprintf(stderr, "Assert failed: %s\nAt: %s:%d\nMessage: %s", expr, file, line, message);
#if defined(_MSC_VER)
        __debugbreak();
#elif defined(__GNUC__)
        __builtin_trap();
#else
        std::abort();
#endif
    }
} // namespace magique::util


#if !defined(_DEBUG) || defined(NDEBUG)
#define M_ASSERT(expr, message) ((void)0)
#else
#define M_ASSERT(expr, message) ((expr) ? (void)0 : magique::internal::AssertHandler(#expr, __FILE__, __LINE__, message))
#endif

#endif //MAGIQUE_MACROS_H