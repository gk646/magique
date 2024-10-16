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
    # Baseline compile flags for GCC/Clang
    set(CMAKE_CXX_FLAGS_DEBUG "-Og -g -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG -march=native -mavx -flto -fno-exceptions -fno-rtti -ffast-math")
    set(CMAKE_EXE_LINKER_FLAGS "-flto")

    target_compile_options(magique-${MODULE_NAME} PRIVATE
            -Wall -ffast-math -fno-exceptions -fno-rtti -fvisibility=hidden
    )

    target_link_options(magique-${MODULE_NAME} PRIVATE
            -flto
    )

    if (ENABLE_SANITIZER)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    endif ()
elseif (MSVC)
    # Clear flags
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /Ob3")

    target_compile_options(magique-${MODULE_NAME} PRIVATE
            /W4 /Zc:preprocessor /EHc /GA /fp:fast /arch:AVX /GS- /Gy /Oi /Gw /GF /GL /GR- /Oi
    )
    target_link_options(magique-${MODULE_NAME} PRIVATE
            /LTCG /OPT:REF /OPT:ICF
    )

    if (MAGIQUE_ENABLE_SANITIZER)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /fsanitize=address")
    endif ()
else ()
    message(FATAL_ERROR "Compiler not supported")
endif ()

if(MAGIQUE_STEAM)
    target_compile_definitions(magique-${MODULE_NAME} PUBLIC MAGIQUE_USE_STEAM) # Compile the definition for the target and all consuming ones
endif ()