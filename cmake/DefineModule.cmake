# This script expects MODULE_NAME to be set
if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not defined")
    message("Use set(MODULE_NAME \"MyModule\") to set the name")
endif ()

# Parse files
file(GLOB_RECURSE MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Add the module as static library
add_library(magique-${MODULE_NAME} OBJECT ${MODULE_FILES})

target_include_directories(magique-${MODULE_NAME} PRIVATE
        ${MAGIQUE_PUBLIC_INCLUDE} # Public includes
        ${MAGIQUE_PRIVATE_INCLUDE} # Private includes
        ${MAGIQUE_CXSTRUCTS_INCLUDE} # cxstructs includes
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(magique-${MODULE_NAME} PRIVATE
            -std=c++20 -march=native -ffast-math -fno-exceptions -fno-rtti -fvisibility=hidden
            -Wall
            -Wc++20-compat
            -Wc++20-extensions
            -Wcomma-subscript
            -Wdeprecated-declarations
            -Wextra-semi
            -Wnon-virtual-dtor
            -Wuseless-cast
            -Wvla
            -Wno-virtual-dtor
            -Wno-useless-cast
    )

    target_compile_options(magique-${MODULE_NAME} PRIVATE
            $<$<CONFIG:Debug>:
            -Og
            -g
            -fstack-protector-strong
            -D_DEBUG
            -D_GLIBCXX_ASSERTIONS
            -D_DEBUG_ASSERT
            >
            $<$<CONFIG:Release>:
            -Ofast
            -DNDEBUG
            -fuse-linker-plugin
            -fomit-frame-pointer
            -ffunction-sections
            -fdata-sections
            -fno-strict-aliasing
            -funroll-loops
            -fno-semantic-interposition
            >
    )

    target_link_options(magique-${MODULE_NAME} PRIVATE
            $<$<CONFIG:Debug>:
            -Wl,--as-needed
            -Wl,--no-undefined
            -Wl,--warn-common
            >

            $<$<CONFIG:Release>:
            -Wl,--gc-sections
            -Wl,--as-needed
            -Wl,--build-id
            -Wl,--no-undefined
            -Wl,--strip-all
            -Wl,--icf=all
            -Wl,--sort-section=alignment
            >
    )

    if (MAGIQUE_SANITIZER)
        target_compile_options(magique-${MODULE_NAME} PUBLIC
                -fsanitize=address -fno-omit-frame-pointer
        )
        target_link_options(magique-${MODULE_NAME} PUBLIC
                -fsanitize=address
        )
    endif ()
elseif (MSVC)
    target_compile_options(magique-${MODULE_NAME} PRIVATE
            $<$<CONFIG:Debug>:/W3 /Od /Zi /RTC1 /Zc:preprocessor>
            $<$<CONFIG:Release>:/DNDEBUG /W4 /O2 /fp:fast /arch:AVX2 /Zc:inline /Zc:preprocessor /GS /Gy /Oi /Gw /GF /GL /GR- /Oi >
    )

    target_link_options(magique-${MODULE_NAME} PRIVATE
            /LTCG /OPT:REF /OPT:ICF
    )

    if (MAGIQUE_SANITIZER)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /fsanitize=address")
    endif ()
else ()
    message(FATAL_ERROR "Compiler not supported")
endif ()

# Compile the definition for the target and all consuming ones
if (MAGIQUE_STEAM)
    target_compile_definitions(magique-${MODULE_NAME} PUBLIC MAGIQUE_STEAM)
    target_include_directories(magique-${MODULE_NAME} PRIVATE ${STEAM_PATH}/public) # Include
elseif (MAGIQUE_LAN)
    target_compile_definitions(magique-${MODULE_NAME} PUBLIC MAGIQUE_LAN)
endif ()

# Make the private members in public headers, public to the implementation (to avoid excessive friend...)
target_compile_definitions(magique-${MODULE_NAME} PRIVATE MAGIQUE_IMPLEMENTATION)