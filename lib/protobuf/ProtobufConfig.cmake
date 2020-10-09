# A Protobuf CMake package configuration assuming Protobuf 3.13.0
# source code residing in the `vendor` directory.
#

project(PROTOBUF VERSION 3.13.0)
cmake_minimum_required(VERSION 2.6)

# The NEW policy is required to overwrite the variable.
# See https://cmake.org/cmake/help/git-stage/policy/CMP0077.html.
cmake_policy(SET CMP0077 NEW)
set(protobuf_BUILD_TESTS OFF)

get_filename_component(PROTOBUF_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
add_subdirectory(${PROTOBUF_CMAKE_DIR}/vendor/cmake)

# Alias certain variables to honour the namimg conventions.
#

set(PROTOBUF_BINARY_DIR ${protobuf_BINARY_DIR})

# If the system has precompiled protoc installed, include dirs would be empty.
set(PROTOBUF_INCLUDE_DIRS ${PROTOBUF_CMAKE_DIR}/vendor/src)

# Ditto.
set(PROTOBUF_LIB_DIRS ${PROTOBUF_BINARY_DIR})

# If the system has a precompiled protoc installed, it'd use it instead.
# Which could lead to ABI problems depending on the current CMake toolchain.
set(PROTOBUF_PROTOC_EXECUTABLE ${PROTOBUF_BINARY_DIR}/protoc)
