# Doctest

For consistency reasons, you have to explicitly create `DoctestConfigVersion.cmake` file in this directory with similar contents.
See the main `CMakeLists.txt` for required Doctest version.

```cmake
set(PACKAGE_VERSION 2.4.0)

if(${PACKAGE_FIND_VERSION_MAJOR} EQUAL 2)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
```
