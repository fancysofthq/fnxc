set(TEST_PROTOBUF_IMPORT_DIR ${CMAKE_SOURCE_DIR}/test/protobuf/proto)
set(TEST_PROTOBUF_SOURCE_DIR ${CMAKE_SOURCE_DIR}/test/protobuf/proto)

file(MAKE_DIRECTORY ${PROTOBUF_BINARY_DIR}/gen/fnx-test/protobuf)
set(TEST_PROTOBUF_GEN_DIR ${PROTOBUF_BINARY_DIR}/gen/fnx-test/protobuf)

add_custom_target(gen-test-protobufs
  DEPENDS protoc
  DEPENDS ${CMAKE_SOURCE_DIR}/test/protobuf/proto/foo.proto
  COMMAND ${CMAKE_COMMAND}
  -DPROTOC_EXECUTABLE=${PROTOBUF_PROTOC_EXECUTABLE}
  -DIMPORT_DIR=${TEST_PROTOBUF_IMPORT_DIR}
  -DSOURCE_DIR=${TEST_PROTOBUF_SOURCE_DIR}
  -DGEN_DIR=${TEST_PROTOBUF_GEN_DIR}
  -DROOT_DIR=${CMAKE_SOURCE_DIR}
  -P ${PROTOBUF_CMAKE_DIR}/GenerateProtos.cmake
  COMMENT "Generate protobufs for testing")

add_library(test-protobuf-foo ${TEST_PROTOBUF_GEN_DIR}/foo.pb.cc)
add_dependencies(test-protobuf gen-test-protobufs)
target_link_libraries(test-protobuf test-protobuf-foo libprotobuf-lite)
# target_include_directories(test-protobuf PRIVATE ${TEST_PROTOBUF_GEN_DIR})
# dump_cmake_variables()
