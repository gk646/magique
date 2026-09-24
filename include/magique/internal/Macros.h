// SPDX-License-Identifier: zlib-acknowledgement
#ifndef MAGIQUE_MACROS_H
#define MAGIQUE_MACROS_H

//===============================================
// Macros
//===============================================
// ................................................................................
// Don't look here
// ................................................................................

//================= ASSERTS =================//

namespace magique::internal
{
    void AssertHandler(const char* expr, const char* file, int line, const char* function, const char* message);
} // namespace magique::internal

#define localize(string) magique::Localize(string)

//================= SCRIPTING =================//

#define PARENS ()
#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__
#define FOR_EACH(macro, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) macro(a1) __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define MQ_EXPAND_LINE() __LINE__
#define MQ_PASTE(x, y) x##y
#define MQ_MAKE_UNIQUE_NAME(line) MQ_PASTE(MQ_BITFLAG_BASE_, line)
#define _MQ_ENUM_CASE(value) value = 1 << (__COUNTER__ - MQ_MAKE_UNIQUE_NAME(MQ_EXPAND_LINE()) - 1),

//================= UTIL =================//

#if defined(NDEBUG)
#define MAGIQUE_ASSERT(expr, message) ((void)0)
#else
#define MAGIQUE_ASSERT(expr, message)                                                                                   \
    ((expr) ? (void)0 : magique::internal::AssertHandler(#expr, __FILE__, __LINE__, M_FUNCTION, message))
#endif

#if defined(_MSC_VER)
#define M_FUNCTION __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__)
#define M_FUNCTION __FUNCTION__
#else
#define M_FUNCTION __func__
#endif

//================= ASSET LOADING =================//

#define ASSET_CHECK(asset)                                                                                              \
    if (!asset.isValid())                                                                                               \
        return {};

#define ASSET_IS_SUPPORTED_IMAGE_TYPE(asset)                                                                            \
    if (!IsSupportedImageFormat(asset.getExtension().data()))                                                           \
    {                                                                                                                   \
        LOG_WARNING("Asset has unsupported extension: %s", asset.getExtension());                                       \
        return {};                                                                                                      \
    }

//================= HELPERS =================//

#define M_ENABLE_STEAM_ERROR(ret)                                                                                       \
    LOG_ERROR("To enable steam use CMake: set(MAGIQUE_STEAM ON)");                                                      \
    return ret;

#define M_GAMESAVE_TYPE_MISMATCH(expected, ret)                                                                         \
    if (cell->type != StorageType::expected)                                                                            \
    {                                                                                                                   \
        LOG_ERROR("Storage slot type mismatch: Expected: %s Have %s", #expected, EnumToString(cell->type).data());      \
        return ret;                                                                                                     \
    }

#define M_GAMESAVE_SLOT_MISSING(ret)                                                                                    \
    if (cell == nullptr) [[unlikely]]                                                                                   \
    {                                                                                                                   \
        LOG_WARNING("No such storage slot: %s", slot.data());                                                           \
        return ret;                                                                                                     \
    }

//================= BUILDING =================//

#if defined(MAGIQUE_TEST_MODE) || defined(MAGIQUE_IMPLEMENTATION)
#define MQ_MAKE_PUB() public:
#else
#define MQ_MAKE_PUB()
#endif

#endif // MAGIQUE_MACROS_H
