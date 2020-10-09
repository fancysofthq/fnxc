#pragma once

#include <filesystem>
#include <optional>
#include <vector>

#include "fnx/unit.hpp"

namespace FNX {
namespace Lua {

/// A physical Lua file.
class File : public Unit {
public:
  std::filesystem::path path;

  /// Return true if the file is cached and the cache is valid.
  bool cache_physical_is_valid();

  /// Return a list of paths to Lua files this file depends on.
  std::optional<std::vector<std::filesystem::path>>
      cache_dependencies;
};

} // namespace Lua
} // namespace FNX
