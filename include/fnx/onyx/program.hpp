#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>
#include <vector>

#include "../c/ast.hpp"
#include "../lua/universe.hpp"
#include "../utils/coro.hpp"
#include "../utils/thread_pool.hpp"
#include "./ast.hpp"
#include "./cst.hpp"

namespace FNX {
namespace Onyx {

/// A complete Onyx program, containing its own Onyx, C and Lua ASTs.
///
class Program {
public:
  /// Construct a new, distinctive Onyx program.
  Program(Utils::ThreadPool *thread_pool);

  /// Require Onyx file(s).
  bool require(std::vector<std::filesystem::path> paths);

private:
  struct Dependency {
    enum Type {
      Require,
      Import,
    };

    const Type type;
    const std::vector<std::filesystem::path> paths;

    Dependency(Type, std::vector<std::filesystem::path>);
  };

  /// Import C file(s).
  bool _import(std::vector<std::filesystem::path> paths);

  /// Load Lua file(s).
  bool _load(std::vector<std::filesystem::path> paths);

  /// The shared thread pool.
  Utils::ThreadPool *_thread_pool;

  /// The global Onyx AST.
  Onyx::AST _onyx_ast;

  /// The global C AST.
  C::AST _c_ast;

  /// The global Lua universe.
  Lua::Universe _lua_universe;

  std::mutex _onyx_ast_mutex;
  std::mutex _c_ast_mutex;

  /// Key is the dependee, values are dependants
  /// (i.e.\ depending on the dependee).
  std::unordered_map<
      std::filesystem::path,
      std::set<std::filesystem::path>>
      _onyx_dependendants;

  /// Key is the dependee, values are dependants
  /// (i.e.\ depending on the dependee).
  std::unordered_map<
      std::filesystem::path,
      std::set<std::filesystem::path>>
      _c_dependendants;

  /// Key is the dependee, values are dependants
  /// (i.e.\ depending on the dependee).
  std::unordered_map<
      std::filesystem::path,
      std::set<std::filesystem::path>>
      _lua_dependendants;

  /// Merge an Onyx CST into the global AST with Onyx macro and C
  /// constant expression evaluation and semantic analysis.
  ///
  /// Onyx, Lua and C ASTs may be written into simulataneously,
  /// therefore synchronization is required. Multiple readers are
  /// possible, thus the synchronization logic shall be implemented
  /// within this function. That said, the function is thread-safe.
  ///
  /// Yields `Dependency` once encountered a dependency. The CST
  /// merging shall stop and only be resumed when the dependency is
  /// satisfied.
  ///
  /// @param cst A parsed Onyx CST.
  Utils::Coro::generator<Dependency> _merge(Onyx::CST *cst);

  void _merge(C::CST *cst);

  Utils::Coro::generator<Dependency> _parse(Onyx::File *file);
};

} // namespace Onyx
} // namespace FNX
