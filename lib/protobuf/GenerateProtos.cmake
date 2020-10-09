file(GLOB_RECURSE PROTO_FILES ${SOURCE_DIR}/*.proto)
list(LENGTH PROTO_FILES PROTO_FILES_LENGTH)

# Paths shall be converte to native prior to passing to an executable
file(TO_NATIVE_PATH ${IMPORT_DIR} NATIVE_IMPORT_DIR)
file(TO_NATIVE_PATH ${GEN_DIR} NATIVE_GEN_DIR)

# TODO: Make it parallel?
foreach(PROTO_PATH ${PROTO_FILES})
  file(TO_NATIVE_PATH ${PROTO_PATH} PROTO_NATIVE_PATH)

  execute_process(
    COMMAND ${PROTOC_EXECUTABLE}
    --proto_path=${NATIVE_IMPORT_DIR}
    --cpp_out=${NATIVE_GEN_DIR} ${PROTO_NATIVE_PATH}
    RESULT_VARIABLE RESULT_VAR)

  if(${RESULT_VAR})
    message(FATAL_ERROR "Generation of protobuf returned non-zero code \
      ${RESULT_VAR} for proto ${PROTO_PATH}")
  else()
    MATH(EXPR COUNTER "${COUNTER}+1")
    file(RELATIVE_PATH PROTO_REL_PATH ${ROOT_DIR} ${PROTO_PATH})
    message(STATUS "[${COUNTER}/${PROTO_FILES_LENGTH}] From ./${PROTO_REL_PATH}")
  endif()
endforeach()

file(RELATIVE_PATH PROTOS_REL_PATH ${ROOT_DIR} ${GEN_DIR})
message(STATUS "Generated ${PROTO_FILES_LENGTH} protobuf(s) to ./${PROTOS_REL_PATH}/")
