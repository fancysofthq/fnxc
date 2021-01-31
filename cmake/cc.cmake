# Compiler configuration
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CLang
#

# add_compile_options("-H")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(STATUS "Compiler is CLang")

  if(MINGW)
    message(STATUS "Compiler is MinGW")

    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
      message(STATUS "Compiler is MSVC")
      add_compile_options("/clang:-fcoroutines-ts")

    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
      message(STATUS "Compiler is GNU")
      add_compile_options("-stdlib=libc++" "-fcoroutines-ts")
      add_link_options("-stdlib=libc++")

    else()
      message(FATAL_ERROR "Unsupported compiler")

    endif()

  elseif(WIN32)
    message(STATUS "Compiler is WIN32")

    # On Windows, libc++ is not needed?
    #

    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
      message(STATUS "Compiler is MSVC")
      add_compile_options("/clang:-fcoroutines-ts")

    elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
      message(STATUS "Compiler is GNU")
      add_compile_options("-fcoroutines-ts")

    else()
      message(FATAL_ERROR "Unsupported compiler")

    endif()

  elseif(UNIX)
    message(STATUS "Compiler is UNIX")

    # On Unix, libc++ is needed.
    #

    add_compile_options("-stdlib=libc++" "-fcoroutines-ts")
    add_link_options("-stdlib=libc++")

  else()
    message(FATAL_ERROR "Unsupported compiler")

  endif()

# GNU CC
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "Compiler is GNU")
  add_compile_options("-fcoroutines")

else ()
  message(FATAL_ERROR "Unsupported compiler")

endif ()
