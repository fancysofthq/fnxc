# A Lua CMake package configuration assuming Lua 5.4.0
# source code residing in the `vendor` directory.
#
# NOTE: `LuaConfigVersion.cmake` or similar file is required.
# See the `README.md` file for details.
#

project(SQLITE VERSION 3.33.0)
cmake_minimum_required(VERSION 2.6)

find_package(Threads REQUIRED)

get_filename_component(SQLITE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

set(SQLITE_INCLUDE_DIR ${SQLITE_CMAKE_DIR}/vendor)

list(APPEND SQLITE_DEFINITIONS
  "-DSQLITE_ENABLE_COLUMN_METADATA"
  "-DSQLITE_ENABLE_DESERIALIZE"
  "-DSQLITE_ENABLE_MEMORY_MANAGEMENT"
  "-DSQLITE_USE_ALLOCA"
  "-DSQLITE_THREADSAFE=2"
  "-DSQLITE_TEMP_STORE=3"
)

add_library(sqlite ${SQLITE_CMAKE_DIR}/vendor/sqlite3.c)
# set_target_properties(sqlite PROPERTIES LINKER_LANGUAGE CXX)

if(MINGW OR WIN32)
  set_target_properties(sqlite PROPERTIES DEFINE_SYMBOL "SQLITE_API=__declspec(dllexport)")
endif()
