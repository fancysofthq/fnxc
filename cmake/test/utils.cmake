set(UTIL_TESTS
  fnv1a
  bits
  utf8
  coro
  flatten_variant
  thread_pool
)

foreach(test ${UTIL_TESTS})
  add_executable(test-utils-${test} test/cpp/utils/${test}.cpp)
  add_test(utils/${test} test-utils-${test})
  add_dependencies(tests test-utils-${test})
endforeach()
