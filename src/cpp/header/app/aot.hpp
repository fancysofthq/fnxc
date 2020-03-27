#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The canonical Ahead-Of-Time compiler implementation.
// It builds the program into a single native executable.
//
// ## Optimization
//
// The compiler is able to optimize the emitted binary code.
// There are multiple levels of optimization, controlled with the
// `-[-O]ptimize` flag. Note that the table represents approximate
// values, and `-OS`'s worst may be better than `-O0`'s worst.
//
// Flag   | Compilation speed | Performance | Binary size
// ---    | ---               | ---         | ---
// `-O0`  | (5) Fastest       | (1) Worst   | (1) Biggest
// `-O1`  | (4) Fast          | (2) Bad     | (2) Big
// `-O2`  | (3) OK            | (3) OK      | (3) OK
// `-OP`  | (1) Slowest       | (5) Best    | (1) Biggest
// `-OS`  | (1) Slowest       | (1) Worst   | (5) Smallest
//
// `-O0` is the default, implicit flag. It implies no optimization,
// but the fastest compilation, hence good for development.`-O1` is
// perfect for staging, `-O2` is a balanced optimization for
// production, and other flags are biased for either greater
// performance or smaller binary size. Without a value, `-O` flag
// imples `-O1`.
//
// It is possible to specify `[p]erformance=` and `[s]ize=` options
// explicitly as numbers from 1 to 5. Their sum reflects the
// compilation time and must be less than or equal to 6, e.g.
// `--optimize [performance = 3, size = 2]`. The second number can be
// omitted, in that case it would be the maximum possible, e.g.
// `-Op=5` with implicit `s=1`, which equals to `-OP`.
//
// ## Debugging
//
// Passing the `-[-d]ebug` flag to the compiler enables emission
// of debugging information.
//
// By default, the debugging information format is the most
// expressive possible, e.g. DWARF. Some XFFs have debug information
// built-in, such as PE (win32), and it would be a error to pass
// specific `[f]ormat=` option for them.
//
// Format | Option       | Notes
// ---    | ---          | ---
// DWARF  | `dwarf[v=4]` | Version 4 by default
//
// Passing `gdb` or `lldb` as an `[e]xt=` option enables specific
// debugger's extensions, e.g. `-d[format = dwarf3, ext = gdb]`.
//
// ## Exterb
//
// To export `extern` definitions as a shared library instead of
// building a whole Onyx application, use the `-[-e]xtern` flag. To
// export an according header, use the `api` command.
//
// ```console
// # Export an optimized Linux object file
// $ onyxc build foo.nx -e -O2 --abi linux && [ -e a.o ]
//
// # Export a Win32 DLL file with debug information
// $ onyxc build foo.nx -e -d -o bin/libfoo.dll --abi win32
// ```
//
// ## Static builds
//
// Passing the `-[-s]tatic` flag enables static building.
class AOT : Shared::BC {
  const shared_ptr<Compiler::Unit> _entry;
  const filesystem::path _output;
  const unsigned char _workers;
  const bool _is_lib;

public:
  AOT(
      // filesystem::path root,
      filesystem::path input,
      filesystem::path output,
      bool lib,
      unsigned char workers);

  // Compile a program.
  void compile();
};
} // namespace App
} // namespace Onyx
