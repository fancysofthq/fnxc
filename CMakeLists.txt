cmake_minimum_required(VERSION 3.15)
project(FNX VERSION 0.0.0)

# Utils
include(cmake/utils.cmake)

# Compiler options
include(cmake/cc.cmake)

# Dependenices
include(cmake/dep.cmake)

# Main include files
include_directories(${PROJECT_SOURCE_DIR}/include)

# Tests
include(cmake/test.cmake)

# Build targets
include(cmake/build.cmake)

# Documentation
include(cmake/doc.cmake)
