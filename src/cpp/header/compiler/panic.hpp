#pragma once

#include "./location.hpp"

namespace Onyx {
namespace Compiler {
// A compiler panic occured due to a syntax or semantic error.
struct Panic : std::logic_error {
  stack<Location> backtrace;

  Panic(Location loc, string msg) : std::logic_error(msg) {
    backtrace.push(loc);
  }
};
} // namespace Compiler
} // namespace Onyx
