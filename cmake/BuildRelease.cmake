# Include CPack to handle packaging

# Configure CPack
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME "${TARGET_NAME}-${MAGIQUE_VERSION}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_SOURCE_DIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include COMPONENT my_project_headers)
install(FILES ${CMAKE_SOURCE_DIR}/cmake-build-debug/magique.lib DESTINATION lib/debug COMPONENT my_project_libs)
install(FILES ${CMAKE_SOURCE_DIR}/cmake-build-release/magique.lib DESTINATION lib/release COMPONENT my_project_libs)
install(FILES ${CMAKE_SOURCE_DIR}/cmake/MagiqueConfig.cmake DESTINATION . COMPONENT my_project_config)

include(CPack)