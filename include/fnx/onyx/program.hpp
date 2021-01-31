#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <ostream>
#include <set>
#include <unordered_map>
#include <vector>

#include "../c/ast.hpp"
#include "../llir/module.hpp"
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

  /// Require Onyx files in parallel.
  bool require(std::vector<std::filesystem::path> paths);

  // /// Compile the program. It iterates through all the `export`ed
  // /// functions, in declaration order. Function and types are
  // /// specialized, and delayed and final macros are evaluated,
  // /// resulting in LLIR modules.
  // std::vector<LLIR::Module> codegen();

  /// The global Onyx AST.
  Onyx::AST ast;

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

  /// Import a C file.
  bool _import(std::filesystem::path);

  /// Load a Lua file.
  bool _load(std::filesystem::path);

  /// The shared thread pool.
  Utils::ThreadPool *_thread_pool;

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

  /// Merge an Onyx CST into the global AST with immediate Onyx macro
  /// and C constant expression evaluation. Note that neither
  /// semantic analysis is performed nor delayed macros are
  /// evaluated.
  ///
  /// TODO: Immediate Onyx macro and C constan expression evaluation
  /// leads to sub-CST, then sub-AST generation.
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
