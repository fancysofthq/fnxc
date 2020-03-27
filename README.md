# OnyxC

OnyxC is the canonical Onyx Compiler. It relies on LLVM to do cross-platform heavy-lifting.

```console
$ onyxc -h
OnyxC, the canonical Onyx language compiler

Usage:
  <file>, run <file>  Run a file in JIT mode
  build <file>        Build a file in AOT mode
  api <file>          Build API documentation
  repl                Run the REPL
  serve               Run the language server
  format <file>       Run the formatter
```

```console
$ onyxc repl -h
The canonical Onyx Read-Eval-Print Loop
REPL version 1.0.0, OnyxC version 1.0.0

Usage:
  $ onyxc repl [options]

Options:
  -[-h]elp    Display this help
  -[-v]ersion Display the REPL version

  -[-r]equire <path>

    Require before running for the first time

      $ onyxc repl -rstd/io
      $ onyxc repl --require file.nx

  -[-R]equire-path <path>

    Add path to look up on requiring

      $ onyxc repl -R.shard -rstd/io

  -[-I]nclude-path <path> Add include lookup path
  -[-l]ib <libname>       Add a library to link
  -[-L]ib-path <path>     Add library lookup path

  -[-O]ptimize <level>

    Enable JIT optimizations from 0 (default) to 2

      $ onyxc repl -O1
```

## Targets

Target consists of two parts: ISA ABI and OS ABI. Not all combinations are supported by the compiler, though.

ISA / OS | `eabi` | `linux` | `win32` | `darwin` | `bsd`
---      | ---    | ---     | ---     | ---      | ---
`x86_64` | T?     | T?      | T?      | T?       | T?
`arm`    | T?     | T?      | T?      | T?       | T?
`spark`  | T?     | T?      | T?      | T?       | T?
`riscv`  | T?     | T?      | T?      | T?       | T?
