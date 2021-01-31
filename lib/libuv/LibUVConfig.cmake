# A LibUV CMake package configuration assuming LibUV 3.13.0
# source code residing in the `vendor` directory.
#

project(LIBUV VERSION 1.39.0)
cmake_minimum_required(VERSION 2.6)

# The NEW policy is required to overwrite the variable.
# See https://cmake.org/cmake/help/git-stage/policy/CMP0077.html.
# cmake_policy(SET CMP0077 NEW)
# set(LIBUV_BUILD_TESTS OFF)

get_filename_component(LIBUV_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
add_subdirectory(${LIBUV_CMAKE_DIR}/vendor)

# # Alias certain variables to honour the namimg conventions.
# #

# set(LIBUV_BINARY_DIR ${protobuf_BINARY_DIR})

# If the system has precompiled protoc installed, include dirs would be empty.
set(LIBUV_INCLUDE_DIR ${LIBUV_CMAKE_DIR}/vendor/include)

# Ditto.
set(LIBUV_LIB_DIR ${LIBUV_BINARY_DIR})

# # If the system has a precompiled protoc installed, it'd use it instead.
# # Which could lead to ABI problems depending on the current CMake toolchain.
# set(LIBUV_PROTOC_EXECUTABLE ${LIBUV_BINARY_DIR}/protoc)
