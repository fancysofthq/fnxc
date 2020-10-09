# A Doctest CMake package configuration assuming Doctest 2.4.0
# source code residing in the `vendor` directory.
#
# NOTE: `DoctestConfigVersion.cmake` or similar file is required.
# See the `README.md` file for details.
#

project(DOCTEST VERSION 2.4.0)
cmake_minimum_required(VERSION 2.6)

get_filename_component(DOCTEST_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(DOCTEST_INCLUDE_DIR "${DOCTEST_CMAKE_DIR}/vendor")
