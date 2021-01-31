#pragma once

#include "../onyx/program.hpp"
#include "./module.hpp"

namespace FNX {
namespace LLIR {

/// Generates LLIR modules from an Onyx program.
class Codegen {
public:
  ///
  /// @param cache_dir If set, would treat the directory as a cache
  /// directory, looking up for and writing cached modules in binary
  /// format. Appends a source file path relative to the working
  /// directory to construct a cache path. For example, a file with
  /// relative path `./src/main.nx` would be cached into
  /// `cache_dir/./src/main.nx.llbc`.
  ///
  /// @param output If set, would output modules into the provided
  /// directory preserving the source file hierarchy. Whether is the
  /// represenation human-readable is defined by the boolean element.
  ///
  /// @return std::vector<Module>
  std::vector<Module> generate(
      Onyx::Program *program,
      std::optional<std::filesystem::path> cache_dir,
      std::optional<std::tuple<std::filesystem::path, bool>> output);
};

} // namespace LLIR
} // namespace FNX
