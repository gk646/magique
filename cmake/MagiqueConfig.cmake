# Set the include directory
set(MAGIQUE_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

# Set the library directory based on the build type
set(MAGIQUE_LIB_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_BUILD_TYPE}")

# Define the imported library target
add_library(magique STATIC IMPORTED)
set_target_properties(magique PROPERTIES
        IMPORTED_LOCATION "${MAGIQUE_LIB_DIR}/magique.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${MAGIQUE_INCLUDE_DIR}"
)