cmake_minimum_required(VERSION 3.10)
project(fnxc VERSION 0.0.0)

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
  message(FATAL_ERROR "Only CLang is currently supported.")

endif ()

# Dependenices
#

# TODO: The package currently has `unknown` version
# TODO: Add extension sources to the package
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
include_directories(${SQLITE3_INCLUDE_DIR})

find_package(SQLiteCpp 3 CONFIG REQUIRED)
message(STATUS "Using SQLiteCpp v${SQLiteCpp_VERSION}")

find_package(doctest 2.4 CONFIG REQUIRED)
message(STATUS "Using doctest v${doctest_VERSION}")

find_package(Lua 5.3 REQUIRED)
message(STATUS "Using Lua v${Lua_VERSION}") # FIX: Determine version

find_package(LLVM 10 REQUIRED CONFIG)
message(STATUS "Using LLVM v${LLVM_VERSION}")
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

find_package(CLang 10 REQUIRED CONFIG)
message(STATUS "Using CLang v${CLang_VERSION}")
include_directories(${CLANG_INCLUDE_DIRS})
link_directories(${CLANG_LIB_DIR})

# Tests
#

set(UTIL_TESTS
  fnv1a
  bits
  utf8
  coroutines
)

set(TESTS
  sqlite
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

target_link_libraries(test-sqlite SQLiteCpp)

# Build targets
#

add_library(sqlite3-ext-regexp STATIC lib/cpp/sqlite3/ext/misc/regexp.c)

add_library(utils-log src/cpp/source/utils/log.cpp)
add_library(utils-null_stream src/cpp/source/utils/null_stream.cpp)
# add_library(app-aot src/cpp/source/app/aot.cpp)

add_executable(fnxc src/cli.cpp)

target_link_libraries(fnxc
  utils-log
  utils-null_stream
  unofficial::sqlite3::sqlite3
  sqlite3-ext-regexp
  # app-aot
)
