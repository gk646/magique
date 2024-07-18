# This script expects MODULE_NAME to be set

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not defined")
    message("Use set(MODULE_NAME MyModule) to set the name")
endif ()

file(GLOB_RECURSE MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${TARGET_NAME}_${MODULE_NAME} OBJECT ${MODULE_FILES})

target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${SOURCE_PATH}/cxstructs/src)