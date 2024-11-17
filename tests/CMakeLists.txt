add_executable(magique-test main.cpp)

if(UNIX)
    target_compile_options(magique-test PRIVATE -fno-rtti) # Per default compiles with no rtti
endif ()

target_link_libraries(magique-test PRIVATE magique)