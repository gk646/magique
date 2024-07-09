
# Create the package directory structure
file(MAKE_DIRECTORY "${CPACK_TEMPORARY_PACKAGE_DIRECTORY}/include")
file(MAKE_DIRECTORY "${CPACK_TEMPORARY_PACKAGE_DIRECTORY}/lib/${CMAKE_BUILD_TYPE}")

# Copy headers to the package directory
file(COPY include DESTINATION "${CPACK_TEMPORARY_PACKAGE_DIRECTORY}/")

# Copy the built library to the package directory
file(COPY "${CMAKE_SOURCE_DIR}/cmake-build-debug/magique.lib" DESTINATION "${CPACK_TEMPORARY_PACKAGE_DIRECTORY}/lib/debug")
file(COPY "${CMAKE_SOURCE_DIR}/cmake-build-release/magique.lib" DESTINATION "${CPACK_TEMPORARY_PACKAGE_DIRECTORY}/lib/release")
file(COPY "${CMAKE_SOURCE_DIR}/cmake/MagiqueConfig.cmake" DESTINATION "${CPACK_TEMPORARY_PACKAGE_DIRECTORY}/MagiqueConfig.cmake")