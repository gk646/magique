
set(MODULE_NAME "lua")
include(${CMAKE_SOURCE_DIR}/cmake/DefineModule.cmake)

target_include_directories(${TARGET_NAME}_${MODULE_NAME} PUBLIC ${DEPENDENCY_PATH}/luajit/src)
target_link_libraries(${TARGET_NAME}_${MODULE_NAME} PUBLIC ${DEPENDENCY_PATH}/luajit/src/lua51.lib)