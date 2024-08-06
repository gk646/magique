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
    add_compile_options(/fp:fast /arch:AVX2 /Gy /GA /Oi /Gw /GF /GL /GR- /Zc:preprocessor -D_HAS_EXCEPTIONS=0)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}  /W3 /Od /Zi")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /Od /Zi")

    set(CMAKE_C_FLAGS_RELEASE "/O2 /Ob3 /EHc /W3 /arch:AVX2 /EHs /GL")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3 /EHc /W4 /std:c++20 /openmp:experimental")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF")

    # Explanation of flags:
    # /arch:AVX2       - Enable AVX2 instruction set
    # /O2              - Maximize speed (Release optimization level)
    # /Ob3             - Inline functions aggressively
    # /GA              - Optimize for Windows applications (assumes no console)
    # /Gy              - Enable function-level linking
    # /GL              - Enable whole program optimization
    # /EHc             - Synchronous exception handling, minimal overhead
    # /GR-             - Disable RTTI (Runtime Type Information)
    # /GF              - Enable read-only string pooling

    # Linker flags for release configuration:
    # /LTCG            - Link-time code generation
    # /OPT:REF         - Eliminate unused functions/data
    # /OPT:ICF         - Identical COMDAT folding (remove duplicate code)

    if (${PROJECT_PREFIX}_ENABLE_SANITIZER)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /fsanitize=address")
    endif ()
endif ()