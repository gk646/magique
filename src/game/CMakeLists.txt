
set(MODULE_NAME "game")
include(${CMAKE_SOURCE_DIR}/cmake/DefineModule.cmake)


target_link_libraries(${TARGET_NAME}_${MODULE_NAME} PUBLIC raylib)
target_include_directories(${TARGET_NAME}_${MODULE_NAME} PUBLIC external/raylib/src)