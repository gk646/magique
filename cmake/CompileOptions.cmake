
# Set include directories for the main library
target_include_directories(magique PUBLIC ${MAGIQUE_PUBLIC_INCLUDE})

target_include_directories(magique PRIVATE
        ${MAGIQUE_PUBLIC_INCLUDE} # Public includes
        ${MAGIQUE_PRIVATE_INCLUDE} # Private includes
        ${MAGIQUE_CXSTRUCTS_INCLUDE} # cxstructs includes
)

# Compile the definition for the target and all consuming ones
if (MAGIQUE_STEAM)
    target_compile_definitions(magique PUBLIC MAGIQUE_STEAM)
    set(FULL_STEAM_PATH "${STEAM_PATH}/public")
    if (NOT IS_ABSOLUTE "${FULL_STEAM_PATH}")
        set(FULL_STEAM_PATH "${CMAKE_SOURCE_DIR}/${FULL_STEAM_PATH}")
    endif ()
    target_include_directories(magique PRIVATE ${FULL_STEAM_PATH}) # Include
elseif (MAGIQUE_LAN)
    target_compile_definitions(magique PUBLIC MAGIQUE_LAN)
endif ()

# Make the private members in public headers, public to the implementation (to avoid excessive friend...)
target_compile_definitions(magique PRIVATE MAGIQUE_IMPLEMENTATION)


if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(magique PUBLIC
            -std=c++23
            -march=haswell
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

    target_compile_options(magique PUBLIC
            $<$<CONFIG:Debug>:
            -Og
            -g2
            -gz
           # -D_GLIBCXX_DEBUG
           # -D_GLIBCXX_ASSERTIONS
            -D_DEBUG
            >
            $<$<CONFIG:Release>:
            -Ofast
            -DNDEBUG
            -fuse-linker-plugin
            -fomit-frame-pointer
            -ffunction-sections
            -fdata-sections
            -fno-strict-aliasing
            -fno-semantic-interposition
            >
    )

    target_link_options(magique PUBLIC
            -flto=auto
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
        target_compile_options(magique PUBLIC
                -fsanitize=address -fno-omit-frame-pointer
        )
        target_link_options(magique PUBLIC
                -fsanitize=address
        )
    endif ()
elseif (MSVC)
    target_compile_options(magique PUBLIC
            $<$<CONFIG:Debug>:/W3 /Od /Zi /RTC1 /Zc:preprocessor>
            $<$<CONFIG:Release>:/DNDEBUG /W4 /O2 /fp:fast /arch:AVX2 /Zc:inline /Zc:preprocessor /GS /Gy /Oi /Gw /GF /GL /GR- /Oi >
    )

    target_link_options(magique PUBLIC
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

target_compile_definitions(magique PUBLIC
        MAGIQUE_VERSION="${PROJECT_VERSION}"
        MAGIQUE_LOGIC_TICKS=${MAGIQUE_LOGIC_TICKS}
        MAGIQUE_TICK_TIME=1.0F/${MAGIQUE_LOGIC_TICKS}
        MAGIQUE_WORKER_THREADS=${MAGIQUE_WORKER_THREADS}
        MAGIQUE_COLLISION_CELL_SIZE=${MAGIQUE_COLLISION_CELL_SIZE}
        MAGIQUE_MAX_ENTITIES_CELL=${MAGIQUE_MAX_ENTITIES_CELL}
        MAGIQUE_PATHFINDING_CELL_SIZE=${MAGIQUE_PATHFINDING_CELL_SIZE}
        MAGIQUE_MAX_PATH_SEARCH_CAPACITY=${MAGIQUE_MAX_PATH_SEARCH_CAPACITY}
        MAGIQUE_CHECK_EXISTS_BEFORE_EVENT=${MAGIQUE_CHECK_EXISTS_BEFORE_EVENT}
        MAGIQUE_TEXTURE_ATLAS_SIZE=${MAGIQUE_TEXTURE_ATLAS_SIZE}
        MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES=${MAGIQUE_TILE_OBJECT_CUSTOM_PROPERTIES}
        MAGIQUE_TILE_SET_CUSTOM_PROPERTIES=${MAGIQUE_TILE_SET_CUSTOM_PROPERTIES}
        MAGIQUE_MAX_ANIM_FRAMES=${MAGIQUE_MAX_ANIM_FRAMES}
        MAGIQUE_MAX_PLAYERS=${MAGIQUE_MAX_PLAYERS}
        MAGIQUE_MAX_LOBBY_MESSAGE_LEN=${MAGIQUE_MAX_LOBBY_MESSAGE_LEN}
        MAGIQUE_PARTICLE_COLORPOOL_SIZE=${MAGIQUE_PARTICLE_COLORPOOL_SIZE}
        MAGIQUE_MAX_FORMAT_LEN=${MAGIQUE_MAX_FORMAT_LEN}
        MAGIQUE_MAX_SUPPORTED_TIMERS=${MAGIQUE_MAX_SUPPORTED_TIMERS}
        MAGIQUE_INCLUDE_FONT=${MAGIQUE_INCLUDE_FONT}
)
