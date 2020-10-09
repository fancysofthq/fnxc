set(PACKAGE_VERSION 3.13.0)

if(
  ${PACKAGE_FIND_VERSION_MAJOR} EQUAL 3 AND
  ${PACKAGE_FIND_VERSION_MINOR} LESS_EQUAL 13
)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
