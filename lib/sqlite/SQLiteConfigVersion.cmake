get_filename_component(SQLITE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

file(STRINGS "${SQLITE_CMAKE_DIR}/vendor/sqlite3.h" _sqlite_api_h_VER_STRING REGEX ".*#define[ ]+SQLITE_VERSION[ ]+")

string(REGEX MATCH "[0-9\\.]+" SQLITE_VER_STRING ${_sqlite_api_h_VER_STRING})
string(REGEX MATCHALL "[0-9]+" _sqlite_ver_LIST "${SQLITE_VER_STRING}")
list(LENGTH _sqlite_ver_LIST _sqlite_list_len)
list(GET _sqlite_ver_LIST 0 SQLITE_VER_MAJOR)
list(GET _sqlite_ver_LIST 1 SQLITE_VER_MINOR)
list(GET _sqlite_ver_LIST 2 SQLITE_VER_PATCH)
if(_sqlite_list_len EQUAL 4)
    list(GET _sqlite_ver_LIST 3 SQLITE_VER_PATCHLEVEL)
endif()

set(PACKAGE_VERSION ${SQLITE_VER_MAJOR}.${SQLITE_VER_MINOR}.${SQLITE_VER_PATCH})

if(
  ${PACKAGE_FIND_VERSION_MAJOR} EQUAL ${SQLITE_VER_MAJOR} AND
  ${PACKAGE_FIND_VERSION_MINOR} LESS_EQUAL ${SQLITE_VER_MINOR}
)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
