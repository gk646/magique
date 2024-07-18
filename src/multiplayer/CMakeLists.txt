
set(MODULE_NAME "multiplayer")
include(${CMAKE_SOURCE_DIR}/cmake/DefineModule.cmake)

target_include_directories(${TARGET_NAME}_${MODULE_NAME} PRIVATE ${DEPENDENCY_PATH}/sdk/public)