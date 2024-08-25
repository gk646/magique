# This script expects MODULE_NAME to be set
if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not defined")
    message("Use set(MODULE_NAME \"MyModule\") to set the name")
endif ()

file(GLOB_RECURSE MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(magique-${MODULE_NAME} OBJECT ${MODULE_FILES})

target_include_directories(magique-${MODULE_NAME} PRIVATE
        ${MAGIQUE_PUBLIC_INCLUDE} # Public includes
        ${MAGIQUE_PRIVATE_INCLUDE} # Private includes
        ${MAGIQUE_CXSTRUCTS_INCLUDE} # cxstructs includes
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -ffast-math -fno-exceptions -fno-rtti -fvisibility=hidden)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -g")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -ffast-math -fno-exceptions -fno-rtti")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -flto -fno-exceptions -fno-rtti -ffast-math")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    if (ENABLE_SANITIZER)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    endif ()
elseif (MSVC)
    # Clear flags
    set(CMAKE_CXX_FLAGS_DEBUG "/W4 /Od /Zi /Zc:preprocessor")
    set(CMAKE_CXX_FLAGS_RELEASE "/W4 /O2 /Zi /Zc:preprocessor")

    target_compile_options(magique-${MODULE_NAME} PRIVATE
           /Ob2 /EHc /favor:INTEL64 /GA /fp:fast /arch:AVX2 /GS- /Gy /Oi /Gw /GF /GL /GR- /Oi
    )
    target_link_options(magique-${MODULE_NAME} PRIVATE
            /LTCG /OPT:REF /OPT:ICF
    )

    if (MAGIQUE_ENABLE_SANITIZER)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /fsanitize=address")
    endif ()

endif ()