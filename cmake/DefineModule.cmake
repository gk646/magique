# This script expects MODULE_NAME to be set

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "PLUGIN_NAME is not defined")
    message("Use set(PLUGIN_NAME MyPlugin) to set the name")
endif ()

file(GLOB_RECURSE MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${TARGET_NAME}-${MODULE_NAME} STATIC ${MODULE_FILES})

target_include_directories(${TARGET_NAME}-${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${TARGET_NAME}-${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_include_directories(${TARGET_NAME}-${MODULE_NAME} PRIVATE ${SOURCE_PATH}/cxstructs/src)
target_include_directories(${TARGET_NAME}-${MODULE_NAME} PRIVATE ${SOURCE_PATH}/cxgpu/src)