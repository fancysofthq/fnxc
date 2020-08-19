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

    ```console
    git clone https://github.com/fancysofthq/fnxc
    ```

1. Put the SQLite3 extensions source code under `lib/sqlite3/ext`, using either variant, so the `noop()` extension source code resides at `lib/sqlite3/ext/misc/noop.c`.

    1. Use [Fossil SCM](https://fossil-scm.org/).

        ```console
        mkdir lib/sqlite3
        cd lib/sqlite3
        fossil clone https://www.sqlite.org/src sqlite.fossil
        fossil open sqlite.fossil
        cd ../..
        ```

    1. Download a snapshot of the complete (raw) source tree from https://sqlite.org/download.html (e.g. `sqlite-src-3330000.zip`), and extract the `ext` folder to `lib/sqlite3/ext`.

1. Install [VCPKG](https://github.com/Microsoft/vcpkg).
Make sure you have `vcpkg.exe` in your `PATH` environment variable.

1. Configure the compiler using a recent [CMake](https://cmake.org/) version.
This will automatically install the dependencies.

    ```console
    mkdir build
    cd build

    # You have to set VCPKG as the CMake toolchain
    cmake \
      -DCMAKE_TOOLCHAIN_FILE=${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake \
      ../
    ```

1. Build the compiler.

    ```console
    # From ./build
    make fnxc
    ./fnxc -v
    ```

1. (Optional) Run C++ tests during development

    ```console
    # From ./build
    make tests
    ctest
    ```

1. (Optional) Run Onyx tests once the compiler is compiled

    ```console
    # From ./build
    ./fnxc ../test/nx/all.nx
    ```

NOTE: Do not clutter `.gitignore` with files local to your workflow!
Use the `$GIT_DIR/info/exclude` file instead, as per [Git documentation](https://git-scm.com/docs/gitignore).
