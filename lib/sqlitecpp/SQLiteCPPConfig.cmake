project(SQLITECPP VERSION 3.1.1)
cmake_minimum_required(VERSION 2.6)

get_filename_component(SQLITECPP_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
set(SQLITECPP_INCLUDE_DIR ${SQLITECPP_CMAKE_DIR}/vendor/include)

add_library(sqlitecpp
  ${SQLITECPP_CMAKE_DIR}/vendor/src/Backup.cpp
  ${SQLITECPP_CMAKE_DIR}/vendor/src/Column.cpp
  ${SQLITECPP_CMAKE_DIR}/vendor/src/Database.cpp
  ${SQLITECPP_CMAKE_DIR}/vendor/src/Exception.cpp
  ${SQLITECPP_CMAKE_DIR}/vendor/src/Statement.cpp
  ${SQLITECPP_CMAKE_DIR}/vendor/src/Transaction.cpp
)
