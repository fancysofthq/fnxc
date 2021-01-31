# A UVW CMake package configuration assuming UVW 3.13.0
# source code residing in the `vendor` directory.
#

project(UVW VERSION 2.7.0)
cmake_minimum_required(VERSION 2.7)

# The NEW policy is required to overwrite the variable.
# See https://cmake.org/cmake/help/git-stage/policy/CMP0077.html.
cmake_policy(SET CMP0077 NEW)
set(BUILD_UVW_LIBS OFF)
set(BUILD_TESTING OFF)

get_filename_component(UVW_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
add_subdirectory(${UVW_CMAKE_DIR}/vendor)

# # Alias certain variables to honour the namimg conventions.
# #

# set(UVW_BINARY_DIR ${protobuf_BINARY_DIR})

# If the system has precompiled protoc installed, include dirs would be empty.
set(UVW_INCLUDE_DIRS ${UVW_CMAKE_DIR}/vendor/src)

# # Ditto.
# set(UVW_LIB_DIRS ${UVW_BINARY_DIR})

# # If the system has a precompiled protoc installed, it'd use it instead.
# # Which could lead to ABI problems depending on the current CMake toolchain.
# set(UVW_PROTOC_EXECUTABLE ${UVW_BINARY_DIR}/protoc)
