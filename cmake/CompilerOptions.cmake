if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -ffast-math -fno-exceptions -fvisibility=hidden)

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -g -march=native -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g -march=native -fno-omit-frame-pointer")

    set(CMAKE_C_FLAGS_RELEASE "-Ofast -ffast-math -fno-exceptions  -march=native -funroll-loops ")
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -ffast-math -fno-exceptions -fno-rtti -march=native -funroll-loops ")

    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    #set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto")

    if (MAGIQUE_ENABLE_SANITIZER)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    endif ()
elseif (MSVC)
    add_compile_options(/favor:INTEL64 /GA /fp:fast /arch:AVX2 /GS- /Gy /Oi /Gw /GF /GL /GR- /Zc:preprocessor)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /W3 /Od /Zi")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4 /Od /Zi")

    set(CMAKE_C_FLAGS_RELEASE "/O2 /Ob3 /EHc /W3 /EHs /GL")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3 /EHc /W4")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF")

    if (MAGIQUE_ENABLE_SANITIZER)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /fsanitize=address")
    endif ()

endif ()

add_compile_definitions(-D_HAS_EXCEPTIONS=0)