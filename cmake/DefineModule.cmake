# This script expects MODULE_NAME to be set
if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not defined")
    message("Use set(MODULE_NAME \"MyModule\") to set the name")
endif ()

# Parse files
file(GLOB_RECURSE MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Add the module as static library
add_library(magique-${MODULE_NAME} OBJECT ${MODULE_FILES})

target_include_directories(magique-${MODULE_NAME} PRIVATE
        ${MAGIQUE_PUBLIC_INCLUDE} # Public includes
        ${MAGIQUE_PRIVATE_INCLUDE} # Private includes
        ${MAGIQUE_CXSTRUCTS_INCLUDE} # cxstructs includes
)

set(TARGET_NAME magique-${MODULE_NAME})
include(CompileOptions)

# Compile the definition for the target and all consuming ones
if (MAGIQUE_STEAM)
    target_compile_definitions(magique-${MODULE_NAME} PUBLIC MAGIQUE_STEAM)
    set(FULL_STEAM_PATH "${STEAM_PATH}/public")
    if (NOT IS_ABSOLUTE "${FULL_STEAM_PATH}")
        set(FULL_STEAM_PATH "${CMAKE_SOURCE_DIR}/${FULL_STEAM_PATH}")
    endif ()
    target_include_directories(magique-${MODULE_NAME} PRIVATE ${FULL_STEAM_PATH}) # Include
elseif (MAGIQUE_LAN)
    target_compile_definitions(magique-${MODULE_NAME} PUBLIC MAGIQUE_LAN)
endif ()

# Make the private members in public headers, public to the implementation (to avoid excessive friend...)
target_compile_definitions(magique-${MODULE_NAME} PRIVATE MAGIQUE_IMPLEMENTATION)