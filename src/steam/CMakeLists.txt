set(MODULE_NAME "steam")
include(DefineModule)

target_include_directories(magique-${MODULE_NAME} PRIVATE ${DEPENDENCY_PATH}/sdk/public)