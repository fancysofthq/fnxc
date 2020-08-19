```console
$ fnxc repl -h
Run the Onyx Read-Eval-Print Loop

Usage:
  $ fnxc [options]
  $ fnxc repl [options]

Options:
  -[-h]elp  Display this help

  -[-r]equire <path>

    Require a file before running for the first time

      $ fnxc repl -rstd/io
      $ fnxc repl --require file.nx

  -[-R]equire-path <path>

    Add a path to look up on requiring

      $ fnxc repl -R.shard -rstd/io

  -[-I]import-path <path> Add an import lookup path
  -[-l]ib <libname>       Add a library to link
  -[-L]ib-path <path>     Add a library lookup path

  -[-O]ptimize <level>

    Enable JIT optimizations from 0 (default) to 2

      $ fnxc repl -O1
```

```console
$ fnxc run -h
Run the program in JIT compilation mode

Usage:
  $ fnxc <file> [options]
  $ fnxc run <file> [options]

Options:
  -[-h]elp  Display this help


```
