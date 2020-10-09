set(TESTS
  clang
  sqlitecpp
  protobuf
)

foreach(test ${TESTS})
  add_executable(test-${test} test/cpp/${test}.cpp)
  add_test(${test} test-${test})
  add_dependencies(tests test-${test})
endforeach()

include(cmake/test/root/clang.cmake)
include(cmake/test/root/sqlitecpp.cmake)
include(cmake/test/root/protobuf.cmake)
