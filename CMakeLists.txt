cmake_minimum_required(VERSION 3.10)
project(fnx VERSION 0.0.0)

# Compiler configuration
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# `clang-cl`, comes with libc++ already?
if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC"))
  add_compile_options("/clang:-fcoroutines-ts")

# CLang, but not `clang-cl`.
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options("-stdlib=libc++" "-fcoroutines-ts")
  add_link_options("-stdlib=libc++")

# GCC is not supported yet.
else ()
  message(FATAL_ERROR "GCC is not supported yet")

endif ()

# Dependenices
#

find_package(doctest 2.4 REQUIRED CONFIG)
find_path(DOCTEST_INCLUDE_DIR doctest/doctest.h)
include_directories(${DOCTEST_INCLUDE_DIR})
message(STATUS "Using DOCtest v${DOCTEST_VERSION}")
message(STATUS "DOCtest include dir: ${DOCTEST_INCLUDE_DIR}")

find_package(Lua 5.3 REQUIRED)
message(STATUS "Using Lua v${Lua_VERSION}") # FIX: Determine version

find_package(LLVM 10 REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
message(STATUS "Using LLVM v${LLVM_VERSION}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM library dirs: ${LLVM_LIB_DIR}")

find_package(CLang 10 REQUIRED CONFIG)
include_directories(${CLANG_INCLUDE_DIRS})
link_directories(${CLANG_LIB_DIR})
message(STATUS "Using CLang v${CLang_VERSION}")
message(STATUS "CLang include dirs: ${CLANG_INCLUDE_DIRS}")
message(STATUS "CLang library dirs: ${CLANG_LIB_DIR}")

# Tests
#

set(UTIL_TESTS
  fnv1a
  bits
  utf8
  coroutines
  flatten_variant
)

set(TESTS
  clang
)

# set(BUILD_TESTING 0) # We don't want that bunch of targets
enable_testing()
include(CTest)

add_custom_target(tests)

foreach(test ${UTIL_TESTS})
  add_executable(test-utils-${test} test/cpp/utils/${test}.cpp)
  add_test(utils/${test} test-utils-${test})
  add_dependencies(tests test-utils-${test})
endforeach()

foreach(test ${TESTS})
  add_executable(test-${test} test/cpp/${test}.cpp)
  add_test(${test} test-${test})
  add_dependencies(tests test-${test})
endforeach()

target_link_libraries(test-clang libclang)

# Build targets
#

add_library(utils-logging src/cpp/source/utils/logging.cpp)
add_library(utils-null_stream src/cpp/source/utils/null_stream.cpp)
# add_library(app-aot src/cpp/source/app/aot.cpp)

add_executable(fnxc src/cli.cpp)

target_link_libraries(fnxc
  utils-logging
  utils-null_stream
  # app-aot
)

# Documentation
#

add_subdirectory ("doc/doxygen")
