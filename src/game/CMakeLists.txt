
set(MODULE_NAME "game")

file(GLOB_RECURSE MODULE_FILES Game.cpp)


add_library(${TARGET_NAME}_${MODULE_NAME} OBJECT ${MODULE_FILES})

target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${SOURCE_PATH}/cxstructs/src)
target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${SOURCE_PATH}/cxgpu/src)


target_link_libraries(${TARGET_NAME}_${MODULE_NAME} PUBLIC raylib)
target_include_directories(${TARGET_NAME}_${MODULE_NAME} PUBLIC external/raylib/src)