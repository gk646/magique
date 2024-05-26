

# Fetch all source files
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(${TARGET_NAME} ${SRC_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/luajit/src)
target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/cxstructs/include)
target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/cxgpu/include)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)


#link
target_link_libraries(${TARGET_NAME} PRIVATE "${DEPENDENCY_PATH}/luajit/src/lua51.lib")