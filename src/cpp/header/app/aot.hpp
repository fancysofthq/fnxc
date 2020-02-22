#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The canonical Ahead-Of-Time compiler implementation.
// It builds the whole program into a single executable.
//
// ## Optimization
//
// The compiler is able to optimize the emitted binary code.
// There are multiple levels of optimization, controlled with `-O`
// flag:
//
//   * `-O0` — (default) no optimization, but fastest compilation;
//   hence good for development. It's the only optimization flag
//   allowing the `-d` flag, which enables debug info
//   * `-O1` — fast optimization, balanced; good for staging
//   * `-O2` — production optimization, balanced
//   * `-Os` — production optimization, biased for lesser binary size
//   * `-OS` — production optimization, minimum binary size possible
//   * `-Op` — production optimization, biased for greater performance
//   * `-OP` — production optimization, maximum performance
//
// ## Debugging
//
// Passing the `-d` flag to the compiler enables emission of debug
// information. It's only possible for the `-O0` optimization level.
class AOT : Shared::BC {
  const shared_ptr<Compile::Unit> _entry;
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
