#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// The canonical Ahead-Of-Time compiler implementation.
// It builds the whole program into a single executable.
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
