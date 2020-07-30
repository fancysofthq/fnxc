cmake_minimum_required(VERSION 3.18)
project(onyx VERSION 0.1.0)

# `clang-cl`, comes with libc++ already?
if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND "x${CMAKE_CXX_SIMULATE_ID}" STREQUAL "xMSVC"))
  add_compile_options("/clang:-std=c++2a" "/clang:-fcoroutines-ts")

# CLang, but not `clang-cl`.
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options("-stdlib=libc++" "-std=c++2a" "-fcoroutines-ts")
  add_link_options("-stdlib=libc++")

# GCC is not supported yet.
else ()
  message(FATAL_ERROR "Only CLang is currently supported.")

endif ()

# set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# enable_testing()
include(CTest)

# find_package(llvm 8.0 REQUIRED)
# include_directories(${LLVM_INCLUDE_DIRS})

set(UTIL_TESTS
  fnv1a
  bits
  utf8
  coroutines
)

foreach(test ${UTIL_TESTS})
  add_executable(test-utils-${test} test/utils/${test}.cpp)
  add_test(utils/${test} test-utils-${test})
endforeach()
