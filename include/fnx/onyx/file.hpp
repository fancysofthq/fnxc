#pragma once

#include <filesystem>
#include <optional>
#include <set>
#include <variant>
#include <vector>

#include "../unit.hpp"
#include "./cst.hpp"

namespace FNX {
namespace Onyx {

/// A physical file containing Onyx source code.
/// Its `source_stream` is read from the `bytes` buffer.
class File : public Unit {
public:
  struct CachedDependency {
    enum Type { C, Onyx, Lua };

    const Type type;

    const std::variant<
        std::filesystem::path,
        std::vector<std::filesystem::path>>
        paths;

    CachedDependency(
        Type type,
        const std::variant<
            std::filesystem::path,
            std::vector<std::filesystem::path>> paths);
  };

  // NOTE: The file containing a `complete` type implementation
  // contains the according specialization.
  //
  // TODO: Make the `File` class LLIR-unaware.
  struct StructSpecialization {};

  struct FunctionSpecialization {
    const std::string name;
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

  /// A cached file may reuse its LLIR module.
  bool cached;

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

  /// Check in the file has its CST cached. If the file has changed,
  /// its CST is deemed staleh, hence invalid.
  ///
  /// @return True if the file has its CST cached and valid.
  bool cached_cst_check();

  /// Return a list of cached dependees for this file. These shall be
  /// checked in parallel prior to working with the file.
  ///
  /// @return Nullopt if there is no dependees at all.
  std::optional<std::vector<CachedDependency>> cached_deps();

  /// @return True if current target parameters equal to the cached
  /// ones.
  bool cached_target_check();

  /// Evaluate the Onyx cache macro function, if any, and compare its
  /// result with the stored macro cache value. Updates the
  /// macro_cache_value variable with the new one, but does not write
  /// it to the cache file yet.
  ///
  /// @return False if the value returned from the function differs
  /// from the cached one.
  ///
  /// @return True if there is no cache macro function at all or the
  /// returned value is not different from the cached one.
  bool cached_macro_check();

  /// Check if any of the cached implicitly defined C macro values is
  /// invalid. If so, rebuilding is required.
  ///
  /// @return True if there is no need in rebuilding.
  bool cached_cmacro_check();

  /// Check if any of the cached environment variable values is
  /// invalid. If so, rebuilding is required.
  ///
  /// @return True of there is no need in rebuilding.
  bool cached_env_check();
};
} // namespace Onyx
} // namespace FNX
