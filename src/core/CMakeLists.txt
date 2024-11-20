set(MODULE_NAME "core")
include(DefineModule)


if (MAGIQUE_STEAM)
    target_include_directories(magique-${MODULE_NAME} PRIVATE ${STEAM_PATH}/public)
endif ()