---
layout: index.html
permalink: "index.html"
title: OnyxC design
---

# OnyxC design

## AOT

In the AOT mode, OnyxC has the following stages of compilation:

  1. The source code is compiled into SLIR, a.k.a. Source-Level Intermediate Representation. SLIR represents the code exactly the way a programmer intended, but in a more machine-friendly way. SLIR is an OnyxC feature;
  2. Then, SLIR is compiled into NXIR (the Onyx Itermediate Representation), an AST with functions and types inferred, specialized and checked. Specializations are guaranteed to be unique. The NXIR standard is defined elsewhere;
  3. After that, NXIR is compiled into LLIR, the LLVM (or alternatively, Low-Level) Intermediate Representation. Depending on the desired optimization level, [ThinLTO](https://blog.llvm.org/2016/06/thinlto-scalable-and-incremental-lto.html) metadata may be included into the modules. In the end, each source file corresponds to a separate LLIR module to be linked.
  4. Finally, modules are linked into a single assembly with optional LLVM optimization stages.

All IRs may be [cached](#cache) either in human- (e.g. `.nxir`) or machine- (e.g. `.nxbc`) friendly formats.

## JIT

In the JIT mode, LLIR is not stored on the disk. Instead, it's loaded directly into the LLVM JIT engine, with optional optimizations.

JIT compilation happens on the same machine, thus there is no need to worry about the resulting binary size. Hence the optimization only cares about a compiled function performance.

### Hot reloading

When hot reloading is enabled, source files in the dependency tree are being contiguously watched for updates. Internally, the compiler compares NXIRs before and after an update. Only certain types of changes trigger hot reloading:

  * Function body changes, but not of their prototypes;
  * New function definitions, including overloading. This could lead to dependant functions (i.e. which are or may be calling the new function) reloading as well;
  * New `require`s and `import`s. Deleting them would require full reload;
  * Value changes of static variables, but not of their types. This includes updates of variables' default values;

As a rule of thumb, any data layout change would require full reloading. The JIT execution may continue without interruption, though.

## REPL

The REPL (Read-Eval-Print-Loop, however, the Print part isn't implemented) mode is architecturally similar to JIT. Code read from the input is injected contiguously into an implicit `repl` function. A transparent stack frame is allocated in the dynamic memory upon the session start and passed as an argument to the `repl` function on every loop.

```console
$ onyxc repl
Onyx REPL @ 1.0.0 / OnyxC x86_64-linux-gnu @ 1.0.0[45a28b1d]
> require "io" from ".shard/std"
> let x = 42
> let y = x + 1
> @cout << y
43
>
```

```nx
# That is an approximation of the `repl` function
#

def repl(frame : $void*, stack_offset : $int)
entry:
  # Code from the previous loops
  #

  require "io" from ".shard/std"

  let x = unsafe { alloca SInt32 }
  let y = unsafe { alloca SInt32 }

  set_stack_to(frame + stack_offset)

  # Current loop code
  #

  @cout << y # Would output 43

  # Implicit return
  #

  return frame + @sizeof(SInt32) * 2 # Return the new offset
end
```

## Cache

Caching is required to speed up consecutive builds. By default, all OnyxC cache is stored in a `./.onyxccache` directory relative to the working directoty. It is possible to redefine the cache directory using the `-[-C]ache-dir` flag, e.g. `-C/tmp/cache`.

OnyxC stores all types referenced from a source file, which allows to build a type dependency graph, in some cache file. A change in a type leads to a recompilation of all the files in the graph.

Macros can access type information outside of the scope of the current file via the `__type("T")` call. In that case, the file containing the macro is considered referencing `T`.

Different target parameters imply different cache units. The parameters include ISA, ABI, XFF, CPU, CPU features etc. A parameters hash is used as a cache differentiator, for example:

```
.onyxccache/
  79cd5a0a31b1572a3ef41084ab96ad18/ # x86_64-linux-elf
    main.nxbc
  023cc7e58a92677e2811b68573e96b36/ # x86_64-win32-pe
    main.nxbc
```
