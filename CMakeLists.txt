cmake_minimum_required(VERSION 3.10)
project(onyx VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

# add_executable(test-utils-fnv1a test/utils/fnv1a.cpp)
# add_test(utils/fnv1a test-utils-fnv1a)
# add_executable(test-utils-bits test/utils/bits.cpp)
# add_test(utils/bits test-utils-bits)
# add_executable(test-utils-utf8 test/utils/utf8.cpp)
# add_test(utils/utf8 test-utils-utf8)
# add_executable(test-utils-coroutines test/utils/coroutines.cpp)
# add_test(utils/coroutines test-utils-coroutines)
# target_compile_options(test-temp PRIVATE -Wpragma-once-outside-header)

# add_custom_target(tests)
# add_dependencies(tests test-temp)
