#pragma once

#include "./shared/bc.hpp"

namespace Onyx {
namespace App {
// An Ahead-Of-Time compiler application.
// It builds the program into a single object file.
class AOT : Shared::BC {
  const shared_ptr<Compiler::Unit> _entry;
  const filesystem::path _output;
  const unsigned short _workers;
  const bool _is_lib;

public:
  AOT(
      // filesystem::path root,
      filesystem::path input,
      filesystem::path output,
      bool lib,
      unsigned short workers);

  // Compile a program.
  void compile();
};
} // namespace App
} // namespace Onyx
