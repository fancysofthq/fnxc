add_library(utils-logging src/cpp/utils/logging.cpp)
add_library(utils-null_stream src/cpp/utils/null_stream.cpp)
# add_library(app-aot src/cpp/app/aot.cpp)

add_executable(fnx src/cli.cpp)

target_compile_definitions(fnx PUBLIC -DLUA_USER_H="${LUA_USER_H}")
target_link_directories(fnx PUBLIC ${LUA_BINARY_DIR})
target_link_libraries(fnx lualib)

target_compile_definitions(fnx PUBLIC SQLITE_DEFINITIONS)
target_link_libraries(fnx sqlite)

target_link_libraries(fnx sqlitecpp)

target_compile_definitions(fnx PUBLIC ${LLVM_DEFINITIONS})
target_link_directories(fnx PUBLIC ${CLANG_LIB_DIR})

target_link_libraries(fnx
  utils-logging
  utils-null_stream
  # app-aot
)

if(UNIX)
  target_link_libraries(fnx m)
endif()
