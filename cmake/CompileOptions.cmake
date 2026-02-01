# This script expects MODULE_NAME to be set
if (NOT DEFINED TARGET_NAME)
    message(FATAL_ERROR "TARGET_NAME is not defined")
    message("Use set(TARGET_NAME \"MyModule\") to set the name")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${TARGET_NAME} PUBLIC
            -std=c++23
            -march=native
            -flto=auto
            -fno-exceptions
            -fno-rtti
            -fvisibility=hidden
            -Wall
            -Wextra
            -Wpedantic
            -Wc++20-compat
            -Wc++20-extensions
            -Wcomma-subscript
            -Wdeprecated-declarations
            -Wuseless-cast
            -Wvla
            -Wno-useless-cast
            -Wno-extra-semi
            -Wno-missing-field-initializers
            -Wno-unused-parameter
    )

    target_compile_options(${TARGET_NAME} PUBLIC
            $<$<CONFIG:Debug>:
            -Og
            -g1
            -gz
           # -D_GLIBCXX_DEBUG
           # -D_GLIBCXX_ASSERTIONS
            -D_DEBUG
            >
            $<$<CONFIG:Release>:
            -Ofast
            -ffast-math
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

    target_link_options(${TARGET_NAME} PUBLIC
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
            -Wl,--sort-section=alignment
            >
    )

    if (MAGIQUE_SANITIZER)
        target_compile_options(${TARGET_NAME} PUBLIC
                -fsanitize=address -fno-omit-frame-pointer
        )
        target_link_options(${TARGET_NAME} PUBLIC
                -fsanitize=address
        )
    endif ()
elseif (MSVC)
    target_compile_options(${TARGET_NAME} PUBLIC
            $<$<CONFIG:Debug>:/W3 /Od /Zi /RTC1 /Zc:preprocessor>
            $<$<CONFIG:Release>:/DNDEBUG /W4 /O2 /fp:fast /arch:AVX2 /Zc:inline /Zc:preprocessor /GS /Gy /Oi /Gw /GF /GL /GR- /Oi >
    )

    target_link_options(${TARGET_NAME} PUBLIC
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
