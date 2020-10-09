#pragma once

// #include <cstddef>
#include <filesystem>
// #include <istream>
#include <optional>
#include <set>
#include <vector>
// #include <sstream>
// #include <stdint.h>
// #include <variant>
// #include <vector>

// #include "../c/token.hpp"
// #include "../lua/token.hpp"
// #include "../position.hpp"
#include "../unit.hpp"
// #include "../utils/flatten_variant.hpp"
#include "./cst.hpp"
// #include "./token.hpp"

namespace FNX {
namespace Onyx {

/// A physical file containing Onyx source code.
/// Its `source_stream` is read from the `bytes` buffer.
class File : public Unit {
public:
  struct Dependency {
    enum Type { C, Onyx, Lua };

    const Type type;
    const std::vector<std::filesystem::path> paths;

    Dependency(Type, const std::vector<std::filesystem::path>);
  };

  /// The CST of this file. It may be built either by parsing the
  /// file contents or by reading its cache.
  CST cst;

  /// Path to the source file.
  const std::filesystem::path file_path;

  /// Read bytes of the file, unless the file is cached.
  std::optional<std::basic_istream<std::byte>> bytes;

  /// Return a pointer to the `bytes` memory buffer.
  std::basic_iostream<char8_t> *source_stream() override;

  // /// Set of files depending on this file.
  // /// @note Circular dependencies are not possible.
  // std::set<std::weak_ptr<File>> dependants;

  // /// A precompiled Lua caching function.
  // /// It is called every time on validating a file cache.
  // std::optional<std::string> macro_cache_function;

  // /// The macro cache value, if any.
  // std::optional<std::string> macro_cache_value;

  // /// List of Onyx source files which this file depends on.
  // std::set<std::filesystem::path> onyx_dependendees;

  // /// List of C source files which this file depends on.
  // std::set<std::filesystem::path> c_dependendees;

  // /// List of Lua source files which this file depends on.
  // std::set<std::filesystem::path> lua_dependendees;

  File(std::filesystem::path);

  /// Check in system-dependent way if the file has changed.
  ///
  /// @return True if there is no `.basic.fnxcache.bin` file at all
  /// or it is determined that the file has actually changed.
  bool cache_is_physically_invalid();

  /// Return a list of cached dependees of this file. These shall be
  /// checked in parallel prior to working with the file.
  ///
  /// @return Nullopt if there is no dependees at all. In that case,
  /// the file may be immediately processed.
  std::optional<std::vector<Dependency>> cache_dependencies();

  /// @return True if target options cached for this file are no more
  /// valid, thus requiring AST rebuilding.
  bool cache_is_target_invalid();

  /// Evaluate the Onyx cache macro function, if any, and compare its
  /// result with the stored macro cache value. Updates the
  /// macro_cache_value variable with the new one, but not writes the
  /// it to the cache file yet.
  ///
  /// @return True if the value returned from the function differs
  /// from the cached one.
  ///
  /// @return False if there is no cache macro function or the
  /// returning value is not different from the cached one.
  bool cache_is_onyx_macro_invalid();

  /// Check if any of the cached C macro values is invalid. If so,
  /// rebuilding is required.
  bool cache_is_c_macro_invalid();

  /// Check if any of the cached environment variable values is
  /// invalid. If so, rebuilding is required.
  bool cache_is_env_invalid();
};
} // namespace Onyx
} // namespace FNX
