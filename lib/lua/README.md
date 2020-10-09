# Lua

For consistency reasons, you have to explicitly create `LuaConfigVersion.cmake` file in this directory with similar contents.
See the main `CMakeLists.txt` for required Lua version.

```cmake
set(PACKAGE_VERSION 5.4.0)

if(${PACKAGE_FIND_VERSION_MAJOR} EQUAL 5 AND ${PACKAGE_FIND_VERSION_MINOR} EQUAL 4)
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
endif()
```
