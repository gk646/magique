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
#define FUNCTION_CASE(eventType)                                                                                        \
    if constexpr (type == eventType)                                                                                    \
    {                                                                                                                   \
        script->eventType(std::forward<Args>(args)...);                                                                 \
    }

#define MQ_REGISTER_SCRIPT_EVENTS(...)                                                                                  \
    template <ScriptEvent type, class Script, class... Args>                                                            \
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

#define M_ENABLE_STEAM_ERROR(ret)                                                                                       \
    LOG_ERROR("To enable steam use CMake: set(MAGIQUE_STEAM ON)");                                                      \
    return ret;

#define M_GAMESAVE_TYPE_MISMATCH(expected, ret)                                                                         \
    if (cell->type != StorageType::expected)                                                                            \
    {                                                                                                                   \
        LOG_ERROR("Storage slot type mismatch: Expected: %s Have (enum id): %d", #expected, slot);                      \
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
#define M_MAKE_PUB() public:
#else
#define M_MAKE_PUB()
#endif

#if defined(__GNUC__) || defined(__clang__)
#define STRINGIFY(x) #x
#define IGNORE_WARNING_GCC(warning) _Pragma("GCC diagnostic push") _Pragma(STRINGIFY(GCC diagnostic ignored warning))
#define UNIGNORE_WARNING_GCC() _Pragma("GCC diagnostic pop")
#else
#define IGNORE_WARNING_GCC(warning)
#define UNIGNORE_WARNING_GCC()
#endif


#endif // MAGIQUE_MACROS_H
