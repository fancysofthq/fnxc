find_package(LLVM 10 REQUIRED CONFIG)
message(STATUS "Found LLVM v${LLVM_VERSION}")
message(VERBOSE "LLVM_INCLUDE_DIRS == ${LLVM_INCLUDE_DIRS}")
# message(VERBOSE "LLVM_LIB_DIR == ${LLVM_LIB_DIR}")
include_directories(${LLVM_INCLUDE_DIRS})
