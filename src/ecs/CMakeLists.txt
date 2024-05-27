
set(MODULE_NAME "ecs")
include(${CMAKE_SOURCE_DIR}/cmake/DefineModule.cmake)

target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/entt-3.13.1/src)