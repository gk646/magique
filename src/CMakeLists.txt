
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Fetch all source files
file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(${TARGET_NAME} ${SRC_FILES})

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/luajit/src)
target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/Qt/6.7.1/msvc2019_64/include)
target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/cxstructs/include)
target_include_directories(${TARGET_NAME} PUBLIC ${DEPENDENCY_PATH}/cxgpu/include)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)


target_link_libraries(${TARGET_NAME} PRIVATE ${DEPENDENCY_PATH}/luajit/src/lua51.lib)

# Specify the path to the Qt installation directory
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${DEPENDENCY_PATH}/Qt/6.7.1/msvc2019_64")

# Find the required Qt packages
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)  # Add other components as needed

# Assuming TARGET_NAME is your project name
target_link_libraries(${TARGET_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)