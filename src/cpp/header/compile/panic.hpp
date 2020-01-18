#pragma once
#include "./location.hpp"

namespace Onyx {
namespace Compile {
// A compiler panic occured due to a source program error.
struct Panic : std::logic_error {
  stack<Location> backtrace;

  Panic(Location loc, string msg) : std::logic_error(msg) {
    backtrace.push(loc);
  }
};
} // namespace Compile
} // namespace Onyx
