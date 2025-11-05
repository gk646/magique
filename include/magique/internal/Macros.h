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

#if defined(_MSC_VER)
#define M_FUNCTION __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__)
#define M_FUNCTION __PRETTY_FUNCTION__
#else
#define M_FUNCTION __func__
#endif


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
#define FUNCTION_CASE(eventType)                                                                                        \
    if constexpr (type == eventType)                                                                                    \
    {                                                                                                                   \
        script->eventType(std::forward<Args>(args)...);                                                                 \
    }

#define REGISTER_EVENTS(...)                                                                                            \
    template <EventType type, class Script, class... Args>                                                              \
    void Call(Script* script, Args... args)                                                                             \
    {                                                                                                                   \
        FOR_EACH(FUNCTION_CASE, __VA_ARGS__)                                                                            \
    }

//================= UTIL =================//

#if defined(NDEBUG)
#define MAGIQUE_ASSERT(expr, message) ((void)0)
#else
#define MAGIQUE_ASSERT(expr, message)                                                                                   \
    ((expr) ? (void)0 : magique::internal::AssertHandler(#expr, __FILE__, __LINE__, M_FUNCTION, message))
#endif

//================= ASSET LOADING =================//

#define ASSET_CHECK(asset)                                                                                              \
    if (!AssetBaseCheck(asset))                                                                                         \
        return {};

#define ASSET_IS_SUPPORTED_IMAGE_TYPE(asset)                                                                            \
    if (!IsSupportedImageFormat(asset.getExtension()))                                                                  \
    {                                                                                                                   \
        LOG_WARNING("Asset has unsupported extension: %s", asset.getExtension());                                       \
        return {};                                                                                                      \
    }

#define ASSET_SPRITE_SHEET_FITS_INSIDE_ATLAS(width)                                                                     \
    if (width > MAGIQUE_TEXTURE_ATLAS_SIZE)                                                                             \
    {                                                                                                                   \
        LOG_WARNING("SpriteSheet width would exceed texture atlas width! Skipping: %s", asset.getFileName(true));       \
        UnloadImage(image);                                                                                             \
        return {};                                                                                                      \
    }

#define ASSET_CHECK_IMAGE_DIVISIBILITY(image, checkW, checkH)                                                           \
    if ((image.width % checkW != 0) || (image.height % checkH != 0))                                                    \
    {                                                                                                                   \
        LOG_WARNING("Image dimensions (%d x %d) are not a multiple of the specified frame size (%d x %d). "             \
                    "Only full frames will be used.",                                                                   \
                    image.width, image.height, checkW, checkH);                                                         \
    }


//================= HELPERS =================//

#define M_SHARECODE_CHECKTYPE(checkType, err)                                                                           \
    if (p.type != checkType)                                                                                            \
    {                                                                                                                   \
        LOG_WARNING(err, p.name);                                                                                       \
        return;                                                                                                         \
    }

#define M_FRIEND(type) friend type;
#define befriend(...) FOR_EACH(M_FRIEND, __VA_ARGS__)

//================= BUILDING =================//

#if defined(MAGIQUE_TEST_MODE) || defined(MAGIQUE_IMPLEMENTATION)
#define M_MAKE_PUB() public:
#else
#define M_MAKE_PUB()
#endif

#ifdef _MSC_VER
#define M_IGNORE_WARNING(num) __pragma(warning(push)) __pragma(warning(disable : num))
#define M_UNIGNORE_WARNING() __pragma(warning(pop))
#else
#define M_IGNORE_WARNING(num)
#define M_UNIGNORE_WARNING()
#endif

#if defined(__GNUC__) || defined(__clang__)
#define STRINGIFY(x) #x
#define IGNORE_WARNING_GCC(warning) _Pragma("GCC diagnostic push") _Pragma(STRINGIFY(GCC diagnostic ignored warning))
#define UNIGNORE_WARNING_GCC() _Pragma("GCC diagnostic pop")
#else
#define IGNORE_WARNING_GCC(warning)
#define UNIGNORE_WARNING_GCC()
#endif


#endif //MAGIQUE_MACROS_H
