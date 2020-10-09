# set(BUILD_TESTING 0) # We don't want that bunch of targets
enable_testing()
include(CTest)

add_custom_target(tests)

include(cmake/test/utils.cmake)
include(cmake/test/root.cmake)
