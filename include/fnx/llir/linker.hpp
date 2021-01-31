#pragma once

#include <optional>
#include <ostream>
#include <stdint.h>
#include <vector>

#include "./module.hpp"

namespace FNX {
namespace LLIR {

/// Links multiple LLIR modules into a single binary.
class Linker {
public:
  enum Mode {
    /// A single executable binary with a entry point.
    Binary,

    /// A shared library, i.e.\ an executable without a entry point.
    SharedLibrary,

    /// A static library, i.e.\ an archieve of object files.
    StaticLibrary,
  };

  /// @param mode The compilation mode.
  /// @param stream The output stream to write resulting binary into.
  /// @param modules A list of LLIR modules to link.
  /// @param entry The entry function name.
  /// @return The resulting size in bytes.
  unsigned long long link(
      Mode mode,
      std::ostream *stream,
      std::vector<Module *> modules,
      std::optional<std::string> entry = std::nullopt);
};

} // namespace LLIR
} // namespace FNX
