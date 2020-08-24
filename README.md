# Fancy Onyx Compiler

An Onyx programming language compiler implementation.

## About

The [Fancy Onyx Compiler](https://fnxc.fancysoft.xyz) (FNXC) is an Onyx programming language compiler implementation built in C++ on top of [LLVM](http://llvm.org/).

Target ISAs supported by FNXC are `i386`, `amd64`, `arm7` and `arm8`.

The `fnxc` binary implements the Onyx Compiler Interface standard (the `build` and `doc` commands), and does not peform neither linking nor dependency management.
For a fully-featured building environment, see [Fancy Onyx](https://fnx.fancysoft.xyz), which may use FNXC as a dependency.

Additional features of FNXC include running a program in JIT mode, REPL and Onyx Language Server Protocol implementations, and formatting.

## Usage

```console
$ fnxc -h
Fancy Onyx Compiler, v0

Usage:
  $ fnxc, fnxc repl               Run the REPL
  $ fnxc <file>, fnxc run <file>  Run a file in JIT mode
  $ fnxc build <file>             Build a file in AOT mode
  $ fnxc doc <file>               Build documentation
  $ fnxc serve                    Run the LSP server
  $ fnxc format <file>            Run the formatter

Options:
  -[-h]elp    Display help on topic
  -[-v]ersion Display FNXC version
```

## Development

1. Clone the FNXC repository.

    ```sh
    $ git clone https://github.com/fancysofthq/fnxc
    ```

1. Put the SQLite3 extensions source code under `lib/cpp/sqlite3/ext`, using either variant, so the `noop()` extension source code resides at `lib/cpp/sqlite3/ext/misc/noop.c`.

    1. Use [Fossil SCM](https://fossil-scm.org/).

        ```sh
        $ mkdir lib/cpp/sqlite3
        $ cd lib/cpp/sqlite3
        $ fossil clone https://www.sqlite.org/src sqlite.fossil
        $ fossil open sqlite.fossil
        ```

    1. Download a snapshot of the complete (raw) source tree from https://sqlite.org/download.html (e.g. `sqlite-src-3330000.zip`), and extract the `ext` folder to `lib/cpp/sqlite3/ext`.

    1. TODO: Enable extensions in the VCPG `sqlite3` dependency.

1. Install LLVM `~> 10` with exception handling enabled.
Make sure the `LLVMConfig.cmake` file can be found at `${LLVM_DIR}` (a variable later passed to CMake).

    When building LLVM from source, set the following CMake variables:

    ```
    -DLLVM_ENABLE_EH=ON
    -DLLVM_ENABLE_RTTI=ON
    -DLLVM_TARGETS_TO_BUILD=X86;ARM
    ```

    For example, on a Windows x64 machine:

    ```sh
    $ cmake \
      -Thost=x64 \
      -DCMAKE_C_COMPILER="C:/msys64/mingw64/bin/clang-cl.exe" \
      -DCMAKE_CXX_COMPILER="C:/msys64/mingw64/bin/clang-cl.exe" \
      -DCMAKE_LINKER="C:/msys64/mingw64/bin/lld-link.exe" \
      -DCMAKE_RANLIB="C:/msys64/mingw64/bin/llvm-ranlib.exe" \
      -DCMAKE_AR="C:/msys64/mingw64/bin/llvm-ar.exe" \
      -DLLVM_ENABLE_WARNINGS=OFF \
      -DLLVM_ENABLE_LTO=THIN \
      -DLLVM_ENABLE_EH=ON \
      -DLLVM_ENABLE_RTTI=ON \
      -DLLVM_TARGETS_TO_BUILD="X86;ARM"
    ```

    NOTE: For LTO usage with CLang, see https://clang.llvm.org/docs/ThinLTO.html#clang-bootstrap.

    NOTE: The manual LLVM installation shall be replaced with VCPKG dependency.
    Right now it:

      1. Does not support exception handling.

      1. Does not allow to use compiler other than MSVC, which fails with some `inconsistent behavior between llvm:: and std:: implementation of is_trivially_copyable` error on my machine.

1. Install [VCPKG](https://github.com/Microsoft/vcpkg).
Make sure you have `vcpkg.exe` in your `PATH` environment variable.

1. Configure the compiler using a recent [CMake](https://cmake.org/) version.
This will automatically install VCPKG  dependencies.

    ```sh
    # Note: change VCPKG_PATH and LLVM_DIR to reflect your environment
    $ cmake \
      -DCMAKE_TOOLCHAIN_FILE=${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake \
      -DLLVM_DIR=${LLVM_DIR}
    ```

1. Build the compiler.

    ```sh
    $ cmake --build . -t fnxc
    $ ./fnxc -v
    ```

    Pro tip: On Windows 10, temporaly disable Microsoft Defender Antivirual real-time protection to speed up the build.

1. (Optional) Run C++ tests during development

    ```sh
    $ cmake --build . -t tests
    $ ctest
    ```

1. (Optional) Run Onyx tests once the compiler is compiled

    ```sh
    $ ./fnxc ../test/nx/all.nx
    ```

NOTE: Do not clutter `.gitignore` with files local to your workflow!
Use the `$GIT_DIR/info/exclude` file instead, as per [Git documentation](https://git-scm.com/docs/gitignore).
