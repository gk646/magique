set(MODULE_NAME "multiplayer")
include(DefineModule)

if (MAGIQUE_STEAM)
    target_include_directories(magique-${MODULE_NAME} PRIVATE ${STEAM_SDK_PATH}/public)
endif ()